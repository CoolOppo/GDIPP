#pragma once

#include "global.h"
#include <map>
#include <vector>
using namespace std;

class _gdimm_font_link
{
	map<wstring, vector<wstring>> _link_table;

	void get_font_link_info();

public:
	_gdimm_font_link()
	{ get_font_link_info(); }

	const WCHAR *lookup(const WCHAR *font_name, size_t index) const;
	const size_t get_link_count(const WCHAR *font_name) const;
};

typedef singleton<_gdimm_font_link> gdimm_font_link;