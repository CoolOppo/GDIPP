#pragma once

#include "gdipp_lib/helper.h"
#include "gdipp_lib/rpc.h"

namespace gdipp
{

/*
only the constructor changes internal state
all public functions are read-only
therefore it is thread-safe
*/
class font_link
{
public:
	font_link();

	const font_link_node *lookup_link(const wchar_t *font_name, size_t index) const;
	size_t get_link_count(const wchar_t *font_name) const;

private:
	typedef std::map<std::wstring, std::vector<font_link_node>, wstring_ci_less> link_map;

	link_map _link_table;
};

}
