#pragma once

#include "gdipp_lib/config_cache.h"

namespace gdipp
{

class dc_context
{
	// data structures and metrics retrieved from HDC are commonly used by multiple gdipp components
	// since gdipp does not alter state of the DC except its selected bitmap during painting, all members are considered constant

public:
	// metrics of the selected bitmap in the DC
	BITMAPINFOHEADER bmp_header;

	// reference to the DC
	HDC hdc;

	// logical font of the selected font in the DC
	LOGFONTW log_font;

	// metrics of the text in the DC
	OUTLINETEXTMETRICW *outline_metrics;

	bool init(HDC hdc);

private:
	// actual data buffer of the OUTLINETEXTMETRICW structure
	std::vector<BYTE> _metric_buf;
};

/*struct font_info
{
	DWORD table_header;
	DWORD face_index;
	FT_StreamRec stream;
	os2_metrics os2;
};*/

// apply alignment on the reference point and use it to calculate the baseline
POINT get_baseline(UINT alignment, int x, int y, int width, int ascent, int descent);

// for given bitmap width and bit count, compute the bitmap pitch
int get_bmp_pitch(int width, WORD bpp);

// retrieve BITMAPINFOHEADER from the selected bitmap of the given DC
bool get_dc_bmp_header(HDC hdc, BITMAPINFOHEADER &dc_dc_bmp_header);

// get outline metrics of the DC
OUTLINETEXTMETRICW *get_dc_metrics(HDC hdc, std::vector<BYTE> &metric_buf);

LOGFONTW get_log_font(HDC hdc);

// return true and fill the corresponding FT_Glyph_To_Bitmap render mode if find an appropriate render mode
// otherwise, return false
bool get_render_mode(const font_config_cache *font_config, WORD dc_bmp_bpp, BYTE font_quality, FT_Render_Mode &render_mode);

BOOL paint_background(HDC hdc, const RECT *bg_rect, COLORREF bg_color);

COLORREF parse_palette_color(HDC hdc, COLORREF color);

}
