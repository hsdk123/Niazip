#pragma once
#include <codecvt>
#include "niazip_types.h"

namespace niazpp
{
	std::wstring s2ws(const std::string& str);

	std::string ws2s(const std::wstring& wstr);

	std::wstring clean_filepath(const std::wstring& filepath);

	std::string clean_filepath(const std::string& filepath);
}