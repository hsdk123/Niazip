#include "niazip.h"

#include <mz.h>
#include <mz_strm.h>
#include <mz_strm_mem.h>
#include <mz_zip.h>
#include <mz_zip_rw.h>

#include <stdexcept>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;
using namespace niazpp;

niazip::niazip()
	: _archive(std::make_shared<ZipArchive>())
{
	reset();
}
niazip::~niazip()
{
	reset();
}

//-----------------------------------------------------------------------------------------------------

std::unique_ptr<niazip> niazpp::niazip::Create()
{
	auto zipfile = std::make_unique<niazip>();
	return zipfile;
}

std::unique_ptr<niazip> niazpp::niazip::CreateFromFile(
	const string_type& filename)
{
	auto zipfile = std::make_unique<niazip>();
	const auto status = mz_zip_reader_open_file(zipfile->_archive.get(), filename.c_str());
	if (!status)
	{
		return nullptr;
	}

	//std::ifstream stream(filename, std::ios::binary);
	//if (!stream) {
	//	return nullptr;
	//}

	///*load(stream);*/

	//zipfile->reset();
	//zipfile->_buffer.assign(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
	//zipfile->start_read();

	return zipfile;
}

std::unique_ptr<niazip> niazpp::niazip::CreateFromMemory(
	const memory_source& source)
{
	auto zipfile = std::make_unique<niazip>();
	{
		zipfile->_buffer = source;

		void* mem_stream = NULL;
		mz_stream_mem_create(&mem_stream);
		mz_stream_mem_set_buffer(mem_stream, zip_buffer, zip_buffer_size);
		mz_stream_open(mem_stream, NULL, MZ_OPEN_MODE_READ);

		if (!mz_zip_reader_init_mem(zipfile->_archive.get(), zipfile->_buffer.data(), zipfile->_buffer.size(), 0)) {
			return nullptr;
		}
	}
	return zipfile;
}

std::unique_ptr<niazip> niazpp::niazip::CreateFromMemory(memory_source&& source)
{
	auto zipfile = std::make_unique<niazip>();
	{
		zipfile->_buffer = std::move(source);
		if (!mz_zip_reader_init_mem(zipfile->_archive.get(), zipfile->_buffer.data(), zipfile->_buffer.size(), 0)) {
			return nullptr;
		}
	}
	return zipfile;
}

//-----------------------------------------------------------------------------------------------------

bool niazpp::niazip::add_entry_from_memory(const string_type& entry_name,
	const memory_source& source, const Compression& compression)
{
	// setup write mode if needed.
	start_write();

	mz_zip_file info;
	{

	}


	const auto res = mz_zip_writer_add_mem(_archive.get(), entry_name.c_str(),
		source.data(), source.size(), static_cast<mz_uint>(compression));
	return res;
}

bool niazpp::niazip::add_entry_from_file(const string_type& filepath,
	const Compression& compression)
{
	// read file to memory
	std::string data;
	{
		std::ifstream zipfile(filepath, std::ios::binary);
		data = std::string((std::istreambuf_iterator<char>(zipfile)),
			std::istreambuf_iterator<char>());
	}

	return add_entry_from_memory(filepath, data, compression);
}

bool niazpp::niazip::add_directory_contents(const string_type& directory, const Compression& compression)
{
	for (const auto& entry : fs::recursive_directory_iterator(directory)) {
		if (entry.is_regular_file()) {
			if (!add_entry_from_file(entry.path().u8string())) {
				return false;
			}
		}
	}

	return true;
}

//
//std::string read(const zip_info& info)
//{
//	std::size_t size;
//	char* data = static_cast<char*>(mz_zip_reader_extract_file_to_heap(archive_.get(), info.filename.c_str(), &size, 0));
//	if (data == nullptr)
//	{
//		throw std::runtime_error("file couldn't be read");
//	}
//	std::string extracted(data, data + size);
//	mz_free(data);
//	return extracted;
//}
//
//std::string read(const std::string& name)
//{
//	return read(getinfo(name));
//}

memory_source niazpp::niazip::extract_entry(const string_type& entry_name)
{
	memory_source ret = read(entry_name);
	return ret;
}

bool niazpp::niazip::exists_entry(const string_type& entry_name)
{
	// entry checkup requires read mode.
	start_read();

	const auto index = mz_zip_reader_locate_entry(_archive.get(), entry_name.c_str(), nullptr, 0);
	return index != -1;
}

std::vector<zip_info> niazpp::niazip::get_info_entries()
{
	if (_archive->m_zip_mode != MZ_ZIP_MODE_READING) {
		start_read();
	}

	std::vector<zip_info> ret;
	{
		const auto n = static_cast<int>(mz_zip_reader_get_num_files(_archive.get()));
		for (auto i = 0; i < n; ++i) {
			ret.emplace_back(getinfo(i));
		}
	}
	return ret;
}

std::vector<string_type> niazpp::niazip::get_decrypted_names()
{
	std::vector<string_type> ret;
	{
		const auto entries = get_info_entries();
		for (const auto& entry : entries) {
			ret.emplace_back(entry.filename);
		}
	}
	return ret;
}