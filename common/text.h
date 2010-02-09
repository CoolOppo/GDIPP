#pragma once

#include "global.h"
#include <vector>
using namespace std;

class _gdimm_text
{
	// device context attributes
	HDC _hdc_text;
	POINT _cursor;
	BOOL _update_cursor;
	UINT _text_alignment;
	int _char_extra;
	RGBQUAD _fg_rgb;

	// metrics
	BYTE *_metric_buf;
	OUTLINETEXTMETRIC *_outline_metrics;
	LOGFONT _font_attr;

	//misc
	UINT _eto_options;

	static void draw_background(HDC hdc, CONST RECT *bg_rect, COLORREF color);
	static WORD get_bmp_bit_count(WORD dc_bit_count);
	FT_Render_Mode get_render_mode(WORD dc_bit_count) const;
	bool get_metrics();
	void get_glyph_clazz();
	void get_bmi(BITMAPINFO *&bmi) const;
	FT_BitmapGlyph get_glyph(WCHAR ch, UINT ggo_format, const MAT2 &matrix, FT_Render_Mode render_mode) const;
	void set_bmp_bits_mono(const FT_Bitmap &src_bitmap,
		int src_x, int src_y,
		BYTE *dest_bits,
		int dest_x,	int dest_y,
		int dest_width, int dest_height,
		bool is_dest_up,
		WORD dest_bit_count) const;
	void set_bmp_bits_gray(const FT_Bitmap &src_bitmap,
		int src_x, int src_y,
		BYTE *dest_bits,
		int dest_x, int dest_y,
		int dest_width, int dest_height,
		bool is_dest_up) const;
	void set_bmp_bits_lcd(const FT_Bitmap &src_bitmap,
		int src_x, int src_y,
		BYTE *dest_bits,
		int dest_x, int dest_y,
		int dest_width, int dest_height,
		bool is_dest_up,
		WORD dest_bit_count) const;
	void draw_glyph(const vector<FT_BitmapGlyph> &glyphs,
		const vector<POINT> &positions,
		const POINT &src_origin,
		CONST RECT *clip_rect,
		BITMAPINFO *bmi) const;

	const TCHAR *get_family_name() const;
	const TCHAR *get_full_name() const;

public:
	bool test;

	_gdimm_text();
	~_gdimm_text();
	bool init(HDC hdc, int x, int y, UINT options);
	bool to_glyph_indices(LPCWSTR text, unsigned int count, WORD *glyph_indices);
	bool text_out(const WCHAR *string, unsigned int count, CONST RECT *lprect, CONST INT *lpDx);
};

typedef singleton<_gdimm_text> gdimm_text;