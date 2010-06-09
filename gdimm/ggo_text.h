#pragma once

#include "gdi_text.h"

class gdimm_ggo_text : public gdimm_gdi_text
{
	static const FT_Glyph_Class *_glyph_clazz;

	UINT _ggo_format;
	MAT2 _matrix;

	const FT_BitmapGlyph outline_to_bitmap(wchar_t ch, GLYPHMETRICS &glyph_metrics) const;
	bool render(UINT options, LPCWSTR lpString, UINT c, CONST INT *lpDx);

public:
	bool begin(const gdimm_text_context *context);
	bool text_out(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx);
};