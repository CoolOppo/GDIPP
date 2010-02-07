#include "stdafx.h"
#include "text.h"
#include "ft.h"
#include "font_man.h"
#include "font_link.h"
#include FT_INTERNAL_OBJECTS_H	// for FT_PAD_CEIL
#include <cmath>
#include <vector>
using namespace std;

const FT_Glyph_Class *glyph_clazz = NULL;

inline FT_F26Dot6 fix_to_26dot6(const FIXED &fx)
{
	// convert 16.16 fixed float type to 26.6 format
	return *((FT_F26Dot6*) &fx) >> 10;
}

_gdimm_text::_gdimm_text()
{
	_metric_buf = NULL;
}

_gdimm_text::~_gdimm_text()
{
	if (_metric_buf != NULL)
		delete[] _metric_buf;
}

bool _gdimm_text::get_metrics()
{
	// load outline metrics, which also include the text metrics, of the DC

	int metric_size = GetOutlineTextMetrics(_hdc_text, 0, NULL);
	if (metric_size == 0)
		return false;

	if (_metric_buf != NULL)
		delete[] _metric_buf;

	_metric_buf = new BYTE[metric_size];
	_outline_metrics = (OUTLINETEXTMETRIC*) _metric_buf;
	metric_size = GetOutlineTextMetrics(_hdc_text, metric_size, _outline_metrics);
	assert(metric_size != 0);

	return true;
}

void _gdimm_text::get_glyph_clazz()
{
	/*
	glyph clazz is a private field, cannot be constructed through FreeType API
	instead, we load the glyph of the default character from the current font
	use the clazz for all subsequent FT_OutlineGlyph
	*/

	const TCHAR *font_full_name = get_full_name();
	unsigned int font_index = gdimm_font_man::instance().lookup_index(_hdc_text, font_full_name);

	FT_Error ft_error;
	const FTC_FaceID face_id = (FTC_FaceID) font_index;

	FT_Face font_face;
	ft_error = FTC_Manager_LookupFace(ft_cache_man, face_id, &font_face);
	assert(ft_error == 0);

	FTC_ScalerRec cache_scale = {face_id, 0, 0, 1, 0, 0};
	FT_Size font_size;
	ft_error = FTC_Manager_LookupSize(ft_cache_man, &cache_scale, &font_size);
	assert(ft_error == 0);

	ft_error = FT_Load_Char(font_face, _outline_metrics->otmTextMetrics.tmDefaultChar, FT_LOAD_NO_BITMAP);
	assert(ft_error == 0);

	FT_Glyph useless;
	FT_Get_Glyph(font_face->glyph, &useless);
	glyph_clazz = useless->clazz;
	FT_Done_Glyph(useless);
}

void _gdimm_text::draw_background(CONST RECT *lprect) const
{
	COLORREF _bg_color = GetBkColor(_hdc_text);
	assert(_bg_color != CLR_INVALID);

	const HBRUSH bg_brush = CreateSolidBrush(_bg_color);
	assert(bg_brush != NULL);

	int i_ret = FillRect(_hdc_text, lprect, bg_brush);
	assert(i_ret != 0);

	DeleteObject(bg_brush);
}

void _gdimm_text::set_bmp_bits_mono(const FT_Bitmap &src_bitmap, BYTE *dest_bits, bool is_dest_up, bool inverted) const
{
	// both bitmap are 1-bit, 8 pixels per byte, in most-significant order

	const LONG bmp_width = src_bitmap.width;
	const LONG bmp_height = src_bitmap.rows;
	const int src_pitch = abs(src_bitmap.pitch);
	const int dest_pitch = FT_PAD_CEIL(max(bmp_width / 8, 1), sizeof(DWORD));

	for (int j = 0; j < bmp_height; j++)
	{
		for (int i = 0; i < bmp_width; i++)
		{
			int src_ptr = i / 8;
			if (src_bitmap.pitch > 0)
				src_ptr += j * src_pitch;
			else
				src_ptr += (bmp_height - j - 1) * src_pitch;
			
			int dest_ptr = i / 8;
			if (is_dest_up)
				dest_ptr += (bmp_height - j - 1) * dest_pitch;
			else
				dest_ptr += j * dest_pitch;

			const int bit_pos = 7 - i % 8;
			const bool bit_set = ((src_bitmap.buffer[src_ptr] & (1 << bit_pos)) != 0);

			if (bit_set != inverted)
				dest_bits[dest_ptr] |= (1 << bit_pos);
			else
				dest_bits[dest_ptr] &= ~(1 << bit_pos);
		}
	}
}

