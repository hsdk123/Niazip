#pragma once
#include "file_info.h"
#include "niazip_types.h"

#include <optional>
#include <vector>
#include <memory>
#include <functional>

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
		niazip_writer(const std::string& password = "");
		~niazip_writer();

		static std::unique_ptr<niazip_writer> Create(const pathstring_type& filepath, const std::string& password = "");

		// individual entry additions
		bool add_entry_from_file(const pathstring_type& filepath);
		bool add_entry_from_memory(const memory_source& source);

		// adds the [contents] of the directory + original directory
		bool add_directory(const pathstring_type& directory_path);

		// adds the [contents] of the directory, thus excludes including the directory itself.
		bool add_directory_contents(const pathstring_type& directory_path);

	public:
		// custom methods.

		using file_encryptor = std::function<bool(std::string&/*name to register in zip*/, std::string&/*data*/)>;
		void set_file_encrypter(file_encryptor func) { _file_encrypter = func; }

	public:
		// information
		std::vector<file_info> get_info_entries();
		std::vector<string_type> get_decrypted_names();

	private:
		using ZipHandle = void;
		ZipHandle* _handle = nullptr;
		file_encryptor _file_encrypter = nullptr;
		const std::string _password;
	};
}
