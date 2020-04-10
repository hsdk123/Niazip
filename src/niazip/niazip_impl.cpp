#include "niazip.h"
//#include "./miniz/miniz.h"
#include <stdexcept>
#include <fstream>

using namespace niazpp;

//-----------------------------------------------------------------------------------------------------
// Utility

bool niazpp::niazip::save_to_file(const string_type& filename)
{
	using namespace std;
	finalise();

	ofstream stream(filename, std::ios::binary);
	if (!stream) {
		return false;
	}

	stream.write(_buffer.data(), static_cast<long>(_buffer.size()));
	return true;
}

//-----------------------------------------------------------------------------------------------------
// implementation

namespace Utility
{
	std::size_t write_callback(void* opaque, std::uint64_t file_ofs, const void* pBuf, std::size_t n)
	{
		auto buffer = static_cast<memory_source*>(opaque);

		if (file_ofs + n > buffer->size())
		{
			auto new_size = static_cast<memory_source::size_type>(file_ofs + n);
			buffer->resize(new_size);
		}

		for (std::size_t i = 0; i < n; i++)
		{
			(*buffer)[static_cast<std::size_t>(file_ofs + i)] = (static_cast<const char*>(pBuf))[i];
		}

		return n;
	}
}

void niazip::reset()
{
	// a. end all previous modes
	switch (_archive->m_zip_mode)
	{
	case MZ_ZIP_MODE_READING:
		mz_zip_reader_end(_archive.get());
		break;
	case MZ_ZIP_MODE_WRITING:
		mz_zip_writer_finalize_archive(_archive.get());
		mz_zip_writer_end(_archive.get());
		break;
	case MZ_ZIP_MODE_WRITING_HAS_BEEN_FINALIZED:
		mz_zip_writer_end(_archive.get());
		break;
	case MZ_ZIP_MODE_INVALID:
		break;
	}

	// b. ended all previous modes -> thus now should be in invalid state
	if (_archive->m_zip_mode != MZ_ZIP_MODE_INVALID) {
		throw std::runtime_error("");
	}

	_buffer.clear();
	_comment.clear();

	/*start_write();
	mz_zip_writer_finalize_archive(_archive.get());
	mz_zip_writer_end(_archive.get());*/
}

void niazpp::niazip::finalise()
{
	if (_archive->m_zip_mode == MZ_ZIP_MODE_WRITING) {
		mz_zip_writer_finalize_archive(_archive.get());
	}

	if (_archive->m_zip_mode == MZ_ZIP_MODE_WRITING_HAS_BEEN_FINALIZED) {
		mz_zip_writer_end(_archive.get());
	}

	// why this?
	if (_archive->m_zip_mode == MZ_ZIP_MODE_INVALID) {
		start_read();
	}
}

void niazip::start_read()
{
	if (_archive->m_zip_mode == MZ_ZIP_MODE_READING) {
		return;
	}

	if (_archive->m_zip_mode == MZ_ZIP_MODE_WRITING) {
		mz_zip_writer_finalize_archive(_archive.get());
	}

	if (_archive->m_zip_mode == MZ_ZIP_MODE_WRITING_HAS_BEEN_FINALIZED) {
		mz_zip_writer_end(_archive.get());
	}

	if (!mz_zip_reader_init_mem(_archive.get(), _buffer.data(), _buffer.size(), 0)) {
		throw std::runtime_error("bad zip");
	}
}

