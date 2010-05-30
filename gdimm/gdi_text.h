#pragma once

#include "text.h"
#include "font_man.h"

using namespace std;

class gdimm_gdi_text : public gdimm_text
{
	struct draw_metrics
	{
		LONG width;
		LONG height;
		LONG ascent;
		LONG descent;
		POINT baseline;
	};

	// device context attributes
	RGBTRIPLE _text_rgb;

	// gamma ramps for red, green, blue
	const BYTE *_gamma_ramps[3];

	void set_mono_mask_bits(const FT_BitmapGlyph glyph,
		BYTE *dest_bits,
		POINT dest_pos,
		int dest_width,
		int dest_ascent,
		int dest_descent) const;
	void set_gray_text_bits(const FT_BitmapGlyph glyph,
		BYTE *dest_bits,
		POINT dest_pos,
		int dest_width,
		int dest_ascent,
		int dest_descent) const;
	void set_lcd_text_bits(const FT_BitmapGlyph glyph,
		BYTE *dest_bits,
		POINT dest_pos,
		int dest_width,
		int dest_ascent,
		int dest_descent,
		WORD alpha) const;

	bool draw_mono(const draw_metrics &metrics, UINT options, CONST RECT *lprect) const;
	bool draw_gray(const draw_metrics &metrics, UINT options, CONST RECT *lprect) const;
	bool draw_lcd(const draw_metrics &metrics, UINT options, CONST RECT *lprect) const;
	bool draw_glyphs(int x, int y, UINT options, CONST RECT *lprect) const;

	virtual bool render(UINT options, LPCWSTR lpString, UINT c, CONST INT *lpDx, FT_Render_Mode render_mode) = 0;

protected:
	POINT _cursor;
	int _char_extra;

	vector<const FT_BitmapGlyph> _glyphs;
	vector<POINT> _glyph_pos;

	gdimm_font_man _font_man;

public:
	virtual bool begin(const gdimm_text_context *context);
	bool text_out(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx);
};