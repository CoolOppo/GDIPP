#include "stdafx.h"
#include "text.h"
#include "text_helper.h"

gdimm_text::~gdimm_text()
{
}

bool gdimm_text::begin(const gdimm_text_context *context)
{
	_context = context;

	if (_context->setting_cache->max_height != 0 &&
		_context->setting_cache->max_height < _context->outline_metrics->otmTextMetrics.tmHeight)
		return false;

	_font_attr = get_logfont(_context->hdc);

	_text_alignment = GetTextAlign(_context->hdc);
	assert(_text_alignment != GDI_ERROR);

	get_dc_dc_bmp_header(_context->hdc, _dc_bmp_header);

	_text_color = GetTextColor(_context->hdc);
	assert(_text_color != CLR_INVALID);

	return true;
}

bool gdimm_text::text_out(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
	return false;
}

void gdimm_text::end()
{
}