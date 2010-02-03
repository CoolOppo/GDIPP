#include "stdafx.h"
#include "text.h"
#include "global.h"
#include "ft.h"
#include "font_man.h"
#include <cmath>
#include <limits.h>
#include <vector>
using namespace std;

// helper definitions

#define REFTORGB(ref) (*((COLORRGB*) &ref))
#define AlignDown(num, alignment) (num - num & (alignment -	1))

// helper functions

inline int AlignUp(int num, int alignment)
{
	if (num == 0)
		return alignment;
	else if (num % alignment == 0)
		return num;
	else
		return (num / alignment + 1) * alignment;
}

inline FT_F26Dot6 fix_to_26dot6(const FIXED &fx)
{
	return *(LONG*)(&fx) >> 10;
}

inline void minimize_origin(const FT_Vector &new_point, FT_Vector &origin)
{
	if (new_point.x < origin.x)
		origin.x = new_point.x;
	
	if (new_point.y < origin.y)
		origin.y = new_point.y;
}

template <typename T>
inline char sign(T n)
{
	if (n == 0)
		return T(0);
	else
		return (n < 0) ? T(-1) : T(1);
}

void _gdimm_text::load_metrics()
{
	int metric_size = GetOutlineTextMetrics(_hdc_text, 0, NULL);
	assert(metric_size != 0);

	if (_metric_buf != NULL)
		delete[] _metric_buf;

	_metric_buf = new BYTE[metric_size];
	_outline_metrics = (OUTLINETEXTMETRIC*) _metric_buf;
	metric_size = GetOutlineTextMetrics(_hdc_text, metric_size, _outline_metrics);
	assert(metric_size != 0);

	_text_height = _outline_metrics->otmTextMetrics.tmHeight - _outline_metrics->otmTextMetrics.tmInternalLeading;
}

void _gdimm_text::get_glyph_clazz()
{
	TCHAR *font_full_name = (TCHAR*)(_metric_buf + (UINT) _outline_metrics->otmpFullName);
	unsigned int font_index = gdimm_font_man::instance().lookup_index(_hdc_text, font_full_name);

	FT_Error ft_error;
	FTC_FaceID face_id = (FTC_FaceID) font_index;

	FT_Face font_face;
	ft_error = FTC_Manager_LookupFace(ft_cache_man, face_id, &font_face);
	assert(ft_error == 0);

	FTC_ScalerRec cache_scale = {face_id, _text_height, _text_height, 1, 0, 0};
	FT_Size font_size;
	ft_error = FTC_Manager_LookupSize(ft_cache_man, &cache_scale, &font_size);
	assert(ft_error == 0);

	FT_Load_Char(font_face, _outline_metrics->otmTextMetrics.tmDefaultChar, FT_LOAD_NO_BITMAP);

	FT_Glyph stub;
	FT_Get_Glyph(font_face->glyph, &stub);
	_glyph_clazz = stub->clazz;
	FT_Done_Glyph(stub);
}

void _gdimm_text::draw_glyph_lcd(const FT_BitmapGlyph glyph) const
{
	const FT_Bitmap bitmap = glyph->bitmap;
	const WORD src_byte_count = 3;	// 24-bit
	const WORD dest_bit_count = 24;
	const WORD dest_byte_count = dest_bit_count / 8;
	const LONG bmp_width = bitmap.width / src_byte_count;
	const LONG bmp_height = bitmap.rows;
	const int abs_pitch = abs(bitmap.pitch);
	const char pitch_sign = sign(bitmap.pitch);

	BITMAPINFO bmi = {0};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biBitCount = dest_bit_count;
	bmi.bmiHeader.biWidth = bmp_width;
	bmi.bmiHeader.biHeight = -1 * pitch_sign * bmp_height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biCompression = BI_RGB;
	
	BYTE *dest_bits;
	HBITMAP dest_bitmap = CreateDIBSection(_hdc_text, &bmi, DIB_RGB_COLORS, (VOID**) &dest_bits, NULL, 0);
	assert(dest_bitmap != NULL);

	for (int j = 0; j < bmp_height; j++)
	{
		for (int i = 0; i < bmp_width; i++)
		{
			int src_ptr = j * abs_pitch + i * src_byte_count;
			int dest_ptr = src_ptr;

			BYTE r = bitmap.buffer[src_ptr];
			BYTE g = bitmap.buffer[src_ptr+1];
			BYTE b = bitmap.buffer[src_ptr+2];
			
			dest_bits[dest_ptr] = (b * _fg_rgb.b + (255 - b) * _bg_rgb.b) / 255;
			dest_bits[dest_ptr+1] = (g * _fg_rgb.g + (255 - g) * _bg_rgb.g) / 255;
			dest_bits[dest_ptr+2] = (r * _fg_rgb.r + (255 - r) * _bg_rgb.r) / 255;
		}
	}

	HDC hdc_canvas = CreateCompatibleDC(_hdc_text);
	assert(hdc_canvas != NULL);
	SelectObject(hdc_canvas, dest_bitmap);

	int x = _cursor.x + glyph->left;
	int y = _cursor.y + _text_height - glyph->top;
	BOOL b_ret = TransparentBlt(_hdc_text, x, y, bmp_width, bmp_height, hdc_canvas, 0, 0, bmp_width, bmp_height, _bg_color);
	assert(b_ret == TRUE);
	
	DeleteObject(dest_bitmap);
	DeleteDC(hdc_canvas);
}

