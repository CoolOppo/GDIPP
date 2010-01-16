#pragma once

#include "global.h"

#include <map>
#include <vector>
#include <string>

using namespace std;

class gdimm_fontlink
{
	typedef map<t_string, vector<t_string>> fl_mapping;

	static gdimm_fontlink* _instance;
	fl_mapping fl_table;
	void get_fontlink_info();

public:
	gdimm_fontlink();
	static gdimm_fontlink *instance();
	const TCHAR *lookup(const TCHAR *font_name, size_t index) const;
};