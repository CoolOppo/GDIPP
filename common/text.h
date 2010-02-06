#pragma once

#include "global.h"
#include <map>
using namespace std;

class _gdimm_text
{
	// device context attributes
	HDC _hdc_text;
	FT_Vector _cursor;
	BOOL _update_cursor;
	UINT _text_alignment;

	// colors
	COLORREF _fg_color;
	RGBQUAD _fg_rgb;

	// metrics
	BYTE *_metric_buf;
	OUTLINETEXTMETRIC *_outline_metrics;

	//misc
	const FT_Glyph_Class *_glyph_clazz;

	bool load_metrics();
	void get_glyph_clazz();
	void set_bmp_bits_mono(const FT_Bitmap &src_bitmap, BYTE *dest_bits, bool is_dest_up) const;
	void set_bmp_bits_gray(const FT_Bitmap &src_bitmap, BYTE *dest_bits, bool is_dest_up) const;
	void set_bmp_bits_lcd(const FT_Bitmap &src_bitmap, BYTE *dest_bits, bool is_dest_up) const;
	void set_bmp_bits_alpha(const FT_Bitmap &src_bitmap, BYTE *dest_bits, bool is_dest_up) const;
	WORD create_bitmap(FT_Glyph *glyph_ptr, UINT width, UINT height, BITMAPINFO *&bmi) const;
	void draw_glyph(const FT_BitmapGlyph bmp_glyph, WORD src_bit_count, BITMAPINFO *bmi) const;

	const TCHAR *get_family_name() const;
	const TCHAR *get_full_name() const;

public:
	_gdimm_text();
	~_gdimm_text();
	bool init(HDC hdc, int x, int y);
	bool to_glyph_indices(LPCWSTR text, unsigned int count, WORD *glyph_indices);
	void text_out(const WCHAR *string, unsigned int count, CONST RECT *lprect, CONST INT *lpDx, BOOL is_glyph_index, BOOL is_pdy);
};

typedef singleton<_gdimm_text> gdimm_text;