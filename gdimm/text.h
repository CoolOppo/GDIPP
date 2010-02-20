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
	COLORREF _bg_color;

	// metrics
	BYTE *_metric_buf;
	OUTLINETEXTMETRIC *_outline_metrics;
	LOGFONT _font_attr;

	//misc
	UINT _eto_options;

	static int get_ft_bmp_width(const FT_Bitmap &bitmap);
	static void draw_background(HDC hdc, const RECT *bg_rect, COLORREF bg_color);
	
	int get_dc_bpp() const;
	FT_Render_Mode get_render_mode(WORD dc_bpp, const TCHAR *font_family) const;
	FT_UInt32 get_load_mode(FT_Render_Mode render_mode, const TCHAR *font_family) const;
	bool get_dc_metrics();
	void get_glyph_clazz();
	const TCHAR *get_font_family() const
	{ return (const TCHAR*)(_metric_buf + (UINT) _outline_metrics->otmpFamilyName); }
	const TCHAR *get_font_style() const
	{ return (const TCHAR*)(_metric_buf + (UINT) _outline_metrics->otmpStyleName); }

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
		bool is_dest_up,
		WORD dest_bpp) const;
	void set_bmp_bits_gray(
		const FT_Bitmap &src_bitmap,
		int x_in_dest, int y_in_dest,
		BYTE *dest_bits,
		int dest_width, int dest_height,
		bool is_dest_up,
		WORD dest_bpp) const;
	void set_bmp_bits_lcd(
		const FT_Bitmap &src_bitmap,
		int x_in_dest, int y_in_dest,
		BYTE *dest_bits,
		int dest_width, int dest_height,
		bool is_dest_up,
		WORD dest_bpp) const;
	bool draw_glyphs(
		const vector<FT_BitmapGlyph> &glyphs,
		const vector<POINT> &glyph_pos,
		int max_glyph_height,
		CONST RECT *lprect,
		int dc_bpp) const;

	bool text_out_ggo(LPCWSTR lpString, UINT c, CONST RECT *lprect, CONST INT *lpDx);
	bool text_out_ft(LPCWSTR lpString, UINT c, CONST RECT *lprect, CONST INT *lpDx);

public:
	_gdimm_text();
	~_gdimm_text();
	bool init(HDC hdc, int x, int y, UINT options);
	bool text_out(LPCWSTR lpString, UINT c, CONST RECT *lprect, CONST INT *lpDx);
};

typedef singleton<_gdimm_text> gdimm_text;