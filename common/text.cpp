#include "stdafx.h"
#include "text.h"
#include "global.h"
#include "ft.h"
#include <cmath>
#include <limits.h>
#include <vector>
using namespace std;

struct COLORRGB
{
	BYTE r;
	BYTE g;
	BYTE b;
	BYTE pad;
};
#define REFTORGB(ref) (*((COLORRGB*) &ref))

#define AlignDown(num, alignment) (num - num & (alignment -	1))

inline int AlignUp(int num, int alignment)
{
	if (num == 0)
		return alignment;
	else if (num % alignment == 0)
		return num;
	else
		return (num / alignment + 1) * alignment;
}

FT_F26Dot6 _gdimm_text::fix_to_26dot6(const FIXED &fx)
{
	return *(LONG*)(&fx) >> 10;
}

void _gdimm_text::minimize_origin(const FT_Vector &new_pt, FT_Vector &origin)
{
	if (new_pt.x < origin.x)
		origin.x = new_pt.x;
	
	if (new_pt.y < origin.y)
		origin.y = new_pt.y;
}

void _gdimm_text::prepare_bitmap(WORD bit_count)
{
	BITMAPINFO bmi = {0};

	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biBitCount = bit_count;
	bmi.bmiHeader.biWidth = _glyph_metrics.gmBlackBoxX;
	bmi.bmiHeader.biHeight = _glyph_metrics.gmBlackBoxY;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biCompression = BI_RGB;

	_hbmp = CreateDIBSection(_hdc_text, &bmi, DIB_RGB_COLORS, (VOID**) &_bmp_buf, NULL, 0);
	assert(_hbmp != NULL);

	memset(_bmp_buf, _bg_color, AlignUp(_glyph_metrics.gmBlackBoxX * 3, sizeof(DWORD)) * _glyph_metrics.gmBlackBoxY);
}

void bitmap_span(int y, int count, const FT_Span *spans, void *user)
{
	_gdimm_text *test = (_gdimm_text*) user;
	const COLORRGB fg_rgb = REFTORGB(test->_fg_color);
	const COLORRGB bg_rgb = REFTORGB(test->_bg_color);

	for (int i = 0; i < count; i++)
	{
        BYTE b = (spans[i].coverage * fg_rgb.b + (255 - spans[i].coverage) * bg_rgb.b) / 255;
        BYTE g = (spans[i].coverage * fg_rgb.g + (255 - spans[i].coverage) * bg_rgb.g) / 255;
        BYTE r = (spans[i].coverage * fg_rgb.r + (255 - spans[i].coverage) * bg_rgb.r) / 255;

		int bmp_buf_ptr = AlignUp(test->_glyph_metrics.gmBlackBoxX * 3, sizeof(DWORD)) * y + spans[i].x * 3;
		for (int j = 0; j < spans[i].len; j++)
		{
			test->_bmp_buf[bmp_buf_ptr++] = b;
			test->_bmp_buf[bmp_buf_ptr++] = g;
			test->_bmp_buf[bmp_buf_ptr++] = r;
		}
	}
}

void _gdimm_text::draw_bitmap()
{
	HDC hdc_canvas = CreateCompatibleDC(_hdc_text);
	assert(hdc_canvas != NULL);

	SelectObject(hdc_canvas, _hbmp);

	BOOL b_ret = TransparentBlt(
		_hdc_text,
		_cursor.x + _glyph_metrics.gmptGlyphOrigin.x,
		_cursor.y + _text_height - _glyph_metrics.gmptGlyphOrigin.y,
		_glyph_metrics.gmBlackBoxX,
		_glyph_metrics.gmBlackBoxY,
		hdc_canvas,
		0,
		0,
		_glyph_metrics.gmBlackBoxX,
		_glyph_metrics.gmBlackBoxY,
		_bg_color);
	assert(b_ret == TRUE);

	DeleteObject(_hbmp);
	DeleteDC(hdc_canvas);
}

void _gdimm_text::init(HDC hdc, int x, int y)
{
	_hdc_text = hdc;
	_cursor.x = x;
	_cursor.y = y;

	// get foreground and background color
	_fg_color = GetTextColor(_hdc_text);
	assert(_fg_color != CLR_INVALID);
	_bg_color = GetBkColor(_hdc_text);
	assert(_bg_color != CLR_INVALID);

	GetTextMetrics(_hdc_text, &_text_metrics);
}

BOOL _gdimm_text::is_true_type() const
{
	return (_text_metrics.tmPitchAndFamily & TMPF_TRUETYPE);
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
	_text_height = _text_metrics.tmHeight - _text_metrics.tmInternalLeading;

	// identity matrix
	MAT2 id_mat = {{0, 1}, {0, 0}, {0, 0}, {0, 1}};
	UINT glyph_format = GGO_NATIVE;
	if (is_glyph_index != 0)
		glyph_format |= GGO_GLYPH_INDEX;

	for (unsigned int i = 0; i < count; i++)
	{
		DWORD outline_buf_len = GetGlyphOutline(_hdc_text, str[i], glyph_format, &_glyph_metrics, 0, NULL, &id_mat);
		assert(outline_buf_len != GDI_ERROR);
		/*if (outline_buf_len == GDI_ERROR)
		{
			debug_output(GetLastError());
			continue;
		}*/

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
			
			FT_Outline outline = 
			{
				contour_pos.size(),
				points.size(),
				&points[0],
				&tags[0],
				&contour_pos[0],
				FT_OUTLINE_NONE
			};

			if (outline_origin.x != 0 || outline_origin.y != 0)
				FT_Outline_Translate(&outline, -outline_origin.x, -outline_origin.y);

			prepare_bitmap(24);

			FT_Raster_Params param = {0};
			param.source = &outline;
			param.flags = FT_RASTER_FLAG_AA | FT_RASTER_FLAG_DIRECT;
			param.gray_spans = bitmap_span;
			param.user = this;

			FT_Error ft_error = FT_Outline_Render(ft_lib, &outline, &param);
			assert(ft_error == 0);

			draw_bitmap();
		}

		// advance cursor
		if (lpDx == NULL)
			_cursor.x += _glyph_metrics.gmCellIncX;
		else
			_cursor.x += lpDx[i];

		_cursor.y += _glyph_metrics.gmCellIncY;
	}
}