bool _gdimm_text::init(HDC hdc, int x, int y)
{
	TEXTMETRIC text_metric;
	BOOL b_ret = GetTextMetrics(hdc, &text_metric);
	assert(b_ret);

	if ((text_metric.tmPitchAndFamily & TMPF_TRUETYPE) != TMPF_TRUETYPE)
		return false;

	_hdc_text = hdc;
	_cursor.x = x;
	_cursor.y = y;

	// get foreground and background color

	_fg_color = GetTextColor(_hdc_text);
	assert(_fg_color != CLR_INVALID);
	_fg_rgb = REFTORGB(_fg_color);

	_bg_color = GetBkColor(_hdc_text);
	assert(_bg_color != CLR_INVALID);
	_bg_rgb = REFTORGB(_bg_color);

	_metric_buf = NULL;
	load_metrics();
	get_glyph_clazz();

	return true;
}

void _gdimm_text::draw_background(CONST RECT *lprect) const
{
	// get background rect geometry
	const LONG rect_width = lprect->right - lprect->left;
	const LONG rect_height = lprect->bottom - lprect->top;

	// create brush with background color
	COLORREF _bg_color = GetBkColor(_hdc_text);
	assert(_bg_color != CLR_INVALID);
	HBRUSH bg_brush = CreateSolidBrush(_bg_color);
	assert(bg_brush != NULL);

	// select new brush, and store old brush
	HBRUSH old_brush = (HBRUSH) SelectObject(_hdc_text, bg_brush);

	// paint rect with brush
	BOOL ret = PatBlt(_hdc_text, lprect->left, lprect->top, rect_width, rect_height, PATCOPY);
	assert(ret == TRUE);
	DeleteObject(bg_brush);

	// restore old brush
	SelectObject(_hdc_text, old_brush);
}

void _gdimm_text::text_out(const WCHAR *str, unsigned int count, CONST RECT *lprect, CONST INT *lpDx, BOOL is_glyph_index)
{
	// identity matrix
	MAT2 id_mat = {{0, 1}, {0, 0}, {0, 0}, {0, 1}};
	UINT glyph_format = GGO_NATIVE | GGO_UNHINTED;
	if (is_glyph_index != 0)
		glyph_format |= GGO_GLYPH_INDEX;

	for (unsigned int i = 0; i < count; i++)
	{
		DWORD outline_buf_len = GetGlyphOutline(_hdc_text, str[i], glyph_format, &_glyph_metrics, 0, NULL, &id_mat);
		assert(outline_buf_len != GDI_ERROR);

		// some character's glyph outline is empty (e.g. space)
		if (outline_buf_len > 0)
		{
			BYTE *outline_buf = new BYTE[outline_buf_len];
			outline_buf_len = GetGlyphOutline(_hdc_text, str[i], glyph_format, &_glyph_metrics, outline_buf_len, outline_buf, &id_mat);
			assert(outline_buf_len != GDI_ERROR);

			vector<FT_Vector> points;
			vector<char> tags;
			vector<short> contour_pos;

			DWORD header_off = 0;
			FT_Vector outline_origin = {LONG_MAX, LONG_MAX};
			do
			{
				BYTE *header_ptr = outline_buf + header_off;
				TTPOLYGONHEADER *header = (TTPOLYGONHEADER*) header_ptr;
				FT_Vector start_pt = {fix_to_26dot6(header->pfxStart.x), fix_to_26dot6(header->pfxStart.y)};
				points.push_back(start_pt);
				minimize_origin(start_pt, outline_origin);

				DWORD curve_off = sizeof(TTPOLYGONHEADER);
				while (curve_off < header->cb)
				{
					TTPOLYCURVE *curve = (TTPOLYCURVE*)(header_ptr + curve_off);
					char curr_tag;
					switch (curve->wType)
					{
						case TT_PRIM_LINE:
							curr_tag = FT_CURVE_TAG_ON;
							break;
						case TT_PRIM_QSPLINE:
							curr_tag = FT_CURVE_TAG_CONIC;
							break;
						case TT_PRIM_CSPLINE:
							curr_tag = FT_CURVE_TAG_CUBIC;
							break;
						default:
							curr_tag = 0;
					}

					for (int j = 0; j < curve->cpfx; j++)
					{
						FT_Vector curr_pt = {fix_to_26dot6(curve->apfx[j].x), fix_to_26dot6(curve->apfx[j].y)};
						points.push_back(curr_pt);
						tags.push_back(curr_tag);
						minimize_origin(curr_pt, outline_origin);
					}
					
					curve_off += sizeof(TTPOLYCURVE) + (curve->cpfx - 1) * sizeof(POINTFX);
				}

				contour_pos.push_back(points.size() - 1);
				header_off += header->cb;
			} while (header_off < outline_buf_len);

			delete[] outline_buf;
			
			FT_OutlineGlyphRec outline_glyph = 
			{
				{
					ft_lib,
					_glyph_clazz,
					FT_GLYPH_FORMAT_OUTLINE,
					0,
					0
				},
				{
					contour_pos.size(),
					points.size(),
					&points[0],
					&tags[0],
					&contour_pos[0],
					FT_OUTLINE_NONE
				}
			};

			FT_Glyph generic_glyph = (FT_Glyph) &outline_glyph;
			outline_origin.x = -outline_origin.x;
			outline_origin.y = -outline_origin.y;
			FT_Glyph_To_Bitmap(&generic_glyph, FT_RENDER_MODE_LCD, NULL, FALSE);
			FT_BitmapGlyph bmp_glyph = (FT_BitmapGlyph) generic_glyph;

			draw_glyph_lcd(bmp_glyph);
		}

		// advance cursor
		if (lpDx == NULL)
			_cursor.x += _glyph_metrics.gmCellIncX;
		else
			_cursor.x += lpDx[i];

		_cursor.y += _glyph_metrics.gmCellIncY;
	}
}