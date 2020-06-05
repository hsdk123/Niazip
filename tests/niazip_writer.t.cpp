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

			const auto& vec_entry_names = reader->get_entry_names();
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
		const std::string& password = ""
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
							{
								/*ofstream file(std::filesystem::path(file_fullpath.begin(), file_fullpath.end()));*/
								ofstream file(file_fullpath);
								file << std::filesystem::path(filename).u8string();
								file.close();
							}

							if (only_add_contents) {
								// what we're testing: that we only add the relative filepaths inside the directory.
								populated_filenames.emplace_back(directory_name + L"/" + filename);
							}
							else {
								// what we're testing: that we only add the relative filepaths inside the directory.
								populated_filenames.emplace_back(root_directory + directory_name + L"/" + filename);
							}
						}
					}
				}

				// write directory to archive
				const auto writer = niazip_writer::Create(test_directory_data_write_filepath, password);
				{
					ASSERT_TRUE(writer);
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

				auto vec_entry_names = reader->get_entry_names();
				{
					ASSERT_EQ(vec_entry_names.size(), directory_count);

					// compare: O(n log n)
					{
						clean_filepaths(vec_entry_names);
						clean_filepaths(populated_filenames);

						sort(vec_entry_names.begin(), vec_entry_names.end());
						sort(populated_filenames.begin(), populated_filenames.end());
					}

					ASSERT_EQ(vec_entry_names, populated_filenames);
				}

				for (const auto& entry_name : vec_entry_names) {
					const auto res = reader->extract_entry_to_memory(niazpp::ws2s(entry_name));
					ASSERT_TRUE(res);
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
