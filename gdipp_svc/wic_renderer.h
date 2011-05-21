#pragma once

#include "gdipp_svc/renderer.h"

// WIC painter renders glyphs itself
// this renderer is simply a stub

class gdimm_wic_renderer : public gdimm_renderer
{
public:
	bool begin(const dc_context *context);
};
