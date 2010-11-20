#pragma once

#include "os2_metrics.h"
#include "setting_cache.h"

class dc_context
{
	// data structures and metrics retrieved from HDC commonly used by multiple gdipp components
	// since gdipp does not alter state of the DC except its selected bitmap during painting, all members are considered constant
	
	// actual data buffer of the OUTLINETEXTMETRICW structure
	vector<BYTE> _metric_buf;

public:
	// metrics of the selected bitmap in the DC
	BITMAPINFOHEADER bmp_header;

	// reference to the DC
	HDC hdc;

	// logical font of the selected font in the DC
	LOGFONTW log_font;

	// metrics of the text in the DC
	OUTLINETEXTMETRICW *outline_metrics;

	// gdipp setting associated to the DC
	const font_setting_cache *setting_cache;

	bool init(HDC hdc);
};

struct font_info
{
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

struct glyph_run
{
	// information for a glyph run, minimum units in the glyph run cache
	
	// glyph data pointers
	// different glyph runs could share same glyph, therefore each glyph is the minimum units in the glyph cache
	list<FT_Glyph> glyphs;

	/*
	the bounding boxes are dependent to specific glyph run
	control box is the formal positioning according to the glyph's advance vector
	black box, on the other hand, is the actual positioning, with glyph's bearing and bitmap width concerned
	the left border of the first glyph's control box always starts at 0, while the black box varies
	*/
	list<RECT> ctrl_boxes;
	list<RECT> black_boxes;
};