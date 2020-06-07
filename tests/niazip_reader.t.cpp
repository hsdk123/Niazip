#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <gmock/gmock-matchers.h>

#include <niazip/niazip_reader.h>
#include <niazip/niazip_helpers.h>

#include <filesystem>
#include <unordered_set>
#include <fstream>

const niazpp::pathstring_type g_test_data_filepath = L"./data/test_data.zip";
const niazpp::pathstring_type g_test_data_pw_filepath = L"./data/test_data_pw.zip";
const std::string g_test_data_pw = "asdfasdfasdf";

namespace {
void ReaderCheck(niazpp::niazip_reader& reader)
{
	const auto& vec_entry_info = reader.get_info_entries();
	const auto& vec_entry_names = reader.get_decrypted_names();
	{
		ASSERT_EQ(vec_entry_info.size(), vec_entry_names.size());
		ASSERT_EQ(vec_entry_info.size(), 2);

		ASSERT_THAT(vec_entry_names, testing::UnorderedElementsAre(L"data1.txt", L"data2.txt"));
	}
	{
		const auto entry = reader.extract_entry_to_memory("data1.txt");
		ASSERT_TRUE(entry.has_value());
		ASSERT_EQ(*entry, "data1_internal");
	}
	{
		const auto entry = reader.extract_entry_to_memory("data2.txt");
		ASSERT_TRUE(entry.has_value());
		ASSERT_EQ(*entry, "data2_internal");
	}
}
}

TEST(ReaderTest, BasicCheck)
{
	using namespace niazpp;
	const auto reader = niazip_reader::CreateFromFile(g_test_data_filepath);
	ASSERT_TRUE(reader);

	const auto& vec_entry_info = reader->get_info_entries();
	const auto& vec_entry_names = reader->get_decrypted_names();
	{
		ASSERT_EQ(vec_entry_info.size(), vec_entry_names.size());
		ASSERT_EQ(vec_entry_info.size(), 2);

		ASSERT_THAT(vec_entry_names, testing::UnorderedElementsAre(L"data1.txt", L"data2.txt"));
	}
}

TEST(ReaderTest, EntryCheck)
{
	using namespace niazpp;
	const auto reader = niazip_reader::CreateFromFile(g_test_data_filepath);
	ASSERT_TRUE(reader);
	
	ReaderCheck(*reader);
}

TEST(ReaderTest, CreateFromMemory)
{
	using namespace std;
	using namespace niazpp;

	std::ifstream file(g_test_data_filepath, ios::binary);
	ASSERT_TRUE(file);

	std::string data = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
	const auto reader = niazip_reader::CreateFromMemory(data);
	ASSERT_TRUE(reader);
	
	ReaderCheck(*reader);
}

TEST(ReaderTest, CreatePasswordProtectedFromMemory)
{
	using namespace std;
	using namespace niazpp;

	std::ifstream file(g_test_data_pw_filepath, ios::binary);
	ASSERT_TRUE(file);

	std::string data = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
	const auto reader = niazip_reader::CreateFromMemory(data, g_test_data_pw);
	ASSERT_TRUE(reader);

	ReaderCheck(*reader);
}

TEST(ReaderTest, ReadPasswordProtected)
{
	using namespace std;
	using namespace niazpp;

	std::ifstream file("./data/test_directory_pw.zip", ios::binary);
	ASSERT_TRUE(file);

	std::string data = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
	const auto reader = niazip_reader::CreateFromMemory(data, "asdfasdf");
	ASSERT_TRUE(reader);

	const auto filenames = reader->get_decrypted_names();
	for (const auto& filename : filenames) {
		const auto res = reader->extract_entry_to_memory(niazpp::ws2s(filename));
		ASSERT_TRUE(res);
	}
}
