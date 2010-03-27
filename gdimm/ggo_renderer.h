#pragma once

#include "renderer.h"
using namespace std;

class ggo_renderer: public gdimm_renderer
{
	static const FT_Glyph_Class *_glyph_clazz;

	double _embolden;
	UINT _ggo_format;
	MAT2 _matrix;
	FT_Render_Mode _render_mode;

	FT_BitmapGlyph outline_to_bitmap(WCHAR ch, GLYPHMETRICS &glyph_metrics) const;

public:
	ggo_renderer(gdimm_text *text);
	bool render(UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx, FT_Render_Mode render_mode);
};