#include "stdafx.h"
#include "text.h"
#include "text_helper.h"

gdimm_text::gdimm_text(HDC hdc)
:
_hdc_text(hdc)
{
}

bool gdimm_text::init()
{
	if (!get_dc_metrics(_hdc_text, _metric_buf, _outline_metrics))
		return false;

	_text_alignment = GetTextAlign(_hdc_text);
	assert(_text_alignment != GDI_ERROR);

	_bmp_info = get_dc_bmp_info(_hdc_text);

	_text_color = GetTextColor(_hdc_text);
	assert(_text_color != CLR_INVALID);
	_font_attr = get_logfont(_hdc_text);

	_font_face = metric_face_name(_outline_metrics);

	_setting_cache = setting_cache_instance.lookup(_font_face);

	return true;
}