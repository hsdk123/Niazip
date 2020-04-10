#include "niazip_reader.h"

#include <mz.h>
#include <mz_zip.h>
#include <mz_strm.h>
#include <mz_zip_rw.h>

using namespace niazpp;

niazpp::niazip_reader::niazip_reader(string_view password)
    : _password(password)
{
}

niazpp::niazip_reader::~niazip_reader()
{
    if (_handle) {
        mz_zip_reader_close(_handle);
        mz_zip_reader_delete(&_handle);
    }
}

std::unique_ptr<niazip_reader> niazpp::niazip_reader::CreateFromFile(const string_type& filepath, string_view password)
{
    auto ret = std::make_unique<niazip_reader>(password);
    {
        // create zip reader
        mz_zip_reader_create(&ret->_handle);

        if (!password.empty()) {
            mz_zip_reader_set_password(ret->_handle, password.data());
        }

        mz_zip_reader_set_encoding(ret->_handle, MZ_ENCODING_UTF8);
        /*mz_zip_reader_set_entry_cb(_handle, options, minizip_extract_entry_cb);
        mz_zip_reader_set_progress_cb(_handle, options, minizip_extract_progress_cb);
        mz_zip_reader_set_overwrite_cb(_handle, options, minizip_extract_overwrite_cb);*/

        const auto err = mz_zip_reader_open_file_in_memory(ret->_handle, filepath.c_str());

        // check errors
        if (err != MZ_OK) {
            mz_zip_reader_delete(&ret->_handle);
            return nullptr;
        }
    }
    return ret;
}

std::unique_ptr<niazip_reader> niazpp::niazip_reader::CreateFromMemory(const memory_source& source, string_view password)
{
    auto ret = std::make_unique<niazip_reader>(password);
    {
        // create zip reader
        mz_zip_reader_create(&ret->_handle);

        if (!password.empty()) {
            mz_zip_reader_set_password(ret->_handle, password.data());
        }

        mz_zip_reader_set_encoding(ret->_handle, MZ_ENCODING_UTF8);
        /*mz_zip_reader_set_entry_cb(_handle, options, minizip_extract_entry_cb);
        mz_zip_reader_set_progress_cb(_handle, options, minizip_extract_progress_cb);
        mz_zip_reader_set_overwrite_cb(_handle, options, minizip_extract_overwrite_cb);*/

        const auto err = mz_zip_reader_open_buffer(ret->_handle, (uint8_t*)(source.data()), source.length(), 0);

        // check errors
        if (err != MZ_OK) {
            mz_zip_reader_delete(&ret->_handle);
            return nullptr;
        }
    }
    return ret;
}

bool niazpp::niazip_reader::extract_to_destination(const string_type& destination_directory)
{
    return false;
}

bool niazpp::niazip_reader::extract_entry_to_destination(const string_type& destination_directory)
{
    return false;
}

std::optional<memory_source> niazpp::niazip_reader::extract_entry_to_memory(const string_type& entry_name)
{
    auto ret = std::make_optional<memory_source>();
    {
        auto err = mz_zip_reader_locate_entry(_handle, entry_name.c_str(), 1);
        if (err != MZ_OK) {
            return {};
        }

        err = mz_zip_reader_entry_open(_handle);
        {
            /*if (err == MZ_PASSWORD_ERROR) {
                mz_zip_reader_set_password(_handle, _password.data());
            }*/
            if (err != MZ_OK) {
                return {};
            }

            mz_zip_file* file_info = NULL;
            if (mz_zip_reader_entry_get_info(_handle, &file_info) != MZ_OK) {
                return {};
            }

            auto buffer = memory_source(file_info->uncompressed_size, ' ');
            {
                const auto bytes_read = mz_zip_reader_entry_read(_handle, buffer.data(), buffer.length());
            }
            ret.emplace(std::move(buffer));
        }
        mz_zip_reader_entry_close(_handle);
    }
    return ret;
}

namespace {
file_info mz_zip_file_info_to_file_info(const mz_zip_file& info)
{
    file_info ret;
    {
        ret.filename = info.filename;
    }
    return ret;
}
}

std::vector<file_info> niazpp::niazip_reader::get_info_entries()
{
    std::vector<file_info> ret;
    {
        if (mz_zip_reader_goto_first_entry(_handle) == MZ_OK) {
            do {
                mz_zip_file* file_info = NULL;
                if (mz_zip_reader_entry_get_info(_handle, &file_info) != MZ_OK) {
                    printf("Unable to get zip entry info\n");
                    break;
                }
                ret.emplace_back(mz_zip_file_info_to_file_info(*file_info));
            } while (mz_zip_reader_goto_next_entry(_handle) == MZ_OK);
        }
    }
    return ret;
}

std::vector<string_type> niazpp::niazip_reader::get_entry_names()
{
    std::vector<string_type> ret;
    {
        if (mz_zip_reader_goto_first_entry(_handle) == MZ_OK) {
            do {
                mz_zip_file* file_info = NULL;
                if (mz_zip_reader_entry_get_info(_handle, &file_info) != MZ_OK) {
                    printf("Unable to get zip entry info\n");
                    break;
                }
                ret.emplace_back(file_info->filename);
            } while (mz_zip_reader_goto_next_entry(_handle) == MZ_OK);
        }
    }
    return ret;
}

