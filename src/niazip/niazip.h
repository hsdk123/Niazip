#pragma once
#include <optional>
#include <string>
#include <vector>
#include <memory>

namespace niazpp
{
	using memory_source = std::string; // std::vector<char>;
	using string_type = std::string;

	struct zip_info
	{
		std::string filename;

		struct
		{
			int year = 1980;
			int month = 0;
			int day = 0;
			int hours = 0;
			int minutes = 0;
			int seconds = 0;
		} date_time;

		std::string comment;
		std::string extra;
		uint16_t create_system = 0;
		uint16_t create_version = 0;
		uint16_t extract_version = 0;
		uint16_t flag_bits = 0;
		std::size_t volume = 0;
		uint32_t internal_attr = 0;
		uint32_t external_attr = 0;
		std::size_t header_offset = 0;
		uint32_t crc = 0;
		std::size_t compress_size = 0;
		std::size_t file_size = 0;
	};

	enum class Compression {
		NO_COMPRESSION = 0,
		BEST_SPEED = 1,
		BEST_COMPRESSION = 9,
		UBER_COMPRESSION = 10,
		DEFAULT_LEVEL = 6,
		DEFAULT_COMPRESSION = -1
	};
}
namespace niazpp
{
	class niazip
	{
	public:
		niazip();
		~niazip();

		static std::unique_ptr<niazip> CreateWriter();
		static std::unique_ptr<niazip> CreateReaderFromFile(const string_type& filename);
		static std::unique_ptr<niazip> CreateReaderFromMemory(const memory_source& source);
		static std::unique_ptr<niazip> CreateReaderFromMemory(memory_source&& source);

	public:
		// writing methods
		bool add_entry_from_memory(const string_type& entry_name, const memory_source& source, const Compression& compression = Compression::NO_COMPRESSION);
		bool add_entry_from_file(const string_type& filepath, const Compression& compression = Compression::NO_COMPRESSION);
		bool add_directory_contents(const string_type& directory, const Compression& compression = Compression::NO_COMPRESSION);

		// reading methods
		memory_source extract_entry(const string_type& entry_name);

	public:
		// information
		bool exists_entry(const string_type& entry_name);
		std::vector<zip_info> get_info_entries();
		std::vector<string_type> get_entry_names();

	public:
		// utility
		bool save_to_file(const string_type& filename);

	private:
		void reset();
		void finalise();
		void start_read();
		void start_write();
		niazpp::zip_info getinfo(const int index);
		niazpp::zip_info getinfo(const string_type& entry_name);

		std::string read(const zip_info& info);
		std::string read(const std::string& entry_name);

	private:
		using ZipArchive = void;
		std::shared_ptr<ZipArchive> _archive;

		memory_source _buffer;
		std::string _comment;
	};
}