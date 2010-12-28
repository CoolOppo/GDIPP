#pragma once

#include "os2_metrics.h"
#include "setting_cache.h"

class dc_context
{
	// data structures and metrics retrieved from HDC commonly used by multiple gdipp components
	// since gdipp does not alter state of the DC except its selected bitmap during painting, all members are considered constant
	
	// actual data buffer of the OUTLINETEXTMETRICW structure
	vector<BYTE> _metric_buf;

public:
	// metrics of the selected bitmap in the DC
	BITMAPINFOHEADER bmp_header;

	// reference to the DC
	HDC hdc;

	// logical font of the selected font in the DC
	LOGFONTW log_font;

	// metrics of the text in the DC
	OUTLINETEXTMETRICW *outline_metrics;

	// gdipp setting associated to the DC
	const font_setting_cache *setting_cache;

	bool init(HDC hdc);
};

struct font_info
{
	DWORD table_header;
	DWORD face_index;
	FT_StreamRec stream;
	gdimm_os2_metrics os2_metrics;
};