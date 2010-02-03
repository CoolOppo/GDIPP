#pragma once

#include "global.h"

struct COLORRGB
{
	BYTE r;
	BYTE g;
	BYTE b;
	BYTE pad;
};

class _gdimm_text
{
	// device context attributes
	HDC _hdc_text;
	FT_Vector _cursor;
	LONG _text_height;

	// colors
	COLORREF _fg_color;
	COLORREF _bg_color;
	COLORRGB _fg_rgb;
	COLORRGB _bg_rgb;

	// metrics
	BYTE *_metric_buf;
	OUTLINETEXTMETRIC *_outline_metrics;
	GLYPHMETRICS _glyph_metrics;
	
	// bitmap
	HBITMAP _hbmp;
	BYTE *_bmp_buf;
	const FT_Glyph_Class *_glyph_clazz;

	void load_metrics();
	void get_glyph_clazz();
	void draw_glyph_lcd(const FT_BitmapGlyph glyph) const;

public:
	bool init(HDC hdc, int x, int y);
	void draw_background(CONST RECT *lprect) const;
	void text_out(const WCHAR *str, unsigned int count, CONST RECT *lprect, CONST INT *lpDx, BOOL is_glyph_index);
};

typedef singleton<_gdimm_text> gdimm_text;