void _gdimm_text::set_bmp_bits_gray(const FT_Bitmap &src_bitmap, BYTE *dest_bits, bool is_dest_up) const
{
	// both bitmaps are 8-bit, 1 byte per pixel

	const WORD src_byte_count = 1;
	const LONG bmp_width = src_bitmap.width / src_byte_count;
	const LONG bmp_height = src_bitmap.rows;
	const int src_pitch = abs(src_bitmap.pitch);
	const WORD dest_byte_count = 3;
	const int dest_pitch = FT_PAD_CEIL(bmp_width * dest_byte_count, sizeof(DWORD));

	for (int j = 0; j < bmp_height; j++)
	{
		for (int i = 0; i < bmp_width; i++)
		{
			int src_ptr = i * src_byte_count;
			if (src_bitmap.pitch > 0)
				src_ptr += j * src_pitch;
			else
				src_ptr += (bmp_height - j - 1) * src_pitch;
			
			int dest_ptr = i * dest_byte_count;
			if (is_dest_up)
				dest_ptr += (bmp_height - j - 1) * dest_pitch;
			else
				dest_ptr += j * dest_pitch;

			const BYTE gray_level = src_bitmap.buffer[src_ptr];
			dest_bits[dest_ptr] = (gray_level * gray_level + (255 - gray_level) * dest_bits[dest_ptr]) / 255;
		}
	}
}

void _gdimm_text::set_bmp_bits_lcd(const FT_Bitmap &src_bitmap, BYTE *dest_bits, bool is_dest_up) const
{
	// the source bitmap is 24-bit, 3 bytes per pixel, in order of R, G, B channels
	// the destination bitmap is 24-bit, 3 bytes per pixel, in order of B, G, R channels

	const WORD src_byte_count = 3;
	const LONG bmp_width = src_bitmap.width / src_byte_count;
	const LONG bmp_height = src_bitmap.rows;
	const int src_pitch = abs(src_bitmap.pitch);
	const WORD dest_byte_count = 3;
	const int dest_pitch = FT_PAD_CEIL(bmp_width * dest_byte_count, sizeof(DWORD));

	for (int j = 0; j < bmp_height; j++)
	{
		for (int i = 0; i < bmp_width; i++)
		{
			int src_ptr = i * src_byte_count;
			if (src_bitmap.pitch > 0)
				src_ptr += j * src_pitch;
			else
				src_ptr += (bmp_height - j - 1) * src_pitch;
			
			int dest_ptr = i * dest_byte_count;
			if (is_dest_up)
				dest_ptr += (bmp_height - j - 1) * dest_pitch;
			else
				dest_ptr += j * dest_pitch;

			const BYTE r = src_bitmap.buffer[src_ptr];
			const BYTE g = src_bitmap.buffer[src_ptr+1];
			const BYTE b = src_bitmap.buffer[src_ptr+2];
			
			dest_bits[dest_ptr] = (b * _fg_rgb.rgbBlue + (255 - b) * dest_bits[dest_ptr]) / 255;
			dest_bits[dest_ptr+1] = (g * _fg_rgb.rgbGreen + (255 - g) * dest_bits[dest_ptr+1]) / 255;
			dest_bits[dest_ptr+2] = (r * _fg_rgb.rgbRed + (255 - r) * dest_bits[dest_ptr+2]) / 255;
		}
	}
}

