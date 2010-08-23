#pragma once

#include "MurmurHash2_64.h"
#include "os2_metrics.h"
#include "setting_cache.h"

class dc_context
{
	vector<BYTE> _metric_buf;

public:
	BITMAPINFOHEADER bmp_header;
	HDC hdc;
	LOGFONTW log_font;
	OUTLINETEXTMETRICW *outline_metrics;
	const font_setting_cache *setting_cache;

	bool init(HDC hdc);
};

struct font_info
{
	/*
	font storage stores two kinds of fonts: registered fonts and linked fonts

	registered fonts are created outside the font manager
	they are considered temporary, not managed by font manager
	registered fonts have non-negative font id
	the font information are shared by all threads, however the font holder is stored in TLS

	linked fonts are created by font manager, for font linking
	every linked font are kept alive until the font manager is destructed
	linked fonts have negative font id
	the handle and information of linked fonts are shared by all threads, however the font holder is stored in TLS
	*/

	// handle of the linked font for destruction
	// NULL for registered fonts
	HFONT linked_hfont;

	// used to retrieve font data from GetFontData
	DWORD table_header;
	DWORD face_index;

	FT_StreamRec stream;
	gdimm_os2_metrics os2_metrics;
};

struct font_link_node
{
	wstring font_family;
	double scaling;
};

struct glyph_node
{
	// glyph data pointer
	FT_Glyph glyph;

	// box encloses the bitmap of the glyph
	RECT black_box;

	// box indicates the formal position of the glyph
	RECT ctrl_box;
};

typedef list<glyph_node> glyph_run;