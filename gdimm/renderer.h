#pragma once

#include "text.h"
using namespace std;

const double pi = acos(-1.0);

// convert 16.16 fixed point to 26.6 format
FT_F26Dot6 to_26dot6(const FIXED &fixed);

// convert floating point to 26.6 format
FT_F26Dot6 to_26dot6(double x);

// convert 16.16 fixed float type to integer
LONG from_16dot16(FT_Pos fixed);

// convert floating point to 16.16 format
FT_Pos to_16dot16(double x);

class gdimm_renderer
{
protected:
	gdimm_text *_text;
	int _char_extra;

	vector<const FT_BitmapGlyph> _glyphs;
	vector<POINT> _glyph_pos;

	// gdimm_renderer cannot be instantiated directly
	// also because it is abstract class
	gdimm_renderer(gdimm_text *text);

public:
	~gdimm_renderer();

	bool render(UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx, FT_Render_Mode render_mode) const
	{ return false; }

	const vector<const FT_BitmapGlyph> &get_glyphs() const
	{ return _glyphs; }

	const vector<POINT> &get_glyph_pos() const
	{ return _glyph_pos; }
};