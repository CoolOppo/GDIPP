#pragma once

#include "gdipp_config/render_config.h"
#include "gdipp_rpc/gdipp_rpc.h"

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
	//std::vector<BYTE> _metric_buf;
};

/*struct font_info
{
	DWORD table_header;
	DWORD face_index;
	FT_StreamRec stream;
	os2_metrics os2;
};*/

// high-performance division method to approximate number * numerator / 255
BYTE division_by_255(short number, short numerator);

// apply alignment on the reference point and use it to calculate the baseline
POINT get_baseline(UINT alignment, int x, int y, int width, int ascent, int descent);

// for given bitmap width and bit count, compute the bitmap pitch
int get_bmp_pitch(int width, WORD bpp);

// retrieve BITMAPINFOHEADER from the selected bitmap of the given DC
bool get_dc_bmp_header(HDC hdc, BITMAPINFOHEADER &dc_dc_bmp_header);

// get outline metrics of the DC
OUTLINETEXTMETRICW *get_dc_metrics(HDC hdc, std::vector<BYTE> &metric_buf);

LONG get_glyph_run_width(const gdipp_rpc_bitmap_glyph_run *a_glyph_run, bool is_control_width);

LOGFONTW get_log_font(HDC hdc);

bool mb_to_wc(const char *multi_byte_str, int count, std::wstring &wide_char_str);

BOOL paint_background(HDC hdc, const RECT *bg_rect, COLORREF bg_color);

COLORREF parse_palette_color(HDC hdc, COLORREF color);

}
