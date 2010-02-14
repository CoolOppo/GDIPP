#pragma once

#include "global.h"
#include <map>
#include <vector>
using namespace std;

class _gdimm_font_link
{
	map<t_string, vector<t_string>> _fl_table;

	//map<t_string, HFONT> _hfont_cache;
	//map<const HDC, HFONT> _orig_hfonts;

	void get_font_link_info();

public:
	_gdimm_font_link();
	const TCHAR *lookup(const TCHAR *font_name, size_t index) const;
	//bool font_link(HDC hdc, const TCHAR *family_name, int fl_index);
	//bool restore_font(HDC hdc);
};

typedef singleton<_gdimm_font_link> gdimm_font_link;