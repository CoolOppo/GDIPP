#pragma once

#include "global.h"
#include FT_GLYPH_H
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
	COLORREF _bg_color;

	// metrics
	BYTE *_metric_buf;
	OUTLINETEXTMETRICW *_outline_metrics;
	LOGFONTW _font_attr;

	//misc
	UINT _eto_options;

	static int get_ft_bmp_width(const FT_Bitmap &bitmap);
	static void draw_background(HDC hdc, const RECT *bg_rect, COLORREF bg_color);
	
	BITMAP get_dc_bmp() const;
	FT_Render_Mode get_render_mode(WORD dc_bpp, const WCHAR *font_family) const;
	FT_UInt32 get_load_mode(FT_Render_Mode render_mode, const WCHAR *font_family) const;
	bool get_dc_metrics();
	void get_glyph_clazz();
	const WCHAR *get_font_family() const
	{ return (const WCHAR*)(_metric_buf + (UINT) _outline_metrics->otmpFamilyName); }

	const WCHAR *get_font_style() const
	{ return (const WCHAR*)(_metric_buf + (UINT) _outline_metrics->otmpStyleName); }

	FT_BitmapGlyph outline_to_bitmap(
		WCHAR ch,
		UINT ggo_format,
		const MAT2 &matrix,
		FT_Render_Mode render_mode,
		float bold_strength,
		GLYPHMETRICS &glyph_metrics) const;
	void set_bmp_bits_mono(
		const FT_Bitmap &src_bitmap,
		int x_in_dest, int y_in_dest,
		BYTE *dest_bits,
		int dest_width, int dest_height,
		WORD dest_bpp) const;
	void set_bmp_bits_gray(
		const FT_Bitmap &src_bitmap,
		int x_in_dest, int y_in_dest,
		BYTE *dest_bits,
		int dest_width, int dest_height,
		WORD dest_bpp,
		BYTE alpha) const;
	void set_bmp_bits_lcd(
		const FT_Bitmap &src_bitmap,
		int x_in_dest, int y_in_dest,
		BYTE *dest_bits,
		int dest_width, int dest_height,
		WORD dest_bpp,
		BYTE alpha) const;
	bool draw_glyphs(
		const vector<FT_BitmapGlyph> &glyphs,
		const vector<POINT> &glyph_pos,
		int max_glyph_height,
		CONST RECT *lprect,
		const BITMAP &dc_bmp) const;

	bool text_out_ggo(LPCWSTR lpString, UINT c, CONST RECT *lprect, CONST INT *lpDx);
	bool text_out_ft(LPCWSTR lpString, UINT c, CONST RECT *lprect, CONST INT *lpDx);

public:
	_gdimm_text()
	{ _metric_buf = NULL; }

	~_gdimm_text()
	{ if (_metric_buf != NULL) delete[] _metric_buf; }

	bool init(HDC hdc, int x, int y, UINT options);
	bool text_out(LPCWSTR lpString, UINT c, CONST RECT *lprect, CONST INT *lpDx);
};

typedef singleton<_gdimm_text> gdimm_text;