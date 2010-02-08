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

WORD _gdimm_text::get_bmp_bit_count(WORD dc_bit_count)
{
	switch (dc_bit_count)
	{
	case 1:
		return 1;
	case 8:
		return 8;
	case 24:
	case 32:
		return 24;
	default:
		return 0;
	}
}

FT_Render_Mode _gdimm_text::get_render_mode(WORD dc_bit_count)
{
	switch (dc_bit_count)
	{
	case 1:
		return FT_RENDER_MODE_MONO;
	case 8:
		return FT_RENDER_MODE_NORMAL;
	case 24:
	case 32:
		return FT_RENDER_MODE_LCD;
	default:
		return FT_RENDER_MODE_NORMAL;
	}
}

bitmap_glyph_metrics _gdimm_text::get_glyph_metrics(const vector<FT_BitmapGlyph> &glyphs, const vector<POINT> &positions, WORD bit_count)
{
	const WORD byte_count = max(bit_count / 8, 1);
	bitmap_glyph_metrics metrics = {0};

	for (unsigned int i = 0; i < glyphs.size(); i++)
	{
		metrics.ascent = max(metrics.ascent, glyphs[i]->top);
		metrics.descent = max(metrics.descent, glyphs[i]->bitmap.rows - glyphs[i]->top);
	}

	const int last_index = glyphs.size() - 1;
	metrics.width = positions[last_index].x + glyphs[last_index]->bitmap.width / byte_count - positions[0].x;
	metrics.height = metrics.ascent + metrics.descent;
	metrics.bearing_x = glyphs[0]->left;

	return metrics;
}

