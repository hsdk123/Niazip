#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <gmock/gmock-matchers.h>

#include <niazip/niazip_writer.h>
#include <niazip/niazip_reader.h>

#include <filesystem>
#include <unordered_set>
#include <fstream>

const std::string g_test_data_write_filepath = "./data/test_write_data.zip";

TEST(WriterTest, BasicCheck)
{
	using namespace std;
	using namespace niazpp;
	{
		const auto writer = niazip_writer::Create(g_test_data_write_filepath);
		ASSERT_TRUE(writer);
		{
			// create file
			const std::string filename = "test_file.txt";
			{
				ofstream file(filename);
				file << "my text";
				file.close();
			}

			// add file
			ASSERT_TRUE(writer->add_entry_from_file(filename));

			// cleanup
			remove(filename.c_str());
		}
	}
	{
		const auto reader = niazip_reader::CreateFromFile(g_test_data_write_filepath);
		ASSERT_TRUE(reader);

		const auto& vec_entry_names = reader->get_entry_names();
		{
			ASSERT_EQ(vec_entry_names.size(), 1);
			ASSERT_THAT(vec_entry_names, testing::UnorderedElementsAre("test_file.txt"));
		}
	}
}