#pragma once

#include <map>
#include <vector>
#include "gdipp_support/gs_helper.h"
#include "gdipp_support/gs_rpc.h"

using std::map;
using std::vector;

/*
only the constructor changes internal state
all public functions are read-only
therefore it is thread-safe
*/
class gdipp_font_link
{
public:
	gdipp_font_link();
	const font_link_node *lookup_link(const wchar_t *font_name, size_t index) const;
	size_t get_link_count(const wchar_t *font_name) const;

private:
	typedef map<wstring, vector<font_link_node>, wstring_ci_less> link_map;

	link_map _link_table;
};
