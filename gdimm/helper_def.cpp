#include "stdafx.h"
#include "helper_def.h"
#include "helper_func.h"
#include "gdimm.h"

bool dc_context::init(HDC hdc)
{
	outline_metrics = get_dc_metrics(hdc, _metric_buf);
	if (outline_metrics == NULL)
		return false;
	
	const gdimm_setting_trait setting_trait = {metric_face_name(outline_metrics),
		get_gdi_weight_class(static_cast<unsigned short>(outline_metrics->otmTextMetrics.tmWeight)),
		!!outline_metrics->otmTextMetrics.tmItalic,
		outline_metrics->otmTextMetrics.tmHeight};
	setting_cache = setting_cache_instance.lookup(&setting_trait);

	log_font = get_log_font(hdc);
	log_font.lfWeight = get_gdi_weight_class(static_cast<unsigned short>(log_font.lfWeight));

	if (!get_dc_bmp_header(hdc, bmp_header))
		return false;

	this->hdc = hdc;

	return true;
}