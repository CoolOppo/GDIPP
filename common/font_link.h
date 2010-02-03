#pragma once

#include "global.h"
#include <map>
#include <vector>
using namespace std;

class _gdimm_font_link
{
	typedef map<t_string, vector<t_string>> fl_mapping;

	fl_mapping _fl_table;

	void get_font_link_info();

public:
	_gdimm_font_link();
	const TCHAR *lookup(const TCHAR *font_name, size_t index) const;
};

typedef singleton<_gdimm_font_link> gdimm_font_link;