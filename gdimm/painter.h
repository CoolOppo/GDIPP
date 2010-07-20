#pragma once

#include "helper_def.h"

class gdimm_painter
{
protected:
	const dc_context *_context;
	UINT _text_alignment;
	COLORREF _text_color;
	COLORREF _bg_color;
	POINT _cursor;

public:
	virtual ~gdimm_painter();

	virtual bool begin(const dc_context *context);
	virtual void end();
};