#pragma once

#include "gdimm.h"
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
	vector<BYTE> _metric_buf;
	OUTLINETEXTMETRICW *_outline_metrics;
	LOGFONTW _font_attr;
	const wchar_t *_font_face;

	const font_setting_cache *_setting_cache;

public:
	gdimm_text(HDC hdc);
	virtual bool init();
	virtual bool text_out(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx) = 0;
};