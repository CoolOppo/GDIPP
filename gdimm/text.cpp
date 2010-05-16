#include "stdafx.h"
#include "text.h"
#include "text_helper.h"

gdimm_text::~gdimm_text()
{
}

bool gdimm_text::begin(HDC hdc, const OUTLINETEXTMETRICW *outline_metrics, const wchar_t *font_face, const font_setting_cache *setting_cache)
{
	if (setting_cache->max_height != 0 && setting_cache->max_height < outline_metrics->otmTextMetrics.tmHeight)
		return false;

	_hdc_text = hdc;
	_outline_metrics = outline_metrics;
	_font_face = font_face;
	_setting_cache = setting_cache;

	_text_alignment = GetTextAlign(_hdc_text);
	assert(_text_alignment != GDI_ERROR);

	_bmp_info = get_dc_bmp_info(_hdc_text);

	_text_color = GetTextColor(_hdc_text);
	assert(_text_color != CLR_INVALID);
	_font_attr = get_logfont(_hdc_text);

	return true;
}

bool gdimm_text::text_out(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
	return false;
}

void gdimm_text::end()
{
}