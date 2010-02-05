#pragma once

#include "global.h"
#include <map>
using namespace std;

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
	BOOL _update_cursor;
	LONG _text_height;

	// colors
	COLORREF _fg_color;
	COLORRGB _fg_rgb;

	// metrics
	BYTE *_metric_buf;
	OUTLINETEXTMETRIC *_outline_metrics;

	//misc
	const FT_Glyph_Class *_glyph_clazz;

	void load_metrics();
	void get_glyph_clazz();
	void set_bmp_bits_mono(const FT_Bitmap &bitmap, BYTE *dest_bits) const;
	void set_bmp_bits_gray(const FT_Bitmap &bitmap, BYTE *dest_bits) const;
	void set_bmp_bits_lcd(const FT_Bitmap &bitmap, BYTE *dest_bits) const;
	void set_bmp_bits_alpha(const FT_Bitmap &bitmap, BYTE *dest_bits) const;
	void draw_glyph(const FT_BitmapGlyph glyph) const;

	const TCHAR *get_family_name() const;
	const TCHAR *get_full_name() const;

public:
	_gdimm_text();
	~_gdimm_text();
	void init(HDC hdc, int x, int y, BOOL is_update_cp);
	bool to_glyph_indices(LPCWSTR text, unsigned int count, WORD *glyph_indices);
	void text_out(const WORD *string, unsigned int count, CONST RECT *lprect, CONST INT *lpDx, BOOL is_glyph_index, BOOL is_pdy);
};

typedef singleton<_gdimm_text> gdimm_text;