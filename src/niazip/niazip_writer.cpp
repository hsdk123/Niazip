#include "niazip_writer.h"
#include "niazip_helpers.h"

#include <mz.h>
#include <mz_zip.h>
#include <mz_strm.h>
#include <mz_zip_rw.h>

#include <fstream>
#include <filesystem>

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

std::unique_ptr<niazip_writer> niazpp::niazip_writer::Create(const pathstring_type& filepath, string_view password)
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

		const auto filepath_u8 = std::filesystem::path(filepath.begin(), filepath.end()).u8string();
		const auto err = mz_zip_writer_open_file(ret->_handle, filepath_u8.c_str(), 0, 0 /*create, not append*/);

		// check errors
		if (err != MZ_OK) {
			mz_zip_writer_delete(&ret->_handle);
			return nullptr;
		}
	}
	return ret;
}

bool niazpp::niazip_writer::add_entry_from_file(const pathstring_type& filepath)
{
	// note: could just use mz_zip_writer_add_file

	// get file buffer
	const auto filepath_u8 = std::filesystem::path(filepath.begin(), filepath.end()).u8string();
	std::ifstream file(filepath_u8, std::ios::binary);
	if (!file) {
		return false;
	}
	std::string data = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

	// create file info 
	mz_zip_file file_info = { 0 };
	{
		file_info.filename = filepath_u8.c_str();
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

bool niazpp::niazip_writer::add_directory(const pathstring_type& directory_path)
{
	if (directory_path.empty()) {
		return false;
	}

	const auto full_directory_clean = niazpp::clean_filepath(directory_path);
	pathstring_type root_directory = full_directory_clean;
	{
		if (full_directory_clean.back() == L'/') {
			root_directory.pop_back();
		}

		// get the last directory name in the string, then add '/'
		const auto pos = root_directory.find_last_of(L'/');
		if (pos != pathstring_type::npos) {
			root_directory = root_directory.substr(pos + 1);
		}
		root_directory += L'/';
	}

	using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
	for (auto& path_entry : recursive_directory_iterator(root_directory))
	{
		if (!path_entry.is_regular_file()) {
			continue;
		}

		// strip top directory path when registering in archive
		const auto filepath = path_entry.path();

		// get file buffer
		std::string data;
		{
			std::ifstream file(std::filesystem::path(filepath), std::ios::binary);
			if (!file) {
				return false;
			}
			const auto size = std::filesystem::file_size(path_entry.path());
			data = std::string(size, '\0');
			{
				file.read(data.data(), size);
			}
		}

		/*const auto local_filepath = filepath.wstring().substr(directory_path.size());
		const auto local_filepath_u8 = std::filesystem::path(local_filepath).u8string();*/
		const auto local_filepath_u8 = niazpp::clean_filepath(std::filesystem::path(filepath).u8string());

		// create file info 
		mz_zip_file file_info = { 0 };
		{
			file_info.filename = local_filepath_u8.c_str();
			file_info.flag = MZ_ZIP_FLAG_UTF8;
		}

		if (mz_zip_writer_add_buffer(_handle, data.data(), data.size(), &file_info) != MZ_OK) {
			return false;
		}
	}

	return true;
}

bool niazpp::niazip_writer::add_directory_contents(const pathstring_type& directory_path)
{
	if (directory_path.empty()) {
		return false;
	}

	pathstring_type root_directory = niazpp::clean_filepath(directory_path);
	if (root_directory.back() != '/') {
		root_directory += '/';
	}

	using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
	for (auto& path_entry : recursive_directory_iterator(root_directory))
	{
		if (!path_entry.is_regular_file()) {
			continue;
		}

		// strip top directory path when registering in archive
		const auto filepath = path_entry.path();

		// get file buffer
		std::string data;
		{
			std::ifstream file(std::filesystem::path(filepath), std::ios::binary);
			if (!file) {
				return false;
			}
			const auto size = std::filesystem::file_size(path_entry.path());
			data = std::string(size, '\0');
			{
				file.read(data.data(), size);
			}
		}

		const auto local_filepath = filepath.wstring().substr(directory_path.size());
		const auto local_filepath_u8 = niazpp::clean_filepath(std::filesystem::path(local_filepath).u8string());

		// create file info 
		mz_zip_file file_info = { 0 };
		{
			file_info.filename = local_filepath_u8.c_str();
			file_info.flag = MZ_ZIP_FLAG_UTF8;
		}

		if (mz_zip_writer_add_buffer(_handle, data.data(), data.size(), &file_info) != MZ_OK) {
			return false;
		}
	}

	return true;
}

std::vector<file_info> niazpp::niazip_writer::get_info_entries()
{
	return std::vector<file_info>();
}

std::vector<string_type> niazpp::niazip_writer::get_entry_names()
{
	return std::vector<string_type>();
}
