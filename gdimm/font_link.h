#pragma once

#include "global.h"
#include <map>
#include <vector>
using namespace std;

class _gdimm_font_link
{
	map<t_string, vector<t_string>> _link_table;

	void get_font_link_info();

public:
	_gdimm_font_link();
	const TCHAR *lookup(const TCHAR *font_name, size_t index) const;
	const size_t get_link_count(const TCHAR *font_name) const;
};

typedef singleton<_gdimm_font_link> gdimm_font_link;