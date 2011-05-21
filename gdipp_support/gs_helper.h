#pragma once

#include <sstream>

using std::wistringstream;
using std::wstring;

// convert a string to template value if possible
// helper function to convert raw setting strings to values
template <typename T>
void wcs_convert(const wchar_t *str, T *converted)
{
	if (str != NULL)
		wistringstream(str) >> *converted;
}

struct wstring_ci_less
{
	bool operator()(const wstring &string1, const wstring &string2) const;
};
