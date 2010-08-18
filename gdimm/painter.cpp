#include "stdafx.h"
#include "painter.h"
#include "helper_func.h"

gdimm_painter::~gdimm_painter()
{
}

bool gdimm_painter::begin(const dc_context *context, FT_Render_Mode render_mode)
{
	_context = context;
	_render_mode = render_mode;

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