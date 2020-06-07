#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <gmock/gmock-matchers.h>

#include <niazip/niazip_writer.h>
#include <niazip/niazip_reader.h>
#include <niazip/niazip_helpers.h>

#include <filesystem>
#include <unordered_set>
#include <fstream>

const niazpp::pathstring_type g_test_data_write_filepath = L"./data/test_write_data.zip";

namespace {
	void file_add(const std::string& password = "")
	{
		using namespace std;
		using namespace niazpp;
		{
			// previous test cleanup.
			/*remove(std::filesystem::path(g_test_data_write_filepath.begin(), g_test_data_write_filepath.end()));*/
			remove(std::filesystem::path(g_test_data_write_filepath));

			const auto writer = niazip_writer::Create(g_test_data_write_filepath, password);
			ASSERT_TRUE(writer);
			{
				// create file
				const auto filename = std::filesystem::path(L"test_file.txt");
				{
					ofstream file(filename);
					file << "my text";
					file.close();
				}

				// add file
				ASSERT_TRUE(writer->add_entry_from_file(filename));

				// cleanup
				remove(filename);
			}
		}
		{
			const auto reader = niazip_reader::CreateFromFile(g_test_data_write_filepath, password);
			ASSERT_TRUE(reader);

			const auto& vec_entry_names = reader->get_decrypted_names();
			{
				ASSERT_EQ(vec_entry_names.size(), 1);
				ASSERT_THAT(vec_entry_names, testing::UnorderedElementsAre(L"test_file.txt"));
			}
			for (const auto& entry_name : vec_entry_names) {
				const auto res = reader->extract_entry_to_memory(niazpp::ws2s(entry_name));
				ASSERT_TRUE(res);
			}
		}
	}
}

TEST(WriterTest, BasicCheck)
{
	file_add();
}

TEST(WriterTest, BasicCheck_WithPassword)
{
	file_add("asdfasdf");
}

namespace {
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

TEST(WriterTest, DirectoryAdd)
{
	directory_tests(true);
}

TEST(WriterTest, DirectoryAdd_IncludeDir)
{
	directory_tests(false);
}

TEST(WriterTest, DirectoryAdd_IncludeDir_WithPassword)
{
	directory_tests(false, "asdfasdfasdfasdfasdf");
}

namespace 
{
	class Encrypter
	{
	public:
		bool file_encryptor(std::string& name/*name registered in zip*/, std::string& data/*data*/)
		{
			_id += 1;

			const auto entry_name = std::to_string(_id);

			_original2entry_name[name] = entry_name;
			_entry2original_name[entry_name] = name;

			name = _original2entry_name[name];

			return data_encrypt_decrypter(data);
		}

		std::string/*name registered in zip*/ name_encrypter(const std::string& name/*original name*/)
		{
			const auto iter = _original2entry_name.find(name);
			if (iter == _original2entry_name.end()) {
				return "";
			}
			return iter->second;
		}

		std::string/*original name*/ name_decrypter(const std::string& name/*name registered in zip*/)
		{
			const auto iter = _entry2original_name.find(name);
			if (iter == _entry2original_name.end()) {
				return "";
			}
			return iter->second;
		}

		bool data_encrypt_decrypter(std::string& data/*data*/)
		{
			const auto n = static_cast<int>(data.size());
			for (auto i = 0; i < n && i < 100; ++i) {
				data[i] ^= 'x';
			}
			return true;
		}

	private:
		int _id = 0;
		std::unordered_map<std::string/*entry*/, std::string/*original_name*/> _entry2original_name;
		std::unordered_map<std::string/*original_name*/, std::string/*entry name*/> _original2entry_name;
	};
}

TEST(WriterTest, DirectoryAdd_IncludeDir_WithPassword_CustomEncryptDecrypt)
{
	Encrypter crypt;

	directory_tests(false, "asdfasdfasdfasdfasdf", 
		[&crypt](std::string& name/*name registered in zip*/, std::string& data/*data*/) {
			return crypt.file_encryptor(name, data);
		},
		[&crypt](const std::string& name/*original name*/) {
			return crypt.name_encrypter(name);
		},
		[&crypt](const std::string& name/*name registered in zip*/) {
			return crypt.name_decrypter(name);
		},
		[&crypt](std::string& data/*data*/) {
			return crypt.data_encrypt_decrypter(data);
		}
	);
}


