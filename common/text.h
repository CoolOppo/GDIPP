#pragma once

#include "global.h"
#include <vector>
using namespace std;

struct bitmap_glyph_metrics
{
	FT_Int width;
	FT_Int height;
	FT_Int ascent;
	FT_Int descent;
	FT_Int bearing_x;
};

class _gdimm_text
{
	// device context attributes
	HDC _hdc_text;
	POINT _cursor;
	BOOL _update_cursor;
	UINT _text_alignment;

	// colors
	COLORREF _fg_color;
	RGBQUAD _fg_rgb;

	// metrics
	BYTE *_metric_buf;
	OUTLINETEXTMETRIC *_outline_metrics;

	//misc
	UINT _eto_options;

	static WORD get_bmp_bit_count(WORD dc_bit_count);
	static FT_Render_Mode get_render_mode(WORD dc_bit_count);
	static bitmap_glyph_metrics get_glyph_metrics(const vector<FT_BitmapGlyph> &glyphs, const vector<POINT> &positions, WORD bit_count);
	static void draw_background(HDC hdc, CONST RECT *bg_rect, COLORREF color);
	bool get_metrics();
	void get_glyph_clazz();
	void set_bmp_bits_mono(const FT_Bitmap &src_bitmap, BYTE *dest_bits, int dest_x, int dest_y, int dest_width, int dest_height, bool is_dest_up, bool inverted) const;
	void set_bmp_bits_gray(const FT_Bitmap &src_bitmap, BYTE *dest_bits, int dest_x, int dest_y, int dest_width, int dest_height, bool is_dest_up) const;
	void set_bmp_bits_lcd(const FT_Bitmap &src_bitmap, BYTE *dest_bits, int dest_x, int dest_y, int dest_width, int dest_height, bool is_dest_up) const;
	void set_bmp_bits_alpha(const FT_Bitmap &src_bitmap, BYTE *dest_bits, int dest_x, int dest_y, int dest_width, int dest_height, bool is_dest_up) const;
	void get_dc_bmi(BITMAPINFO *&bmi) const;
	void draw_glyph(const vector<FT_BitmapGlyph> &glyphs,
		const vector<POINT> &positions,
		const POINT &src_origin,
		const bitmap_glyph_metrics &metrics,
		CONST RECT *clip_rect,
		BITMAPINFO *bmi) const;

	const TCHAR *get_family_name() const;
	const TCHAR *get_full_name() const;

public:
	_gdimm_text();
	~_gdimm_text();
	bool init(HDC hdc, int x, int y, UINT options);
	bool to_glyph_indices(LPCWSTR text, unsigned int count, WORD *glyph_indices);
	bool text_out(const WCHAR *string, unsigned int count, CONST RECT *lprect, CONST INT *lpDx);
};

typedef singleton<_gdimm_text> gdimm_text;