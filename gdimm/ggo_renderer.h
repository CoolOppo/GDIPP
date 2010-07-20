#pragma once

#include "renderer.h"

class ggo_font_trait : public font_trait
{
	LOGFONTW _font_attr;

public:
	bool operator<(const font_trait &trait) const;
	bool extract(HDC hdc);
};

class gdimm_ggo_renderer : public gdimm_renderer<ggo_font_trait>
{
	const FT_Glyph_Class *_glyph_clazz;
	UINT _ggo_format;
	MAT2 _matrix;

	bool get_glyph_metrics(wchar_t ch, GLYPHMETRICS &glyph_metrics) const;
	const FT_BitmapGlyph outline_to_bitmap(wchar_t ch, GLYPHMETRICS &glyph_metrics) const;
	bool render(LPCWSTR lpString, UINT c, bool is_glyph_index, glyph_run &a_glyph_run);

public:
	gdimm_ggo_renderer();

	bool begin(const dc_context *context);
};