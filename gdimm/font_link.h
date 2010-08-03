#pragma once

#include "helper_def.h"
#include <gdipp_common.h>

using namespace std;

/*
only the constructor changes internal state
all public functions are read-only
therefore it is thread-safe
*/
class gdimm_font_link
{
	typedef map<wstring, vector<font_link_node>, wstring_ci_less> link_map;

	link_map _link_table;

public:
	gdimm_font_link();
	const font_link_node *lookup_link(const wchar_t *font_name, size_t index) const;
	size_t get_link_count(const wchar_t *font_name) const;
};