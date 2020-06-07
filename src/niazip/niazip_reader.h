#pragma once
#include "file_info.h"
#include "niazip_types.h"

#include <optional>
#include <string>
#include <vector>
#include <memory>
#include <functional>

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
		// custom methods.

		using name_encrypter = std::function<std::string/*name registered in zip*/(const std::string&/*input: original name*/)>;
		void set_name_encrypter(name_encrypter func) { _name_encrypter = func; }

		using name_decrypter = std::function<std::string/*original name*/(const std::string&/*name registered in zip*/)>;
		void set_name_decrypter(name_decrypter func) { _name_decrypter = func; }

		using data_decrypter = std::function<bool(std::string&/*data*/)>;
		void set_data_decrypter(data_decrypter func) { _data_decrypter = func; }

	public:
		// information
		std::vector<file_info> get_info_entries();
		std::vector<pathstring_type> get_decrypted_names();

	private:
		using ZipHandle = void;
		ZipHandle* _handle = nullptr;
		const std::string _password;

		name_encrypter _name_encrypter = nullptr;
		name_decrypter _name_decrypter = nullptr;
		data_decrypter _data_decrypter = nullptr;
	};
}
