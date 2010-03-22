#pragma once

#include "font_man.h"
using namespace std;

class gdimm_text
{
	// device context attributes
	HDC _hdc_text;
	POINT _cursor;
	BOOL _update_cursor;
	UINT _text_alignment;
	int _char_extra;
	RGBQUAD _fg_rgb;
	COLORREF _bg_color;

	// font attributes
	vector<BYTE> _metric_buf;
	OUTLINETEXTMETRICW *_outline_metrics;
	LOGFONTW _font_attr;

	// one font holder for FreeType per instance
	// better alternative might be TLS HDC 
	HDC _font_holder;

	// gamma ramps for gray, red, green, blue
	const BYTE *_gamma_ramps[4];

	//misc
	UINT _eto_options;

	static int get_ft_bmp_width(const FT_Bitmap &bitmap);
	static void draw_background(HDC hdc, const RECT *bg_rect, COLORREF bg_color);
	static void oblique_outline(const FT_Outline *outline, double angle);
	
	BITMAPINFO get_dc_bmp_info() const;
	bool get_render_mode(WORD dc_bpp, const WCHAR *font_name, FT_Render_Mode &render_mode) const;
	FT_UInt32 get_load_mode(FT_Render_Mode render_mode, const WCHAR *font_name) const;
	bool get_dc_metrics();
	void get_glyph_clazz();
	void get_gamma_ramps(const WCHAR *font_name, bool is_lcd);

	FT_BitmapGlyph outline_to_bitmap(
		WCHAR ch,
		UINT ggo_format,
		const MAT2 &matrix,
		FT_Render_Mode render_mode,
		double embolden,
		GLYPHMETRICS &glyph_metrics) const;
	void set_bmp_bits_mono(
		const FT_Bitmap &src_bitmap,
		int x_in_src, int y_in_src,
		BYTE *dest_bits,
		int x_in_dest, int y_in_dest,
		int dest_width, int dest_height,
		WORD dest_bpp) const;
	void set_bmp_bits_gray(
		const FT_Bitmap &src_bitmap,
		int x_in_src, int y_in_src,
		BYTE *dest_bits,
		int x_in_dest, int y_in_dest,
		int dest_width, int dest_height,
		WORD dest_bpp,
		WORD bmp_alpha) const;
	void set_bmp_bits_lcd(
		const FT_Bitmap &src_bitmap,
		int x_in_src, int y_in_src,
		BYTE *dest_bits,
		int x_in_dest, int y_in_dest,
		int dest_width, int dest_height,
		WORD dest_bpp,
		WORD bmp_alpha,
		bool zero_alpha) const;
	bool draw_glyphs(
		const vector<FT_BitmapGlyph> &glyphs,
		const vector<POINT> &glyph_pos,
		CONST RECT *lprect,
		const BITMAPINFO &dc_bmp_info) const;

	bool text_out_ggo(LPCWSTR lpString, UINT c, CONST RECT *lprect, CONST INT *lpDx);
	bool text_out_ft(LPCWSTR lpString, UINT c, CONST RECT *lprect, CONST INT *lpDx);

public:
	gdimm_text();
	~gdimm_text();
	bool init(HDC hdc, int x, int y, UINT options);
	bool text_out(LPCWSTR lpString, UINT c, CONST RECT *lprect, CONST INT *lpDx);
};