#pragma once

#include "font_store.h"

class font_store
{
public:
	font_store();
	~font_store();

	font_info *lookup_font(long font_id);
	long register_font(HDC font_holder, const wchar_t *font_face);
	BOOL register_thread_font_man(gdimm_font_man *font_man);
	const gdimm_font_man *lookup_thread_font_man();
};
