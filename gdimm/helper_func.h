#pragma once

#include "helper_def.h"
#include "setting_cache.h"

using namespace std;

const double pi = acos(-1.0);

// convert 16.16 fixed point to 26.6 format
FT_F26Dot6 to_26dot6(const FIXED &x);

// convert floating point to 26.6 fixed point format
FT_F26Dot6 to_26dot6(FLOAT x);

// convert 26.6 fixed float type to integer
LONG from_26dot6(FT_Pos x);

// convert 16.16 fixed float type to integer
LONG from_16dot16(FT_Pos x);

// convert floating point to 16.16 format
FT_Pos to_16dot16(double x);

DWORD create_tls_index();
BOOL free_tls_index(DWORD tls_index);

BYTE division_by_255(short number, short numerator);

// apply alignment on the reference point and use it to calculate the baseline
POINT get_baseline(UINT alignment, int x, int y, int width, int ascent, int descent);

// for given bitmap width and bit count, compute the bitmap pitch
int get_bmp_pitch(int width, WORD bpp);

bool get_dc_bmp_header(HDC hdc, BITMAPINFOHEADER &dc_dc_bmp_header);

// get outline metrics of the DC
OUTLINETEXTMETRICW *get_dc_metrics(HDC hdc, vector<BYTE> &metric_buf);

uint64_t get_font_trait(const LOGFONTW &log_font, FT_Render_Mode render_mode);

char get_gdi_weight_class(unsigned short weight);

int get_glyph_bmp_width(const FT_Bitmap &bitmap);

LONG get_glyph_run_height(const glyph_run *a_glyph_run);

LONG get_glyph_run_width(const glyph_run *a_glyph_run, bool is_actual_width);

LOGFONTW get_log_font(HDC hdc);

bool operator<(const LOGFONTW &lf1, const LOGFONTW &lf2);

// return true and fill the corresponding FT_Glyph_To_Bitmap render mode if find an appropriate render mode
// otherwise, return false
bool get_render_mode(const font_setting_cache *font_setting, WORD dc_bmp_bpp, BYTE font_quality, FT_Render_Mode &render_mode);

const FT_Glyph make_empty_outline_glyph();

bool mb_to_wc(const char *multi_byte_str, int count, wstring &wide_char_str);

BOOL paint_background(HDC hdc, const RECT *bg_rect, COLORREF bg_color);

COLORREF parse_palette_color(HDC hdc, COLORREF color);

const wchar_t *metric_family_name(const BYTE *metric_buf);
const wchar_t *metric_face_name(const BYTE *metric_buf);
const wchar_t *metric_style_name(const BYTE *metric_buf);
const wchar_t *metric_family_name(const OUTLINETEXTMETRICW *outline_metric);
const wchar_t *metric_face_name(const OUTLINETEXTMETRICW *outline_metric);
const wchar_t *metric_style_name(const OUTLINETEXTMETRICW *outline_metric);