#pragma once

#include "setting_cache.h"
using namespace std;

class gdimm_text
{
protected:
	// device context attributes
	HDC _hdc_text;
	UINT _text_alignment;
	BITMAPINFO _bmp_info;
	COLORREF _text_color;

	// font attributes
	const OUTLINETEXTMETRICW *_outline_metrics;
	LOGFONTW _font_attr;
	const wchar_t *_font_face;

	const font_setting_cache *_setting_cache;

public:
	virtual ~gdimm_text();

	virtual bool begin(HDC hdc, const OUTLINETEXTMETRICW *outline_metrics, const wchar_t *font_face, const font_setting_cache *setting_cache);
	virtual bool text_out(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx) = 0;
	virtual void end();
};