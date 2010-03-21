#pragma once

#include <global.h>
using namespace std;

/*
only the constructor changes internal state
all public functions are read-only
therefore it is thread-safe
*/
class gdimm_font_link
{
	typedef map<wstring, vector<wstring>, wstring_ci_less> link_map;

	link_map _link_table;

public:
	gdimm_font_link();
	const WCHAR *lookup(const WCHAR *font_name, size_t index) const;
	size_t get_link_count(const WCHAR *font_name) const;
};