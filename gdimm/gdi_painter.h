#pragma once

#include "painter.h"
#include "obj_reg.h"

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

	static HDC _hdc_canvas;
	static gdimm_obj_registry _obj_reg;

	RGBQUAD _text_rgb_gamma;

	static void adjust_glyph_run_distance(bool is_pdy, UINT count, CONST INT *lpDx, glyph_run &a_glyph_run);

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
	BOOL paint_glyph_run(UINT options, CONST RECT *lprect, const glyph_run &a_glyph_run);

public:
	gdimm_gdi_painter();

	virtual bool begin(const dc_context *context);
	bool paint(int x, int y, UINT options, CONST RECT *lprect, const void *text, UINT c, CONST INT *lpDx);
};