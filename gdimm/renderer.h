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

	vector<FT_BitmapGlyph> _glyphs;
	vector<POINT> _glyph_pos;

public:
	gdimm_renderer(gdimm_text *text);
	~gdimm_renderer();

	virtual bool render(UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx, FT_Render_Mode render_mode) = 0;

	virtual const vector<FT_BitmapGlyph> &get_glyphs() const
	{ return _glyphs; }

	virtual const vector<POINT> &get_glyph_pos() const
	{ return _glyph_pos; }
};