void _gdimm_text::set_bmp_bits_alpha(const FT_Bitmap &src_bitmap, BYTE *dest_bits, bool is_dest_up) const
{
	// the destination bitmap is 32-bit, 4 bytes per pixel, in order of B, G, R, A channels
	// each row is aligned to DWORD

	const WORD src_byte_count = 3;
	const LONG bmp_width = src_bitmap.width / src_byte_count;
	const LONG bmp_height = src_bitmap.rows;
	const int src_pitch = abs(src_bitmap.pitch);
	const WORD dest_byte_count = 4;
	const int dest_pitch = FT_PAD_CEIL(bmp_width * dest_byte_count, sizeof(DWORD));

	for (int j = 0; j < bmp_height; j++)
	{
		for (int i = 0; i < bmp_width; i++)
		{
			int src_ptr = i * src_byte_count;
			if (src_bitmap.pitch > 0)
				src_ptr += j * src_pitch;
			else
				src_ptr += (bmp_height - j - 1) * src_pitch;
			
			int dest_ptr = i * dest_byte_count;
			if (is_dest_up)
				dest_ptr += (bmp_height - j - 1) * dest_pitch;
			else
				dest_ptr += j * dest_pitch;

			const BYTE r = src_bitmap.buffer[src_ptr];
			const BYTE g = src_bitmap.buffer[src_ptr+1];
			const BYTE b = src_bitmap.buffer[src_ptr+2];
			
			dest_bits[dest_ptr] = (b * _fg_rgb.rgbBlue + (255 - b) * dest_bits[dest_ptr]) / 255;
			dest_bits[dest_ptr+1] = (g * _fg_rgb.rgbGreen + (255 - g) * dest_bits[dest_ptr+1]) / 255;
			dest_bits[dest_ptr+2] = (r * _fg_rgb.rgbRed + (255 - r) * dest_bits[dest_ptr+2]) / 255;
			dest_bits[dest_ptr+3] = 0;
		}
	}
}

WORD _gdimm_text::create_bitmap(FT_Glyph *glyph_ptr, BITMAPINFO *&bmi) const
{
	// create glyph bitmap base on the destination bitmap format

	int lines_ret;
	BOOL b_ret;

	const HDC hdc_canvas = CreateCompatibleDC(_hdc_text);
	assert(hdc_canvas != NULL);

	const HBITMAP dest_bitmap = CreateCompatibleBitmap(_hdc_text, 1, 1);
	assert(dest_bitmap != NULL);
	
	SelectObject(hdc_canvas, dest_bitmap);
	b_ret = BitBlt(hdc_canvas, 0, 0, 1, 1, _hdc_text, _cursor.x, _cursor.y, SRCCOPY);
	assert(b_ret);
	
	bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lines_ret = GetDIBits(hdc_canvas, dest_bitmap, 0, 1, NULL, bmi, DIB_RGB_COLORS);
	assert(lines_ret != 0);

	if (bmi->bmiHeader.biClrUsed > 1)
	{
		// the next GetDIBits will alloc as many RGBQUAD to the color table as biClrUsed specifies
		// where the color table only reserves 1 RGBQUAD
		// need to realloc the BITMAPINFO

		bmi = (BITMAPINFO*) realloc(bmi, sizeof(BITMAPINFOHEADER) + bmi->bmiHeader.biClrUsed * sizeof(RGBQUAD));
		assert(bmi != NULL);
	}
	else if (bmi->bmiHeader.biCompression == BI_BITFIELDS && bmi->bmiHeader.biBitCount % 16 == 0)
	{
		// the next GetDIBits will alloc 3 RGBQUAD to the color table
		// need to realloc the BITMAPINFO				

		bmi = (BITMAPINFO*) realloc(bmi, sizeof(BITMAPINFOHEADER) + 3 * sizeof(RGBQUAD));
		assert(bmi != NULL);
	}

	FT_Render_Mode render_mode;
	WORD src_bit_count;
	switch (bmi->bmiHeader.biBitCount)
	{
	case 1:
		render_mode = FT_RENDER_MODE_MONO;
		src_bit_count = 1;
		break;
	case 8:
		render_mode = FT_RENDER_MODE_NORMAL;
		src_bit_count = 8;
		break;
	case 24:
	case 32:
		render_mode = FT_RENDER_MODE_LCD;
		src_bit_count = 24;
		break;
	}

	FT_Error ft_error = FT_Glyph_To_Bitmap(glyph_ptr, render_mode, NULL, FALSE);
	assert(ft_error == 0);

	DeleteObject(dest_bitmap);
	DeleteDC(hdc_canvas);

	return src_bit_count;
}

