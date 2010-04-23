#pragma once

#include "gdimm.h"
using namespace std;

class gdimm_text
{
	friend class gdimm_renderer;
	friend class ggo_renderer;
	friend class ft_renderer;

	// device context attributes
	HDC _hdc_text;
	POINT _cursor;
	UINT _text_alignment;
	RGBQUAD _fg_rgb;
	BITMAPINFO _bmp_info;

	// font attributes
	vector<BYTE> _metric_buf;
	OUTLINETEXTMETRICW *_outline_metrics;
	LOGFONTW _font_attr;
	const wchar_t *_font_face;
	const font_setting_cache *_setting_cache;

	// gamma ramps for gray, red, green, blue
	const BYTE *_gamma_ramps[4];

	static BOOL draw_background(HDC hdc, const RECT *bg_rect, COLORREF bg_color);
	static BITMAPINFO get_dc_bmp_info(HDC hdc);
	static int get_ft_bmp_width(const FT_Bitmap &bitmap);
	static RECT get_glyph_bmp_rect(const vector<const FT_BitmapGlyph> &glyphs, const vector<POINT> &glyph_pos, POINT cursor);

	bool get_dc_metrics();
	void get_gamma_ramps(const wchar_t *font_name, bool is_lcd);
	bool get_render_mode(const wchar_t *font_name, FT_Render_Mode &render_mode) const;

	void set_bmp_bits_mono(const FT_Bitmap &src_bitmap,
		const POINT &src_pos,
		BYTE *dest_bits,
		const POINT &dest_pos,
		const SIZE &dest_size) const;
	void set_bmp_bits_gray(const FT_Bitmap &src_bitmap,
		const POINT &src_pos,
		BYTE *dest_bits,
		const POINT &dest_pos,
		const SIZE &dest_size,
		WORD bmp_alpha) const;
	void set_bmp_bits_lcd(const FT_Bitmap &src_bitmap,
		const POINT &src_pos,
		BYTE *dest_bits,
		const POINT &dest_pos,
		const SIZE &dest_size,
		WORD bmp_alpha) const;
	bool draw_glyphs(const vector<const FT_BitmapGlyph> &glyphs,
		const vector<POINT> &glyph_pos,
		UINT options,
		CONST RECT *lprect) const;

	template <typename RENDERER>
	bool render_text(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx);

public:
	bool init(HDC hdc);
	bool text_out(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx);
};