#pragma once

#include "renderer.h"

class gdimm_wic_renderer : public gdimm_renderer
{
public:
	bool begin(const dc_context *context);
};