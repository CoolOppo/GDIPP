#pragma once

#include "font.h"

#include <ObjBase.h>
#include <mlang.h>

class gdimm_text
{
private:
	HDC text_hdc;
	gdimm_font font;
	COLORREF fg_color;
	COLORREF bg_color;

	TCHAR original_fontname[LF_FACESIZE];

	void get_text_colors();
	void draw_bitmap_mono(FT_Bitmap bitmap, FT_Vector pos) const;
	void draw_bitmap_256(FT_Bitmap bitmap, FT_Vector pos) const;
	void draw_bitmap_lcd(FT_Bitmap bitmap, FT_Vector pos) const;
	void draw_bitmap(FT_Bitmap bitmap, FT_Vector pos) const;

public:
	FT_Vector cursor;
	CONST RECT *clip_rect;
	CONST INT *distances;

	gdimm_text(HDC hdc);
	~gdimm_text();
	void draw_background(CONST RECT * lprect) const;
	int text_out(const TCHAR *text, unsigned int count, int fontlink_index = 0);
	bool fontlink(int link_index);
};