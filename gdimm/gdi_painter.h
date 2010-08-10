#pragma once

#include "painter.h"

using namespace std;

class gdimm_gdi_painter : public gdimm_painter
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
	RGBQUAD _text_rgb_gamma;

	static INT adjust_glyph_bbox(bool is_pdy, UINT count, CONST INT *lpDx, glyph_run &a_glyph_run);

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

	BOOL paint_mono(UINT options, CONST RECT *lprect, const glyph_run &a_glyph_run, const text_metrics &metrics) const;
	BOOL paint_gray(UINT options, CONST RECT *lprect, const glyph_run &a_glyph_run, const text_metrics &metrics) const;
	BOOL paint_lcd(UINT options, CONST RECT *lprect, const glyph_run &a_glyph_run, const text_metrics &metrics) const;
	BOOL paint_glyph_run(UINT options, CONST RECT *lprect, const glyph_run &a_glyph_run, INT max_glyph_distance = 0);

public:
	bool begin(const dc_context *context, FT_Render_Mode render_mode);
	void end();
	bool paint(int x, int y, UINT options, CONST RECT *lprect, const void *text, UINT c, CONST INT *lpDx);
};