void _gdimm_text::draw_glyph(const FT_BitmapGlyph bmp_glyph, WORD src_bit_count, BITMAPINFO *bmi) const
{
	int lines_ret;
	BOOL b_ret;

	const FT_Bitmap &src_bitmap = bmp_glyph->bitmap;
	const WORD src_byte_count = max(src_bit_count / 8, 1);
	const LONG bmp_width = src_bitmap.width / src_byte_count;
	const LONG bmp_height = src_bitmap.rows;	
	const int x = _cursor.x + bmp_glyph->left;
	const int y = _cursor.y - bmp_glyph->top;

	/*
	the way we draw bitmap to DC is
	1. create compatible DC and bitmap of the physical DC
	2. capture the bitmap of the physical DC by calling BitBlt
	3. get the physical DC bitmap bits by calling GetDIBits
	4. draw the glyph bitmap over the physical DC bitmap
	5. draw the updated physical DC bitmap back to the physical DC
	*/

	const HDC hdc_canvas = CreateCompatibleDC(_hdc_text);
	assert(hdc_canvas != NULL);

	const HBITMAP dest_bitmap = CreateCompatibleBitmap(_hdc_text, bmp_width, bmp_height);
	assert(dest_bitmap != NULL);
	SelectObject(hdc_canvas, dest_bitmap);

	b_ret = BitBlt(hdc_canvas, 0, 0, bmp_width, bmp_height, _hdc_text, x, y, SRCCOPY);
	assert(b_ret);

	bmi->bmiHeader.biWidth = bmp_width;
	bmi->bmiHeader.biHeight = bmp_height;
	bmi->bmiHeader.biSizeImage = FT_PAD_CEIL(max(bmp_width * bmi->bmiHeader.biBitCount / 8, 1), sizeof(DWORD)) * bmp_height;

	BYTE *dest_bits = new BYTE[bmi->bmiHeader.biSizeImage];
	lines_ret = GetDIBits(hdc_canvas, dest_bitmap, 0, bmp_height, dest_bits, bmi, DIB_RGB_COLORS);
	assert(lines_ret == bmp_height);

	/*
	Windows DIB and FreeType Bitmap have different ways to indicate bitmap direction
	biHeight > 0 means the Windows DIB is bottom-up
	biHeight < 0 means the Windows DIB is top-down
	pitch > 0 means the FreeType bitmap is down flow
	pitch > 0 means the FreeType bitmap is up flow
	*/

	switch (bmi->bmiHeader.biBitCount)
	{
	case 1:
		set_bmp_bits_mono(src_bitmap, dest_bits, bmi->bmiHeader.biHeight > 0, *((WORD*) &_fg_rgb) == *((WORD*) bmi->bmiColors));
		break;
	case 8:
		set_bmp_bits_gray(src_bitmap, dest_bits, bmi->bmiHeader.biHeight > 0);
		break;
	case 24:
		set_bmp_bits_lcd(src_bitmap, dest_bits, bmi->bmiHeader.biHeight > 0);
		break;
	case 32:
		set_bmp_bits_alpha(src_bitmap, dest_bits, bmi->bmiHeader.biHeight > 0);
		break;
	}

	lines_ret = SetDIBitsToDevice(_hdc_text, x, y, bmp_width, bmp_height, 0, 0, 0, bmp_height, dest_bits, bmi, DIB_RGB_COLORS);
	assert(lines_ret == bmp_height);

	delete[] dest_bits;
	DeleteObject(dest_bitmap);
	DeleteDC(hdc_canvas);
}

const TCHAR *_gdimm_text::get_family_name() const
{
	return (const TCHAR*)(_metric_buf + (UINT) _outline_metrics->otmpFamilyName);
}

const TCHAR *_gdimm_text::get_full_name() const
{
	return (const TCHAR*)(_metric_buf + (UINT) _outline_metrics->otmpFullName);
}

