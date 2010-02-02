#pragma once

#include "global.h"

class _gdimm_text
{
	/*// device context attributes
	HDC _hdc_text;
	FT_Vector _cursor;
	COLORREF _fg_color;
	COLORREF _bg_color;
	TEXTMETRIC _text_metrics;
	
	// glyph related
	GLYPHMETRICS _glyph_metrics;
	HBITMAP _hbmp;
	BYTE *_bmp_buf;
	LONG _text_height;*/

	static FT_F26Dot6 fix_to_26dot6(const FIXED &fx);
	static void minimize_origin(const FT_Vector &new_pt, FT_Vector &origin);
	void prepare_bitmap(WORD bit_count);
	void draw_bitmap();

public:
	// device context attributes
	HDC _hdc_text;
	FT_Vector _cursor;
	COLORREF _fg_color;
	COLORREF _bg_color;
	TEXTMETRIC _text_metrics;
	
	// glyph related
	GLYPHMETRICS _glyph_metrics;
	HBITMAP _hbmp;
	BYTE *_bmp_buf;
	LONG _text_height;

	void init(HDC hdc, int x, int y);
	BOOL is_true_type() const;
	void draw_background(CONST RECT *lprect) const;
	void text_out(const WCHAR *str, unsigned int count, CONST RECT *lprect, CONST INT *lpDx, BOOL is_glyph_index);
};

typedef singleton<_gdimm_text> gdimm_text;