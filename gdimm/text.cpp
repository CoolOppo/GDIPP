#include "stdafx.h"
#include "text.h"
#include "text_helper.h"
#include "gdimm.h"

bool gdimm_text::gdimm_text_context::init(HDC hdc)
{
	bool b_ret;

	this->hdc = hdc;

	if (!get_dc_metrics(hdc, _metric_buf, outline_metrics))
		return false;

	font_face = metric_face_name(outline_metrics);

	gdimm_os2_metrics os2_metrics;
	b_ret = os2_metrics.init(hdc);
	assert(b_ret);
	
	const gdimm_font_trait font_trait = {font_face, os2_metrics.get_weight_class(), os2_metrics.is_italic()};
	setting_cache = setting_cache_instance.lookup(font_trait);
	
	return true;
}

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