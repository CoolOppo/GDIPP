#pragma once

#include "global.h"

class gdimm_font
{
private:
	HDC font_holder;
	t_string curr_mapping_name;

	void get_mapping_name(HDC hdc);
	void get_font_info(HDC hdc);

public:
	HFONT curr_font_handle;
	LOGFONT curr_font_attr;
	const font_mapping *curr_font_mapping;

	gdimm_font(HDC hdc);
	//~gdimm_font();
	void load_font();
	void load_font(const TCHAR *font_name);
};