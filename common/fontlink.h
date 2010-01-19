#pragma once

#include "global.h"
#include <map>
#include <vector>
#include <string>

using namespace std;

class _gdimm_fontlink
{
	typedef map<t_string, vector<t_string>> fl_mapping;

	fl_mapping fl_table;

	void get_fontlink_info();

public:
	_gdimm_fontlink();
	const TCHAR *lookup(const TCHAR *font_name, size_t index) const;
};

typedef singleton<_gdimm_fontlink> gdimm_fontlink;