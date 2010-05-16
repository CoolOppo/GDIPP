#pragma once

#include "setting_cache.h"
using namespace std;

const double pi = acos(-1.0);

// convert 16.16 fixed point to 26.6 format
FT_F26Dot6 to_26dot6(const FIXED &x);

// convert floating point to 26.6 fixed point format
FT_F26Dot6 to_26dot6(FLOAT x);

// convert 16.16 fixed float type to integer
LONG from_16dot16(FT_Pos x);

// convert floating point to 16.16 format
FT_Pos to_16dot16(double x);

BOOL draw_background(HDC hdc, const RECT *bg_rect, COLORREF bg_color);

// apply alignment on the reference point and use it to calculate the baseline
POINT get_baseline(UINT alignment, int x, int y, int width, int ascent, int descent);

// for given bitmap width and bit count, compute the bitmap pitch
int get_bmp_pitch(int width, WORD bpp);

BITMAPINFO get_dc_bmp_info(HDC hdc);

// get various metrics of the DC
bool get_dc_metrics(HDC hdc, vector<BYTE> &metric_buf, OUTLINETEXTMETRICW *&outline_metrics);

int get_ft_bmp_width(const FT_Bitmap &bitmap);

RECT get_ft_glyph_run_rect(const vector<const FT_BitmapGlyph> &glyphs, const vector<POINT> &glyph_pos);

// for given DC bitmap bit count, return the corresponding FT_Glyph_To_Bitmap render mode
bool get_ft_render_mode(const wchar_t *font_name, WORD dc_bmp_bpp, FT_Render_Mode &render_mode);

LOGFONTW get_logfont(HDC hdc);

bool is_non_aa(BYTE font_quality, const font_setting_cache *setting_cache);

const wchar_t *metric_family_name(const OUTLINETEXTMETRICW *outline_metric);
const wchar_t *metric_face_name(const OUTLINETEXTMETRICW *outline_metric);
const wchar_t *metric_style_name(const OUTLINETEXTMETRICW *outline_metric);