void _gdimm_text::draw_background(HDC hdc, CONST RECT *bg_rect, COLORREF color)
{
    const HBRUSH bg_brush = CreateSolidBrush(color);
    assert(bg_brush != NULL);

	int i_ret = FillRect(hdc, bg_rect, bg_brush);
    assert(i_ret != 0);

    DeleteObject(bg_brush);
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

void _gdimm_text::set_bmp_bits_mono(const FT_Bitmap &src_bitmap, BYTE *dest_bits, int dest_x, int dest_y, int dest_width, int dest_height, bool is_dest_up, bool inverted) const
{
	// both bitmap are 1-bit, 8 pixels per byte, in most-significant order

	const LONG src_width = src_bitmap.width;
	const LONG src_height = src_bitmap.rows;
	const int src_pitch = abs(src_bitmap.pitch);
	const int dest_pitch = FT_PAD_CEIL(max(dest_width / 8, 1), sizeof(DWORD));

	for (int j = 0; j < src_height; j++)
	{
		for (int i = 0; i < src_width; i++)
		{
			int src_ptr = i / 8;
			if (src_bitmap.pitch > 0)
				src_ptr += j * src_pitch;
			else
				src_ptr += (src_height - j - 1) * src_pitch;
			
			int dest_ptr = (dest_x + i) / 8;
			if (is_dest_up)
				dest_ptr += (dest_height - dest_y - j - 1) * dest_pitch;
			else
				dest_ptr += (dest_y + j) * dest_pitch;

			const BYTE bit_pos = 7 - i % 8;
			const bool bit_set = ((src_bitmap.buffer[src_ptr] & (1 << bit_pos)) != 0);

			if (bit_set != inverted)
				dest_bits[dest_ptr] |= (1 << bit_pos);
			else
				dest_bits[dest_ptr] &= ~(1 << bit_pos);
		}
	}
}

void _gdimm_text::set_bmp_bits_gray(const FT_Bitmap &src_bitmap, BYTE *dest_bits, int dest_x, int dest_y, int dest_width, int dest_height, bool is_dest_up) const
{
	// both bitmaps are 8-bit, 1 byte per pixel

	const WORD src_byte_count = 1;
	const LONG src_width = src_bitmap.width / src_byte_count;
	const LONG src_height = src_bitmap.rows;
	const int src_pitch = abs(src_bitmap.pitch);
	const WORD dest_byte_count = 3;
	const int dest_pitch = FT_PAD_CEIL(dest_width * dest_byte_count, sizeof(DWORD));

	for (int j = 0; j < src_height; j++)
	{
		for (int i = 0; i < src_width; i++)
		{
			int src_ptr = i * src_byte_count;
			if (src_bitmap.pitch > 0)
				src_ptr += j * src_pitch;
			else
				src_ptr += (src_height - j - 1) * src_pitch;
			
			int dest_ptr = (dest_x + i) * dest_byte_count;
			if (is_dest_up)
				dest_ptr += (dest_height - dest_y - j - 1) * dest_pitch;
			else
				dest_ptr += (dest_y + j) * dest_pitch;

			const BYTE gray_level = src_bitmap.buffer[src_ptr];
			dest_bits[dest_ptr] = (gray_level * gray_level + (255 - gray_level) * dest_bits[dest_ptr]) / 255;
		}
	}
}

void _gdimm_text::set_bmp_bits_lcd(const FT_Bitmap &src_bitmap, BYTE *dest_bits, int dest_x, int dest_y, int dest_width, int dest_height, bool is_dest_up) const
{
	// the source bitmap is 24-bit, 3 bytes per pixel, in order of R, G, B channels
	// the destination bitmap is 24-bit, 3 bytes per pixel, in order of B, G, R channels

	const WORD src_byte_count = 3;
	const LONG src_width = src_bitmap.width / src_byte_count;
	const LONG src_height = src_bitmap.rows;
	const int src_pitch = abs(src_bitmap.pitch);
	const WORD dest_byte_count = 3;
	const int dest_pitch = FT_PAD_CEIL(dest_width * dest_byte_count, sizeof(DWORD));

	for (int j = 0; j < src_height; j++)
	{
		for (int i = 0; i < src_width; i++)
		{
			int src_ptr = i * src_byte_count;
			if (src_bitmap.pitch > 0)
				src_ptr += j * src_pitch;
			else
				src_ptr += (src_height - j - 1) * src_pitch;
			
			int dest_ptr = (dest_x + i) * dest_byte_count;
			if (is_dest_up)
				dest_ptr += (dest_height - dest_y - j - 1) * dest_pitch;
			else
				dest_ptr += (dest_y + j) * dest_pitch;

			const BYTE r = src_bitmap.buffer[src_ptr];
			const BYTE g = src_bitmap.buffer[src_ptr+1];
			const BYTE b = src_bitmap.buffer[src_ptr+2];
			
			dest_bits[dest_ptr] = (b * _fg_rgb.rgbBlue + (255 - b) * dest_bits[dest_ptr]) / 255;
			dest_bits[dest_ptr+1] = (g * _fg_rgb.rgbGreen + (255 - g) * dest_bits[dest_ptr+1]) / 255;
			dest_bits[dest_ptr+2] = (r * _fg_rgb.rgbRed + (255 - r) * dest_bits[dest_ptr+2]) / 255;
		}
	}
}

void _gdimm_text::set_bmp_bits_alpha(const FT_Bitmap &src_bitmap, BYTE *dest_bits, int dest_x, int dest_y, int dest_width, int dest_height, bool is_dest_up) const
{
	// the destination bitmap is 32-bit, 4 bytes per pixel, in order of B, G, R, A channels
	// each row is aligned to DWORD

	const WORD src_byte_count = 3;
	const LONG src_width = src_bitmap.width / src_byte_count;
	const LONG src_height = src_bitmap.rows;
	const int src_pitch = abs(src_bitmap.pitch);
	const WORD dest_byte_count = 4;
	const int dest_pitch = FT_PAD_CEIL(dest_width * dest_byte_count, sizeof(DWORD));

	for (int j = 0; j < src_height; j++)
	{
		for (int i = 0; i < src_width; i++)
		{
			int src_ptr = i * src_byte_count;
			if (src_bitmap.pitch > 0)
				src_ptr += j * src_pitch;
			else
				src_ptr += (src_height - j - 1) * src_pitch;
			
			int dest_ptr = (dest_x + i) * dest_byte_count;
			if (is_dest_up)
				dest_ptr += (dest_height - dest_y - j - 1) * dest_pitch;
			else
				dest_ptr += (dest_y + j) * dest_pitch;

			const BYTE bmp_r = src_bitmap.buffer[src_ptr];
			const BYTE bmp_g = src_bitmap.buffer[src_ptr+1];
			const BYTE bmp_b = src_bitmap.buffer[src_ptr+2];
			
			dest_bits[dest_ptr] = (bmp_b * _fg_rgb.rgbBlue + (255 - bmp_b) * dest_bits[dest_ptr]) / 255;
			dest_bits[dest_ptr+1] = (bmp_g * _fg_rgb.rgbGreen + (255 - bmp_g) * dest_bits[dest_ptr+1]) / 255;
			dest_bits[dest_ptr+2] = (bmp_r * _fg_rgb.rgbRed + (255 - bmp_r) * dest_bits[dest_ptr+2]) / 255;
			dest_bits[dest_ptr+3] = 0;
		}
	}
}

void _gdimm_text::get_dc_bmi(BITMAPINFO *&bmi) const
{
	int lines_ret;
	BOOL b_ret;

	const HDC hdc_canvas = CreateCompatibleDC(_hdc_text);
	assert(hdc_canvas != NULL);

	const HBITMAP dest_bitmap = CreateCompatibleBitmap(_hdc_text, 1, 1);
	assert(dest_bitmap != NULL);
	
	SelectObject(hdc_canvas, dest_bitmap);
	b_ret = BitBlt(hdc_canvas, 0, 0, 1, 1, _hdc_text, _cursor.x, _cursor.y, SRCCOPY);
	assert(b_ret);
	
	// fill bmi with device bitmap properties
	bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lines_ret = GetDIBits(hdc_canvas, dest_bitmap, 0, 1, NULL, bmi, DIB_RGB_COLORS);
	assert(lines_ret != 0);

	bool need_color_table = true;
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
	else
		need_color_table = false;

	if (need_color_table)
	{
		// fill bmi's color table
		BYTE useless[4];
		lines_ret = GetDIBits(hdc_canvas, dest_bitmap, 0, 1, useless, bmi, DIB_RGB_COLORS);
		assert(lines_ret == 1);
	}

	DeleteObject(dest_bitmap);
	DeleteDC(hdc_canvas);
}

void _gdimm_text::draw_glyph(const vector<FT_BitmapGlyph> &glyphs,
	const vector<POINT> &positions,
	const POINT &src_origin,
	const bitmap_glyph_metrics &metrics,
	CONST RECT *clip_rect,
	BITMAPINFO *bmi) const
{
	/*
	the way we draw bitmap to DC is
	1. create compatible DC and bitmap of the physical DC
	2. capture the bitmap of the physical DC by calling BitBlt
	3. get the physical DC bitmap bits by calling GetDIBits
	4. draw the glyph bitmap over the physical DC bitmap
	5. draw the updated physical DC bitmap back to the physical DC
	*/

	BOOL b_ret;

	POINT dest_origin = src_origin;
	switch ((TA_LEFT | TA_RIGHT | TA_CENTER) & _text_alignment)
	{
	case TA_LEFT:
		break;
	case TA_RIGHT:
		dest_origin.x -= metrics.width;
		break;
	case TA_CENTER:
		dest_origin.x -= metrics.width / 2;
		break;
	}
	switch ((TA_TOP | TA_BOTTOM | TA_BASELINE) & _text_alignment)
	{
	case TA_TOP:
		break;
	case TA_BOTTOM:
		dest_origin.y -= metrics.height;
		break;
	case TA_BASELINE:
		dest_origin.y -= metrics.ascent;
		break;
	}

	const LONG dest_x = dest_origin.x + metrics.bearing_x;
	const LONG dest_y = dest_origin.y;

	bmi->bmiHeader.biWidth = metrics.width;
	bmi->bmiHeader.biHeight = metrics.height;
	bmi->bmiHeader.biSizeImage = 0;
	if (bmi->bmiHeader.biBitCount == 1)
		int a = 0;
	//bmi->bmiHeader.biSizeImage = FT_PAD_CEIL(max(metrics.width * bmi->bmiHeader.biBitCount / 8, 1), sizeof(DWORD)) * metrics.height;
	
	BYTE *dest_bits;
	const HBITMAP dest_bitmap = CreateDIBSection(_hdc_text, bmi, DIB_RGB_COLORS, (VOID**) &dest_bits, NULL, 0);
	assert(dest_bitmap != NULL);

	const HDC hdc_canvas = CreateCompatibleDC(_hdc_text);
	assert(hdc_canvas != NULL);
	SelectObject(hdc_canvas, dest_bitmap);

	if (GetBkMode(_hdc_text) == OPAQUE)
	{
		COLORREF bg_color = GetBkColor(_hdc_text);
        assert(bg_color != CLR_INVALID);

		RECT bg_rect = {0, 0 , metrics.width, metrics.height};
		//ExtTextOut(hdc_canvas, 0, 0, ETO_OPAQUE, &bg_rect, NULL, 0, NULL);
		draw_background(hdc_canvas, &bg_rect, bg_color);
	}
	else
	{
		b_ret = BitBlt(hdc_canvas, 0, 0, metrics.width, metrics.height, _hdc_text, dest_x, dest_y, SRCCOPY);
		assert(b_ret);
	}

	for (unsigned int i = 0; i < glyphs.size(); i++)
	{
		/*
		Windows DIB and FreeType Bitmap have different ways to indicate bitmap direction
		biHeight > 0 means the Windows DIB is bottom-up
		biHeight < 0 means the Windows DIB is top-down
		pitch > 0 means the FreeType bitmap is down flow
		pitch > 0 means the FreeType bitmap is up flow
		*/

		const int x = (glyphs[i]->left - metrics.bearing_x) + (positions[i].x - src_origin.x);
		const int y = (metrics.ascent - glyphs[i]->top) + (positions[i].y - src_origin.y);

		switch (bmi->bmiHeader.biBitCount)
		{
		case 1:
			set_bmp_bits_mono(glyphs[i]->bitmap, dest_bits, x, y, metrics.width, metrics.height, bmi->bmiHeader.biHeight > 0, *((WORD*) &_fg_rgb) == *((WORD*) bmi->bmiColors));
			break;
		case 8:
			set_bmp_bits_gray(glyphs[i]->bitmap, dest_bits, x, y, metrics.width, metrics.height, bmi->bmiHeader.biHeight > 0);
			break;
		case 24:
			set_bmp_bits_lcd(glyphs[i]->bitmap, dest_bits, x, y, metrics.width, metrics.height, bmi->bmiHeader.biHeight > 0);
			break;
		case 32:
			set_bmp_bits_alpha(glyphs[i]->bitmap, dest_bits, x, y, metrics.width, metrics.height, bmi->bmiHeader.biHeight > 0);
			break;
		}

		if (bmi->bmiHeader.biBitCount == 1)
			break;
	}

	if (_eto_options & ETO_CLIPPED)
	{
		RECT dest_rect = {dest_x, dest_y, dest_x + metrics.width, dest_y + metrics.height};
		IntersectRect(&dest_rect, &dest_rect, clip_rect);
		const LONG dest_x_off = dest_rect.left - dest_x;
		const LONG dest_y_off = dest_rect.top - dest_y;
		const LONG dest_width = dest_rect.right - dest_rect.left;
		const LONG dest_height = dest_rect.bottom - dest_rect.top;

		b_ret = BitBlt(_hdc_text,
			dest_rect.left,
			dest_rect.top,
			dest_width,
			dest_height,
			hdc_canvas,
			dest_x_off,
			dest_y_off,
			SRCCOPY);
		assert(b_ret);
	}
	else
	{
		b_ret = BitBlt(_hdc_text, dest_x, dest_y, metrics.width, metrics.height, hdc_canvas, 0, 0, SRCCOPY);
		assert(b_ret);
	}

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

	_eto_options = options;

	// get foreground color
	_fg_color = GetTextColor(_hdc_text);
	assert(_fg_color != CLR_INVALID);
	_fg_rgb.rgbBlue = GetBValue(_fg_color);
	_fg_rgb.rgbGreen = GetGValue(_fg_color);
	_fg_rgb.rgbRed = GetRValue(_fg_color);
	
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

bool _gdimm_text::text_out(const WCHAR *string, unsigned int count, CONST RECT *lprect, CONST INT *lpDx)
{
	BOOL b_ret;
	FT_Error ft_error;
	
	// is ETO_PDY is set, lpDx contains both x increment and y displacement
	const int dx_factor = ((_eto_options & ETO_PDY) ? 2 : 1);

	// identity matrix
	const MAT2 id_mat = {{0, 1}, {0, 0}, {0, 0}, {0, 1}};
	UINT glyph_format = GGO_NATIVE | GGO_UNHINTED;
	SIZE text_extent;

	if (_eto_options & ETO_GLYPH_INDEX)
	{
		glyph_format |= GGO_GLYPH_INDEX;
		b_ret = GetTextExtentPointI(_hdc_text, (WORD*) string, count, &text_extent);
		assert(b_ret);
	}
	else
	{
		b_ret = GetTextExtentPoint32(_hdc_text, string, count, &text_extent);
		assert(b_ret);
	}

	BITMAPINFO *bmi = (BITMAPINFO*) calloc(1, sizeof(BITMAPINFO));
	get_dc_bmi(bmi);
	const FT_Render_Mode render_mode = get_render_mode(bmi->bmiHeader.biBitCount);

	vector<FT_BitmapGlyph> glyphs;
	vector<POINT> positions;
	POINT bitmap_origin = _cursor;

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

			// parse outline coutours
			DWORD header_off = 0;
			do
			{
				const BYTE *header_ptr = outline_buf + header_off;
				const TTPOLYGONHEADER *header = (TTPOLYGONHEADER*) header_ptr;

				// FreeType uses 26.6 format, while Windows gives logical units
				const FT_Vector start_point = {fix_to_26dot6(header->pfxStart.x), fix_to_26dot6(header->pfxStart.y)};
				points.push_back(start_point);
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
						const FT_Vector curr_point = {fix_to_26dot6(curve->apfx[j].x), fix_to_26dot6(curve->apfx[j].y)};
						points.push_back(curr_point);
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
			ft_error = FT_Glyph_To_Bitmap(&generic_glyph, render_mode, NULL, FALSE);
			assert(ft_error == 0);

			FT_BitmapGlyph bmp_glyph = (FT_BitmapGlyph) generic_glyph;
			glyphs.push_back(bmp_glyph);
			positions.push_back(_cursor);
		}

		// advance cursor
		if (lpDx == NULL)
			_cursor.x += glyph_metrics.gmCellIncX;
		else
			_cursor.x += lpDx[i * dx_factor];

		_cursor.y += glyph_metrics.gmCellIncY;
	}

	bool has_glyph = (!glyphs.empty());
	if (has_glyph)
	{
		const WORD bmp_bit_count = get_bmp_bit_count(bmi->bmiHeader.biBitCount);
		assert(bmp_bit_count != 0);

		bitmap_glyph_metrics metrics = get_glyph_metrics(glyphs, positions, bmp_bit_count);
		metrics.width = max(metrics.width, text_extent.cx);
		metrics.ascent = max(metrics.ascent, _outline_metrics->otmTextMetrics.tmAscent);
		metrics.descent = max(metrics.descent, _outline_metrics->otmTextMetrics.tmDescent);
		metrics.height = metrics.ascent + metrics.descent;

		draw_glyph(glyphs, positions, bitmap_origin, metrics, lprect, bmi);
		
		for (unsigned int i = 0; i < glyphs.size(); i++)
			FT_Done_Glyph((FT_Glyph) glyphs[i]);
		// the FT_OutlineGlyph is manually constructed, no need to destroy it
		
		// if TA_UPDATECP is set, update current position after text out
		if (_update_cursor)
		{
			b_ret = MoveToEx(_hdc_text, _cursor.x, _cursor.y, NULL);
			assert(b_ret);
		}
	}
	
	free(bmi);
	return has_glyph;
}