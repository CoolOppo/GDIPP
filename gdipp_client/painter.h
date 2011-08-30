#pragma once

#include "gdipp_client/helper.h"

namespace gdipp
{

class painter
{
public:
	virtual ~painter();

	virtual bool begin(const dc_context *context);
	virtual void end();
	virtual bool paint(int x, int y, UINT options, CONST RECT *lprect, const void *text, UINT c, CONST INT *lpDx) = 0;

protected:
	const dc_context *_context;
	POINT _cursor;
	COLORREF _bg_color;
	FT_Render_Mode _render_mode;
	UINT _text_alignment;
	COLORREF _text_color;
};

}
