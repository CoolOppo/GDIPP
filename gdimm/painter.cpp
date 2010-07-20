#include "stdafx.h"
#include "painter.h"
#include "helper_func.h"

gdimm_painter::~gdimm_painter()
{
}

bool gdimm_painter::begin(const dc_context *context)
{
	_context = context;

	if (_context->setting_cache->max_height != 0 &&
		_context->setting_cache->max_height < _context->outline_metrics->otmTextMetrics.tmHeight)
		return false;

	_text_alignment = GetTextAlign(_context->hdc);
	assert(_text_alignment != GDI_ERROR);

	_text_color = parse_palette_color(_context->hdc, GetTextColor(_context->hdc));
	assert(_text_color != CLR_INVALID);

	// transparent DC may not have background color
	_bg_color = parse_palette_color(_context->hdc, GetBkColor(_context->hdc));

	return true;
}

void gdimm_painter::end()
{
}