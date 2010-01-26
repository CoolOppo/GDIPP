#pragma once

class gdimm_text
{
	HDC hdc_text;
	FT_Vector cursor;
	CONST RECT *clip_rect;
	CONST INT *distances;

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
	gdimm_text(HDC hdc, int x, int y, CONST RECT *lprect, CONST INT *lpDx);
	~gdimm_text();
	void draw_background() const;
	void prepare();
	void text_out(const WCHAR *glyph_indices, unsigned int count);
	bool font_link(int font_link_index);
	bool to_glyph_indices(LPCWSTR text, unsigned int count, WCHAR *glyph_indices);
};