#pragma once
#include "file_info.h"
#include "niazip_types.h"

#include <optional>
#include <vector>
#include <memory>

namespace niazpp
{
	//--------------------------------------------------------------
	// Usage: 
	// file created upon Create(), 
	// file compression finishes on destructor
	//--------------------------------------------------------------
	class niazip_writer
	{
	public:
		niazip_writer(string_view password = "");
		~niazip_writer();

		static std::unique_ptr<niazip_writer> Create(const pathstring_type& filepath, string_view password = "");

		// individual entry additions
		bool add_entry_from_file(const pathstring_type& filepath);
		bool add_entry_from_memory(const memory_source& source);

		// adds the [contents] of the directory + original directory
		bool add_directory(const pathstring_type& directory_path);

		// adds the [contents] of the directory, thus excludes including the directory itself.
		bool add_directory_contents(const pathstring_type& directory_path);

	public:
		// information
		std::vector<file_info> get_info_entries();
		std::vector<string_type> get_entry_names();

	private:
		using ZipHandle = void;
		ZipHandle* _handle = nullptr;
		const std::string _password;
	};
}
