#include "niazip_writer.h"

#include <mz.h>
#include <mz_zip.h>
#include <mz_strm.h>
#include <mz_zip_rw.h>

#include <fstream>

using namespace niazpp;

niazpp::niazip_writer::niazip_writer(string_view password)
{
}

niazpp::niazip_writer::~niazip_writer()
{
	if (_handle) {
		mz_zip_writer_close(_handle);
		mz_zip_writer_delete(&_handle); // this is when file gets saved.
	}
}

std::unique_ptr<niazip_writer> niazpp::niazip_writer::Create(const string_type& filepath, string_view password)
{
	auto ret = std::make_unique<niazip_writer>();
	{
		// create zip writer
		mz_zip_writer_create(&ret->_handle);

		if (!password.empty()) {
			mz_zip_writer_set_password(ret->_handle, password.data());
		}

		mz_zip_writer_set_compress_method(ret->_handle, MZ_COMPRESS_METHOD_STORE /*no compression*/);
		//mz_zip_writer_set_compress_level

		const auto err = mz_zip_writer_open_file(ret->_handle, filepath.c_str(), 0, 0 /*create, not append*/);

		// check errors
		if (err != MZ_OK) {
			mz_zip_writer_delete(&ret->_handle);
			return nullptr;
		}
	}
	return ret;
}

bool niazpp::niazip_writer::add_entry_from_file(const string_type& filepath)
{
	// note: could just use mz_zip_writer_add_file

	// get file buffer
	std::ifstream file(filepath, std::ios::binary);
	if (!file) {
		return false;
	}
	std::string data = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

	// create file info 
	mz_zip_file file_info = { 0 };
	{
		file_info.filename = filepath.c_str();
		file_info.flag = MZ_ZIP_FLAG_UTF8;
	}

	if (mz_zip_writer_add_buffer(_handle, data.data(), data.size(), &file_info) != MZ_OK) {
		return false;
	}

	return true;
}

bool niazpp::niazip_writer::add_entry_from_memory(const memory_source& source)
{
	return false;
}

bool niazpp::niazip_writer::add_directory_contents(const string_type& directory_path)
{
	return false;
}

std::vector<file_info> niazpp::niazip_writer::get_info_entries()
{
	return std::vector<file_info>();
}

std::vector<string_type> niazpp::niazip_writer::get_entry_names()
{
	return std::vector<string_type>();
}
