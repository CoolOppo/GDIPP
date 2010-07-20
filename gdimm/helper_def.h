#pragma once

#include "setting_cache.h"
#include "MurmurHash2_64.h"

class dc_context
{
	vector<BYTE> _metric_buf;

public:
	BITMAPINFOHEADER bmp_header;
	HDC hdc;
	LOGFONTW log_font;
	OUTLINETEXTMETRICW *outline_metrics;
	FT_Render_Mode render_mode;
	const font_setting_cache *setting_cache;

	bool init(HDC hdc);
};

struct font_link_info
{
	wstring font_family;
	double scaling;
};

class font_trait
{
public:
	virtual bool operator<(const font_trait &trait) const = 0;
	virtual bool extract(HDC hdc) = 0;
};

struct glyph_info
{
	FT_BitmapGlyph glyph_bmp;
	POINT glyph_pos;
};

typedef list<glyph_info> glyph_run;