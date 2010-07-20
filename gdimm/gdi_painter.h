#pragma once

#include "font_man.h"
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

	BOOL paint_mono(UINT options, CONST RECT *lprect, const glyph_run *glyph_run_ptr, const text_metrics &metrics) const;
	BOOL paint_gray(UINT options, CONST RECT *lprect, const glyph_run *glyph_run_ptr, const text_metrics &metrics) const;
	BOOL paint_lcd(UINT options, CONST RECT *lprect, const glyph_run *glyph_run_ptr, const text_metrics &metrics) const;
	BOOL paint_glyph_run(UINT options, CONST RECT *lprect, const glyph_run *glyph_run_ptr);

public:
	gdimm_gdi_painter();
	~gdimm_gdi_painter();

	virtual bool begin(const dc_context *context);
	bool paint(int x, int y, UINT options, CONST RECT *lprect, CONST INT *lpDx, const glyph_run *glyph_run_ptr);
};