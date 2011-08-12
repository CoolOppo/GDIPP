#pragma once

#include <sstream>

namespace gdipp
{

// convert a string to template value if possible
// helper function to convert raw config strings to values
template <typename T>
void wcs_convert(const wchar_t *str, T *converted)
{
	if (str != NULL)
		std::wistringstream(str) >> *converted;
}

struct wstring_ci_less
{
	bool operator()(const std::wstring &string1, const std::wstring &string2) const;
};

}