bool _gdimm_text::init(HDC hdc, int x, int y, UINT options)
{
	_hdc_text = hdc;

	if (!get_metrics())
		return false;
	
	// since we only deal with fonts with outlines, the glyph clazz must be ft_outline_glyph_class
	// therefore this function is called only once
	if (glyph_clazz == NULL)
		get_glyph_clazz();

	// get foreground color
	_fg_color = GetTextColor(_hdc_text);
	assert(_fg_color != CLR_INVALID);
	_fg_rgb.rgbBlue = GetBValue(_fg_color);
	_fg_rgb.rgbGreen = GetGValue(_fg_color);
	_fg_rgb.rgbRed = GetRValue(_fg_color);
	_fg_rgb.rgbReserved = 0;
	
	_text_alignment = GetTextAlign(_hdc_text);
	assert(_text_alignment != GDI_ERROR);
	
	if (((TA_NOUPDATECP | TA_UPDATECP) & _text_alignment) == TA_UPDATECP)
	{
		POINT cp;
		GetCurrentPositionEx(_hdc_text, &cp);
		_cursor.x = cp.x;
		_cursor.y = cp.y;
		_update_cursor = true;
	}
	else
	{
		_cursor.x = x;
		_cursor.y = y;
		_update_cursor = false;
	}

	_eto_options = options;

	return true;
}

/*bool _gdimm_text::to_glyph_indices(const WCHAR *text, unsigned int count, WORD *glyph_indices)
{
	DWORD dw_ret;

	int fl_index = 0;
	while (true)
	{
		dw_ret = GetGlyphIndices(_hdc_text, text, count, glyph_indices, GGI_MARK_NONEXISTING_GLYPHS);
		assert(dw_ret != GDI_ERROR);

		if (glyph_indices[0] != 0xffff)
			break;

		if (!gdimm_font_link::instance().font_link(_hdc_text, get_family_name(), fl_index++))
			return false;
	}

	// redo DC font related procedures
	get_metrics();

	gdimm_font_link::instance().restore_font(_hdc_text);
	return true;
}*/

