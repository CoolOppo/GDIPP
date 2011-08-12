#pragma once

#include "gdipp_server/renderer.h"

// WIC painter renders glyphs itself
// this renderer is simply a stub

class wic_renderer : public renderer
{
public:
	bool begin(const dc_context *context);
};
