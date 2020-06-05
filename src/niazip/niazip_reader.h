#pragma once
#include "file_info.h"
#include "niazip_types.h"

#include <optional>
#include <string>
#include <vector>
#include <memory>

namespace niazpp
{
	using memory_source = std::string; // std::vector<char>;
	using string_type = std::string;
	using string_view = std::string_view;
}
namespace niazpp
{
	class niazip_reader
	{
	public:
		niazip_reader(string_view password = "");
		~niazip_reader();

		static std::unique_ptr<niazip_reader> CreateFromFile(const pathstring_type& filepath, string_view password = "");

		// caveat: expects source's lifetime to be longer than reader
		static std::unique_ptr<niazip_reader> CreateFromMemory(const memory_source& source, string_view password = "");

		// reading methods
		bool extract_to_destination(const pathstring_type& destination_directory);
		bool extract_entry_to_destination(const pathstring_type& destination_directory);
		std::optional<memory_source> extract_entry_to_memory(const string_type& entry_name);

	public:
		// information
		std::vector<file_info> get_info_entries();
		std::vector<pathstring_type> get_entry_names();

	private:
		using ZipHandle = void;
		ZipHandle* _handle = nullptr;
		const std::string _password;
	};
}
