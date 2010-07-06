#include "stdafx.h"
#include "text.h"
#include "text_helper.h"
#include "gdimm.h"

bool gdimm_text::gdimm_text_context::init(HDC hdc)
{
	this->hdc = hdc;

	outline_metrics = get_dc_metrics(hdc, _metric_buf);
	if (outline_metrics == NULL)
		return false;

	font_face = metric_face_name(outline_metrics);
	font_family = metric_family_name(outline_metrics);
	
	const gdimm_font_trait font_trait = {font_face,
		get_gdi_weight_class((unsigned short) outline_metrics->otmTextMetrics.tmWeight),
		!!outline_metrics->otmTextMetrics.tmItalic};
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

	get_dc_bmp_header(_context->hdc, _dc_bmp_header);

	if (!get_render_mode(_context->setting_cache, _dc_bmp_header.biBitCount, _font_attr.lfQuality, _render_mode))
		return false;

	_text_alignment = GetTextAlign(_context->hdc);
	assert(_text_alignment != GDI_ERROR);

	_char_extra = GetTextCharacterExtra(_context->hdc);
	assert(_char_extra != 0x8000000);

	_text_color = parse_palette_color(_context->hdc, GetTextColor(_context->hdc));
	assert(_text_color != CLR_INVALID);

	// transparent DC may not have background color
	_bg_color = parse_palette_color(_context->hdc, GetBkColor(_context->hdc));

	return true;
}

bool gdimm_text::text_out(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
	return false;
}

void gdimm_text::end()
{
}