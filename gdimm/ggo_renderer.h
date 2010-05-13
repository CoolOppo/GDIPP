#pragma once

#include "gdi_text.h"

class ggo_renderer : public gdimm_gdi_text
{
	static const FT_Glyph_Class *_glyph_clazz;

	UINT _ggo_format;
	MAT2 _matrix;
	FT_Render_Mode _render_mode;

	const FT_BitmapGlyph outline_to_bitmap(wchar_t ch, GLYPHMETRICS &glyph_metrics) const;
	bool render(UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx, FT_Render_Mode render_mode);

public:
	ggo_renderer(HDC hdc);
	~ggo_renderer();

	virtual bool init();
};