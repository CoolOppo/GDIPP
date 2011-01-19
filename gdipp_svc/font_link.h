#pragma once

#include <support_helper.h>
#include <support_rpc.h>

using namespace std;

/*
only the constructor changes internal state
all public functions are read-only
therefore it is thread-safe
*/
class gdipp_font_link
{
	typedef map<wstring, vector<font_link_node>, wstring_ci_less> link_map;

	link_map _link_table;

public:
	gdipp_font_link();
	const font_link_node *lookup_link(const wchar_t *font_name, size_t index) const;
	size_t get_link_count(const wchar_t *font_name) const;
};