#pragma once
#include <niazip/niazip_writer.h>
#include <niazip/niazip_reader.h>
#include <niazip/niazip_helpers.h>

#include <filesystem>
#include <fstream>

namespace niaziptests 
{
void clean_filepaths(std::vector<niazpp::pathstring_type>& filepaths) {
	// replace all backward slashes with forward slashes
	for (auto& filepath : filepaths) {
		std::replace(filepath.begin(), filepath.end(), '\\', '/');
	}
}

void directory_tests(
	bool only_add_contents,
	const std::string& password = "",
	const niazpp::niazip_writer::file_encryptor file_encryptor = nullptr,
	const niazpp::niazip_reader::name_encrypter name_encrypter = nullptr,
	const niazpp::niazip_reader::name_decrypter name_decrypter = nullptr,
	const niazpp::niazip_reader::data_decrypter data_decrypter = nullptr
)
{
	using namespace std;
	using namespace std::filesystem;
	using namespace niazpp;

	const niazpp::pathstring_type test_directory_data_write_filepath = L"./data/test_write_directory_data.zip";
	const niazpp::pathstring_type root_directory = L"test_directory/";
	const auto directory_count = 5;
	{
		// previous test cleanup.
		remove(test_directory_data_write_filepath.c_str());
		remove_all(root_directory);

		// a. write
		std::vector<niazpp::pathstring_type> populated_filenames;
		std::unordered_map<niazpp::pathstring_type, std::string/*original data*/> populated_filedata;
		{
			create_directory(root_directory);

			// populate
			{
				for (auto i = 0; i < directory_count; ++i)
				{
					const auto directory_name = L"test" + std::to_wstring(i);
					const auto full_directory = root_directory + directory_name;

					// create file in directory
					create_directory(full_directory);
					{
						const auto filename = L"test" + std::to_wstring(i) + L".txt";
						const auto file_fullpath = full_directory + L"/" + filename;
						const auto file_data = std::filesystem::path(filename).u8string() + "extra_information";
						{
							/*ofstream file(std::filesystem::path(file_fullpath.begin(), file_fullpath.end()));*/
							ofstream file(file_fullpath);
							file << file_data;
							file.close();
						}

						if (only_add_contents) {
							// what we're testing: that we only add the relative filepaths inside the directory.
							const auto register_name = directory_name + L"/" + filename;
							populated_filenames.emplace_back(register_name);
							populated_filedata[register_name] = file_data;
						}
						else {
							// what we're testing: that we only add the relative filepaths inside the directory.
							const auto register_name = root_directory + directory_name + L"/" + filename;
							populated_filenames.emplace_back(register_name);
							populated_filedata[register_name] = file_data;
						}
					}
				}
			}

			// write directory to archive
			const auto writer = niazip_writer::Create(test_directory_data_write_filepath, password);
			{
				ASSERT_TRUE(writer);
				if (file_encryptor) {
					writer->set_file_encrypter(file_encryptor);
				}

				if (only_add_contents) {
					ASSERT_TRUE(writer->add_directory_contents(root_directory));
				}
				else {
					ASSERT_TRUE(writer->add_directory(root_directory));
				}
			}
		}
		// b. read
		{
			const auto reader = niazip_reader::CreateFromFile(test_directory_data_write_filepath, password);
			ASSERT_TRUE(reader);

			if (name_encrypter) {
				reader->set_name_encrypter(name_encrypter);
			}

			if (name_decrypter) {
				reader->set_name_decrypter(name_decrypter);
			}

			if (data_decrypter) {
				reader->set_data_decrypter(data_decrypter);
			}

			auto vec_decrypted__names = reader->get_decrypted_names();
			{
				ASSERT_EQ(vec_decrypted__names.size(), directory_count);

				// compare: O(n log n)
				{
					clean_filepaths(vec_decrypted__names);
					clean_filepaths(populated_filenames);

					sort(vec_decrypted__names.begin(), vec_decrypted__names.end());
					sort(populated_filenames.begin(), populated_filenames.end());
				}

				ASSERT_EQ(vec_decrypted__names, populated_filenames);
			}

			for (const auto& decrypted_name : vec_decrypted__names) {
				const auto res = reader->extract_entry_to_memory(niazpp::ws2s(decrypted_name));
				ASSERT_TRUE(res);
				ASSERT_EQ(populated_filedata[decrypted_name], *res);
			}
		}
	}
}
}
