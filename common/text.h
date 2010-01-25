#pragma once

class gdimm_text
{
	HDC hdc_text;
	LOGFONT font_attr;
	COLORREF fg_color;
	COLORREF bg_color;

	// for font linking
	HFONT original_hfont;
	TCHAR original_fontname[LF_FACESIZE];

	void get_text_colors();
	void draw_bitmap_mono(FT_Bitmap bitmap, FT_Vector pos) const;
	void draw_bitmap_256(FT_Bitmap bitmap, FT_Vector pos) const;
	void draw_bitmap_lcd(FT_Bitmap bitmap, FT_Vector pos) const;
	void draw_bitmap_dwm(FT_Bitmap bitmap, FT_Vector pos) const;
	void draw_bitmap(FT_Bitmap bitmap, FT_Vector pos) const;

public:
	FT_Vector cursor;
	CONST RECT *clip_rect;
	CONST INT *distances;
	bool is_glyph_index;

	gdimm_text(HDC hdc);
	~gdimm_text();
	void draw_background(CONST RECT * lprect) const;
	int text_out(const TCHAR *text, unsigned int count, int fontlink_index = -1);
	bool fontlink(int fontlink_index);
};