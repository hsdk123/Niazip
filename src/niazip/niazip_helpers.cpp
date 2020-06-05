#include "niazip_helpers.h"
#include <algorithm>

namespace niazpp
{
	std::wstring s2ws(const std::string& str)
	{
		typedef std::codecvt_utf8<wchar_t> convert_typeX;
		std::wstring_convert<convert_typeX, wchar_t> converterX;

		return converterX.from_bytes(str);
	}

	std::string ws2s(const std::wstring& wstr)
	{
		typedef std::codecvt_utf8<wchar_t> convert_typeX;
		std::wstring_convert<convert_typeX, wchar_t> converterX;

		return converterX.to_bytes(wstr);
	}

	std::wstring clean_filepath(const std::wstring& filepath)
	{
		auto ret = filepath;
		{
			// replace all backward slashes with forward slashes
			std::replace(ret.begin(), ret.end(), L'\\', L'/');
		}
		return ret;
	}

	std::string clean_filepath(const std::string& filepath)
	{
		auto ret = filepath;
		{
			// replace all backward slashes with forward slashes
			std::replace(ret.begin(), ret.end(), '\\', '/');
		}
		return ret;
	}
}