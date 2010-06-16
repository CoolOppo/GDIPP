#pragma once

#include "text.h"
#include "font_man.h"

using namespace std;

class gdimm_gdi_text : public gdimm_text
{
	struct text_metrics
	{
		LONG width;
		LONG height;
		LONG ascent;
		LONG descent;
		POINT origin;
		POINT baseline;
	};

	HDC _hdc_canvas;
	POINT _cursor;
	RGBQUAD _text_rgb_gamma;

	void set_mono_mask_bits(const FT_BitmapGlyph glyph,
		BYTE *dest_bits,
		POINT dest_pos,
		int dest_width,
		int dest_height,
		int dest_ascent) const;
	void set_gray_text_bits(const FT_BitmapGlyph glyph,
		BYTE *dest_bits,
		POINT dest_pos,
		int dest_width,
		int dest_height,
		int dest_ascent) const;
	void set_lcd_text_bits(const FT_BitmapGlyph glyph,
		BYTE *dest_bits,
		POINT dest_pos,
		int dest_width,
		int dest_height,
		int dest_ascent,
		BYTE alpha) const;

	BOOL draw_mono(const text_metrics &metrics, UINT options, CONST RECT *lprect) const;
	BOOL draw_gray(const text_metrics &metrics, UINT options, CONST RECT *lprect) const;
	BOOL draw_lcd(const text_metrics &metrics, UINT options, CONST RECT *lprect) const;
	BOOL draw_text(UINT options, CONST RECT *lprect);

	virtual bool render(UINT options, LPCWSTR lpString, UINT c, CONST INT *lpDx) = 0;

protected:
	vector<const FT_BitmapGlyph> _glyphs;
	vector<POINT> _glyph_pos;

	gdimm_font_man _font_man;

public:
	gdimm_gdi_text();
	~gdimm_gdi_text();

	virtual bool begin(const gdimm_text_context *context);
	bool text_out(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx);
};