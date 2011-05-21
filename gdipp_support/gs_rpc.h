#pragma once

#include <list>

using std::list;
using std::wstring;

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
	list<void *> glyphs;

	/*
	the bounding boxes are dependent to specific glyph run
	control box is the formal positioning according to the glyph's advance vector
	black box, on the other hand, is the actual positioning, with glyph's bearing and bitmap width concerned
	the left border of the first glyph's control box always starts at 0, while the black box varies
	*/
	list<RECT> ctrl_boxes;
	list<RECT> black_boxes;
};
