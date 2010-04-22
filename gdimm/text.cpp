#include "stdafx.h"
#include "text.h"
#include "text_helper.h"

bool gdimm_text::init(HDC hdc)
{
	_hdc_text = hdc;

	if (!get_dc_metrics(hdc, _metric_buf, _outline_metrics))
		return false;

	_text_alignment = GetTextAlign(hdc);
	assert(_text_alignment != GDI_ERROR);

	_bmp_info = get_dc_bmp_info(_hdc_text);

	_text_color = GetTextColor(_hdc_text);
	assert(_text_color != CLR_INVALID);
	_font_attr = get_logfont(hdc);

	_font_face = metric_face_name(_outline_metrics);

	_setting_cache = setting_cache_instance.lookup(_font_face);

	return true;
}