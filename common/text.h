#pragma once

#include <vector>
using namespace std;

class gdimm_text
{
	// device context attributes
	HDC _hdc_text;
	FT_Vector _cursor;
	CONST RECT *_clip_rect;
	CONST INT *_distances;
	COLORREF _fg_color;
	COLORREF _bg_color;

	// font related
	BYTE *_metric_buf;
	OUTLINETEXTMETRIC *_font_metric;
	LOGFONT _font_attr;
	int _font_height;
	// for font linking
	HFONT _original_hfont;

	void draw_bitmap_mono(FT_Bitmap bitmap, FT_Vector pos) const;
	void draw_bitmap_256(FT_Bitmap bitmap, FT_Vector pos) const;
	void draw_bitmap_lcd(FT_Bitmap bitmap, FT_Vector pos) const;
	void draw_bitmap_dwm(FT_Bitmap bitmap, FT_Vector pos) const;
	void draw_bitmap(FT_Bitmap bitmap, FT_Vector pos) const;

	TCHAR *get_family_name() const;
	TCHAR *get_full_name() const;
	int get_font_height() const;
	void load_font_info();
	bool load_next_font(int font_link_index);
	bool restore_dc_font();

public:
	gdimm_text(HDC hdc, int x, int y);
	~gdimm_text();
	BOOL is_font_true_type() const;
	unsigned int to_glyph_indices(LPCWSTR text, unsigned int start, unsigned int count, WCHAR *glyph_indices);
	void draw_background(CONST RECT *lprect) const;
	void text_out(const WCHAR *glyph_indices, unsigned int count, CONST RECT *lprect, CONST INT *lpDx);
};