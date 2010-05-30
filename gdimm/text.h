#pragma once

#include "setting_cache.h"
using namespace std;

class gdimm_text
{
public:
	struct gdimm_text_context
	{
		HDC hdc;
		OUTLINETEXTMETRICW *outline_metrics;
		const wchar_t *font_face;
		const font_setting_cache *setting_cache;
		bool use_alpha;
	};

protected:
	const gdimm_text_context *_context;
	BITMAPINFOHEADER _dc_bmp_header;
	UINT _text_alignment;
	COLORREF _text_color;
	LOGFONTW _font_attr;

public:
	virtual ~gdimm_text();

	virtual bool begin(const gdimm_text_context *context);
	virtual bool text_out(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx) = 0;
	virtual void end();
};