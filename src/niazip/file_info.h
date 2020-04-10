#pragma once
#include <string>

namespace niazpp
{
	struct file_info
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
}