#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <gmock/gmock-matchers.h>

#include <niazip/niazip_writer.h>
#include <niazip/niazip_reader.h>

#include <filesystem>
#include <unordered_set>
#include <fstream>

const niazpp::pathstring_type g_test_data_write_filepath = L"./data/test_write_data.zip";

TEST(WriterTest, BasicCheck)
{
	using namespace std;
	using namespace niazpp;
	{
		// previous test cleanup.
		/*remove(std::filesystem::path(g_test_data_write_filepath.begin(), g_test_data_write_filepath.end()));*/
		remove(std::filesystem::path(g_test_data_write_filepath));

		const auto writer = niazip_writer::Create(g_test_data_write_filepath);
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
		const auto reader = niazip_reader::CreateFromFile(g_test_data_write_filepath);
		ASSERT_TRUE(reader);

		const auto& vec_entry_names = reader->get_entry_names();
		{
			ASSERT_EQ(vec_entry_names.size(), 1);
			ASSERT_THAT(vec_entry_names, testing::UnorderedElementsAre(L"test_file.txt"));
		}
	}
}

namespace {
void clean_filepaths(std::vector<niazpp::pathstring_type>& filepaths) {
	// replace all backward slashes with forward slashes
	for (auto& filepath : filepaths) {
		std::replace(filepath.begin(), filepath.end(), '\\', '/');
	}
}
}

TEST(WriterTest, DirectoryAdd)
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
						
						// what we're testing: that we only add the relative filepaths inside the directory.
						populated_filenames.emplace_back(directory_name + L"/" + filename);
					}
				}
			}

			// write directory to archive
			const auto writer = niazip_writer::Create(test_directory_data_write_filepath);
			{
				ASSERT_TRUE(writer);
				ASSERT_TRUE(writer->add_directory_contents(root_directory));
			}
		}
		// b. read
		{
			const auto reader = niazip_reader::CreateFromFile(test_directory_data_write_filepath);
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
		}
	}
}

TEST(WriterTest, DirectoryAdd_IncludeDir)
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

						// what we're testing: that we add the full relative filepaths inside the directory.
						populated_filenames.emplace_back(root_directory + directory_name + L"/" + filename);
					}
				}
			}

			// write directory to archive
			const auto writer = niazip_writer::Create(test_directory_data_write_filepath);
			{
				ASSERT_TRUE(writer);
				ASSERT_TRUE(writer->add_directory(root_directory));
			}
		}
		// b. read
		{
			const auto reader = niazip_reader::CreateFromFile(test_directory_data_write_filepath);
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
		}
	}
}