void _gdimm_text::text_out(const WCHAR *string, unsigned int count, CONST RECT *lprect, CONST INT *lpDx)
{
	BOOL b_ret;

	SIZE text_extent;
	if (_eto_options & ETO_GLYPH_INDEX)
	{
		b_ret = GetTextExtentPointI(_hdc_text, (WORD*) string, count, &text_extent);
		assert(b_ret);
	}
	else
	{
		b_ret = GetTextExtentPoint32(_hdc_text, string, count, &text_extent);
		assert(b_ret);
	}

	// draw background if needed

	if (_eto_options & ETO_OPAQUE)
	{
		assert(lprect != NULL);
		draw_background(lprect);
	}

	if (GetBkMode(_hdc_text) == OPAQUE)
	{
		RECT text_rect;
		b_ret = SetRect(&text_rect, _cursor.x, _cursor.y, _cursor.x + text_extent.cx, _cursor.y + text_extent.cy);
		assert(b_ret);

		if (_eto_options & ETO_CLIPPED)
			IntersectRect(&text_rect, &text_rect, lprect);

		draw_background(&text_rect);
	}
	
	// is ETO_PDY is set, lpDx contains both x increment and y displacement
	const int dx_factor = ((_eto_options & ETO_PDY) ? 2 : 1);

	// identity matrix
	const MAT2 id_mat = {{0, 1}, {0, 0}, {0, 0}, {0, 1}};
	UINT glyph_format = GGO_NATIVE | GGO_UNHINTED;
	if (_eto_options & ETO_GLYPH_INDEX)
		glyph_format |= GGO_GLYPH_INDEX;

	switch ((TA_LEFT | TA_RIGHT | TA_CENTER) & _text_alignment)
	{
	case TA_LEFT:
		break;
	case TA_RIGHT:
		_cursor.x -= text_extent.cx;
		break;
	case TA_CENTER:
		_cursor.x -= text_extent.cx / 2;
		break;
	}

	switch ((TA_TOP | TA_BOTTOM | TA_BASELINE) & _text_alignment)
	{
	case TA_TOP:
		_cursor.y += _outline_metrics->otmTextMetrics.tmAscent;
		break;
	case TA_BOTTOM:
		_cursor.y -= _outline_metrics->otmTextMetrics.tmDescent;
		break;
	case TA_BASELINE:
		break;
	}

	for (unsigned int i = 0; i < count; i++)
	{
		GLYPHMETRICS glyph_metrics;

		// get glyph metrics
		DWORD outline_buf_len = GetGlyphOutline(_hdc_text, string[i], glyph_format, &glyph_metrics, 0, NULL, &id_mat);
		assert(outline_buf_len != GDI_ERROR);

		// some character's glyph outline is empty (e.g. space), skip
		if (outline_buf_len > 0)
		{
			BYTE *outline_buf = new BYTE[outline_buf_len];
			outline_buf_len = GetGlyphOutline(_hdc_text, string[i], glyph_format, &glyph_metrics, outline_buf_len, outline_buf, &id_mat);
			assert(outline_buf_len != GDI_ERROR);

			vector<FT_Vector> points;
			vector<char> tags;
			vector<short> contour_pos;

			// parse outline glyph_indicesuctures
			DWORD header_off = 0;
			do
			{
				const BYTE *header_ptr = outline_buf + header_off;
				const TTPOLYGONHEADER *header = (TTPOLYGONHEADER*) header_ptr;

				// FreeType uses 26.6 format, while Windows gives logical units
				const FT_Vector start_pt = {fix_to_26dot6(header->pfxStart.x), fix_to_26dot6(header->pfxStart.y)};
				points.push_back(start_pt);
				tags.push_back(FT_CURVE_TAG_ON);

				DWORD curve_off = sizeof(TTPOLYGONHEADER);
				while (curve_off < header->cb)
				{
					const TTPOLYCURVE *curve = (TTPOLYCURVE*)(header_ptr + curve_off);
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
						const FT_Vector curr_pt = {fix_to_26dot6(curve->apfx[j].x), fix_to_26dot6(curve->apfx[j].y)};
						points.push_back(curr_pt);
						tags.push_back(curr_tag);
					}
					
					curve_off += sizeof(TTPOLYCURVE) + (curve->cpfx - 1) * sizeof(POINTFX);
				}

				contour_pos.push_back(points.size() - 1);
				header_off += header->cb;
			} while (header_off < outline_buf_len);

			assert(points.size() == tags.size());
			delete[] outline_buf;
			
			/*
			once in possess of FT_Outline, there are several way to get FT_Bitmap

			1. FT_Outline_Render: could pass a callback span function to directly draw scanlines to DC
			   unfortunately it only output 8-bit bitmap
			2. FT_Outline_Get_Bitmap: merely a wrapper of FT_Outline_Render
			3. FT_Glyph_To_Bitmap: first conglyph_indicesuct FT_OutlineGlyph from FT_Outline, then render glyph to get FT_Bitmap
			   when conglyph_indicesucting FreeType glyph, the private clazz field must be provided
			   support 24-bit bitmap

			we use method 3
			*/
			
			const FT_OutlineGlyphRec outline_glyph = 
			{
				{
					ft_lib,
					glyph_clazz,
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
			BITMAPINFO *bmi = (BITMAPINFO*) calloc(1, sizeof(BITMAPINFO));

			WORD bit_count = create_bitmap(&generic_glyph, bmi);
			draw_glyph((FT_BitmapGlyph) generic_glyph, bit_count, bmi);

			free(bmi);
			FT_Done_Glyph(generic_glyph);
			// the FT_OutlineGlyph is manually conglyph_indicesucted, no need to destroy it
		}

		// advance cursor
		if (lpDx == NULL)
			_cursor.x += glyph_metrics.gmCellIncX;
		else
			_cursor.x += lpDx[i * dx_factor];

		_cursor.y += glyph_metrics.gmCellIncY;
	}

	// if TA_UPDATECP is set, update current position after text out
	if (_update_cursor)
	{
		b_ret = MoveToEx(_hdc_text, _cursor.x, _cursor.y, NULL);
		assert(b_ret);
	}
}