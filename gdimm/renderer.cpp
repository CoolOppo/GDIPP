#include "stdafx.h"
#include "renderer.h"

FT_F26Dot6 to_26dot6(const FIXED &fixed)
{
	return *((FT_F26Dot6*) &fixed) >> 10;
}

LONG from_16dot16(FT_Pos fixed)
{
	return fixed >> 16;
}

// convert floating point to 16.16 format
FT_Pos to_16dot16(double x)
{
	return (FT_Pos)(x * 65536);
}

gdimm_renderer::gdimm_renderer(gdimm_text *text)
:
_text(text)
{
	_char_extra = GetTextCharacterExtra(text->_hdc_text);
	assert(_char_extra != 0x8000000);
}

bool gdimm_renderer::render(UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx, FT_Render_Mode render_mode) const
{
	return false;
}

const vector<const FT_BitmapGlyph> &gdimm_renderer::get_glyphs() const
{
	return _glyphs;
}

const vector<POINT> &gdimm_renderer::get_glyph_pos() const
{
	return _glyph_pos;
}