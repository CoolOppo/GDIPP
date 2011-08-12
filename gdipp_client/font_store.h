#pragma once

#include "font_man.h"
#include "helper_def.h"

class gdimm_font_store
{
public:
	gdimm_font_store();
	~gdimm_font_store();

	font_info *lookup_font(long font_id);
	long register_font(HDC font_holder, const wchar_t *font_face);
	BOOL register_thread_font_man(gdimm_font_man *font_man);
	const gdimm_font_man *lookup_thread_font_man();
};
