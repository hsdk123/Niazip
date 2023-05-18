#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <gmock/gmock-matchers.h>
#include "niazip_test_helpers.h"

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

TEST(WriterTest, DirectoryAdd)
{
	niaziptests::directory_tests(true);
}

TEST(WriterTest, DirectoryAdd_IncludeDir)
{
	niaziptests::directory_tests(false);
}

TEST(WriterTest, DirectoryAdd_IncludeDir_WithPassword)
{
	niaziptests::directory_tests(false, "asdfasdfasdfasdfasdf");
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

	niaziptests::directory_tests(false, "asdfasdfasdfasdfasdf",
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


