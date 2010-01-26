#pragma once

#include "global.h"
#include <map>
#include <vector>
#include <string>

using namespace std;

class _gdimm_font_chg
{
	typedef map<t_string, vector<t_string>> fl_mapping;
	typedef map<t_string, t_string> fs_mapping;

	fl_mapping fl_table;
	fs_mapping fs_table;

	void get_font_link_info();
	void get_font_sub_info();

public:
	_gdimm_font_chg();
	const TCHAR *fl_lookup(const TCHAR *font_name, size_t index) const;
	const TCHAR *fs_lookup(const TCHAR *font_name) const;
};

typedef singleton<_gdimm_font_chg> gdimm_font_chg;