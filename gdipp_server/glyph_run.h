#pragma once

namespace gdipp
{

class glyph_run
{
public:
	// information for a glyph run, minimum units in the glyph run cache

	// glyph data pointers
	// different glyph runs could share same glyph, therefore each glyph is the minimum units in the glyph cache
	std::vector<FT_Glyph> glyphs;

	/*
	the bounding boxes are dependent to specific glyph run
	control box is the formal positioning according to the glyph's advance std::vector
	black box, on the other hand, is the actual positioning, with glyph's bearing and bitmap width concerned
	the left border of the first glyph's control box always starts at 0, while the black box varies
	*/
	std::vector<RECT> ctrl_boxes;
	std::vector<RECT> black_boxes;
};

}