void niazip::start_write()
{
	// if already in write mode: do nothing.
	if (_archive->m_zip_mode == MZ_ZIP_MODE_WRITING) {
		return;
	}

	switch (_archive->m_zip_mode)
	{
	case MZ_ZIP_MODE_READING:
	{
		// read mode => write mode:
		// requires that all files are copied from reader to writer.

		// a. create copy of current archive
		mz_zip_archive archive_copy;
		std::memset(&archive_copy, 0, sizeof(mz_zip_archive));
		memory_source buffer_copy(_buffer.begin(), _buffer.end());
		{
			if (!mz_zip_reader_init_mem(&archive_copy, buffer_copy.data(), buffer_copy.size(), 0)) {
				throw std::runtime_error("bad zip");
			}

			mz_zip_reader_end(_archive.get());
		}

		// b. update original archive as a writer mode archive
		{
			_archive->m_pWrite = &Utility::write_callback;
			_archive->m_pIO_opaque = &_buffer;
			_buffer = memory_source();

			if (!mz_zip_writer_init(_archive.get(), 0)) {
				throw std::runtime_error("bad zip");
			}
		}

		// c. copy all files into current write mode archive
		{
			const auto n = static_cast<unsigned int>(archive_copy.m_total_files);
			for (auto i = 0u; i < n; i++) {
				if (!mz_zip_writer_add_from_zip_reader(_archive.get(), &archive_copy, i)) {
					throw std::runtime_error("fail");
				}
			}

			mz_zip_reader_end(&archive_copy);
		}
		return;
	}
	case MZ_ZIP_MODE_WRITING_HAS_BEEN_FINALIZED:
		mz_zip_writer_end(_archive.get());
		break;
	case MZ_ZIP_MODE_INVALID:
	case MZ_ZIP_MODE_WRITING:
		break;
	}

	// start write mode.
	_archive->m_pWrite = &Utility::write_callback;
	_archive->m_pIO_opaque = &_buffer;

	if (!mz_zip_writer_init(_archive.get(), 0)) {
		throw std::runtime_error("bad zip");
	}
}

zip_info niazip::getinfo(const int index)
{
	// requires read mode.
	start_read();

	// read entry information
	mz_zip_archive_file_stat stat;
	mz_zip_reader_file_stat(_archive.get(), static_cast<mz_uint>(index), &stat);

	// copy entry information
	zip_info result;
	{
		result.filename = std::string(stat.m_filename, stat.m_filename + std::strlen(stat.m_filename));
		result.comment = std::string(stat.m_comment, stat.m_comment + stat.m_comment_size);
		result.compress_size = static_cast<std::size_t>(stat.m_comp_size);
		result.file_size = static_cast<std::size_t>(stat.m_uncomp_size);
		result.header_offset = static_cast<std::size_t>(stat.m_local_header_ofs);
		result.crc = stat.m_crc32;
		/*auto time = detail::safe_localtime(stat.m_time);
		result.date_time.year = 1900 + time.tm_year;
		result.date_time.month = 1 + time.tm_mon;
		result.date_time.day = time.tm_mday;
		result.date_time.hours = time.tm_hour;
		result.date_time.minutes = time.tm_min;
		result.date_time.seconds = time.tm_sec;*/
		result.flag_bits = stat.m_bit_flag;
		result.internal_attr = stat.m_internal_attr;
		result.external_attr = stat.m_external_attr;
		result.extract_version = stat.m_version_needed;
		result.create_version = stat.m_version_made_by;
		result.volume = stat.m_file_index;
		result.create_system = stat.m_method;
	}
	return result;
}

niazpp::zip_info niazpp::niazip::getinfo(const string_type& entry_name)
{
	// requires read mode.
	start_read();

	const auto index = mz_zip_reader_locate_file(_archive.get(), entry_name.c_str(), nullptr, 0);
	if (index == -1) {
		throw std::runtime_error("not found");
	}

	return getinfo(index);
}

std::string niazpp::niazip::read(const zip_info& info)
{
	// extract from archive
	std::size_t size;
	char* data = static_cast<char*>(mz_zip_reader_extract_file_to_heap(
		_archive.get(), info.filename.c_str(), &size, 0));

	if (data == nullptr) {
		throw std::runtime_error("file couldn't be read");
	}

	// copy data from memory into string: copying!!
	std::string extracted(data, data + size);
	mz_free(data);

	return extracted;
}

std::string niazpp::niazip::read(const std::string& entry_name)
{
	return read(getinfo(entry_name));
}
