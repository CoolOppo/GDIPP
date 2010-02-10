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

// convert 16.16 fixed float type to 26.6 format
inline FT_F26Dot6 to_26dot6(const FIXED &fixed)
{
	return *((FT_F26Dot6*) &fixed) >> 10;
}

inline FT_F26Dot6 to_26dot6(int i)
{
	return i << 6;
}

// convert integer to 16.16 fixed float type
inline FT_Pos to_16dot16(int i)
{
	return i << 16;
}

// convert 16.16 fixed float type to integer
inline int from_16dot16(FT_Pos fixed)
{
	return fixed >> 16;
}

// for given bitmap width and bit count, compute the bitmap pitch
inline int get_pitch(int width, WORD bit_count)
{
	return FT_PAD_CEIL((int) ceil((double)(width * bit_count) / 8), sizeof(LONG));
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

// draw solid rectangle of the specified dimension with the specified color on the HDC
void _gdimm_text::draw_background(HDC hdc, CONST RECT *bg_rect, COLORREF color)
{
    const HBRUSH bg_brush = CreateSolidBrush(color);
    assert(bg_brush != NULL);

	int i_ret = FillRect(hdc, bg_rect, bg_brush);
    assert(i_ret != 0);

    DeleteObject(bg_brush);
}

// for given DC bitmap bit count, return the corresponding FT_Bitmap bit count
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

int _gdimm_text::get_ft_bmp_width(const FT_Bitmap &bitmap)
{
	switch (bitmap.pixel_mode)
	{
	case FT_PIXEL_MODE_LCD:
		return bitmap.width / 3;
	default:
		return bitmap.width;
	}
}

// for given DC bitmap bit count, return the corresponding FT_Glyph_To_Bitmap render mode
FT_Render_Mode _gdimm_text::get_render_mode(WORD dc_bit_count) const
{
	// non-antialiased font
	// draw with monochrome mode
	if (_font_attr.lfQuality == NONANTIALIASED_QUALITY)
		return FT_RENDER_MODE_MONO;

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

// get various metrics of the DC
bool _gdimm_text::get_dc_metrics()
{
	// get outline metrics of the DC, which also include the text metrics
	int metric_size = GetOutlineTextMetrics(_hdc_text, 0, NULL);
	if (metric_size == 0)
		return false;

	if (_metric_buf != NULL)
		delete[] _metric_buf;

	_metric_buf = new BYTE[metric_size];
	_outline_metrics = (OUTLINETEXTMETRIC*) _metric_buf;
	metric_size = GetOutlineTextMetrics(_hdc_text, metric_size, _outline_metrics);
	assert(metric_size != 0);

	// get LOGFONT 
	HFONT h_font = (HFONT) GetCurrentObject(_hdc_text, OBJ_FONT);
	GetObject(h_font, sizeof(LOGFONT), &_font_attr);

	return true;
}

void _gdimm_text::get_text_metrics(const vector<FT_BitmapGlyph> &glyphs,
	const vector<POINT> &positions,
	int &width,
	int &height,
	int &ascent,
	int &extra_height) const
{
	const size_t last_index = glyphs.size() - 1;

	width = positions[last_index].x + get_ft_bmp_width(glyphs[last_index]->bitmap) - positions[0].x;
	height = _outline_metrics->otmTextMetrics.tmHeight;
	ascent = _outline_metrics->otmTextMetrics.tmAscent;

	// the height of the glyphs taller than the text metrics. 0 if the glyphs are shorter
	extra_height = height;

	for (size_t i = 0; i <= last_index; i++)
	{
		height = max(height, glyphs[i]->bitmap.rows);
		ascent = max(ascent, glyphs[i]->top);
	}

	extra_height = max(height - extra_height, 0);
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

// get BITMAPINFO of the DC at cursor
void _gdimm_text::get_bmi(BITMAPINFO *&bmi) const
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

	// if the BITMAPINFO contains color table, we must get it before calling CreateDIBSection
	if (need_color_table)
	{
		// retrieve bmi's color table
		// DWORD is enough to hold one pixel
		DWORD pixel_data;
		lines_ret = GetDIBits(hdc_canvas, dest_bitmap, 0, 1, &pixel_data, bmi, DIB_RGB_COLORS);
		assert(lines_ret == 1);
	}

	DeleteObject(dest_bitmap);
	DeleteDC(hdc_canvas);
}

/*
use GetGlyphOutline to get character outline,
convert the outline to FT_OutlineGlyph,
render the glyph to a FT_BitmapGlyph,
and we have bitmap
*/
FT_BitmapGlyph _gdimm_text::get_glyph(WCHAR ch,
	UINT ggo_format,
	const MAT2 &matrix,
	FT_Render_Mode render_mode,
	GLYPHMETRICS *out_metrics) const
{
	FT_Error ft_error;
	GLYPHMETRICS glyph_metrics;
	FT_BitmapGlyph bmp_glyph = NULL;

	// get glyph metrics
	DWORD outline_buf_len = GetGlyphOutline(_hdc_text, ch, ggo_format, &glyph_metrics, 0, NULL, &matrix);
	assert(outline_buf_len != GDI_ERROR);

	// some character's glyph outline is empty (e.g. space), skip
	if (outline_buf_len > 0)
	{
		BYTE *outline_buf = new BYTE[outline_buf_len];
		outline_buf_len = GetGlyphOutline(_hdc_text, ch, ggo_format, &glyph_metrics, outline_buf_len, outline_buf, &matrix);
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
			const FT_Vector start_point = {to_26dot6(header->pfxStart.x), to_26dot6(header->pfxStart.y)};
			points.push_back(start_point);
			tags.push_back(FT_CURVE_TAG_ON);	// the first point is on the curve

			DWORD curve_off = sizeof(TTPOLYGONHEADER);
			while (curve_off < header->cb)
			{
				const TTPOLYCURVE *curve = (TTPOLYCURVE*)(header_ptr + curve_off);
				char curr_tag = 0;
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
				}

				for (int j = 0; j < curve->cpfx; j++)
				{
					const FT_Vector curr_point = {to_26dot6(curve->apfx[j].x), to_26dot6(curve->apfx[j].y)};
					if (memcmp(&curr_point, &start_point, sizeof(FT_Vector)) != 0)
					{
						points.push_back(curr_point);
						tags.push_back(curr_tag);
					}
				}
				tags[tags.size()-1] = FT_CURVE_TAG_ON;	// the last point is on the curve
				
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

		FT_OutlineGlyphRec outline_glyph = 
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

		bmp_glyph = (FT_BitmapGlyph) generic_glyph;
	}

	if (out_metrics)
		*out_metrics = glyph_metrics;

	return bmp_glyph;
}

void _gdimm_text::set_bmp_bits_mono(const FT_Bitmap &src_bitmap,
	BYTE *dest_bits,
	int dest_x, int dest_y,
	int dest_width, int dest_height,
	bool is_dest_up,
	WORD dest_bit_count) const
{
	// the source bitmap is 1-bit, 8 pixels per byte, in most-significant order
	// the destination is an non antialiased bitmap, in ANY bit count
	// the source bitmap is not blended with the destination bitmap

	const LONG src_width = src_bitmap.width;
	const LONG src_height = src_bitmap.rows;
	const int src_pitch = abs(src_bitmap.pitch);
	const int dest_pitch = get_pitch(dest_width, dest_bit_count);
	const bool use_zero_color = (*(DWORD*) &_fg_rgb == 0);

	for (int j = 0; j < src_height; j++)
	{
		for (int i = 0; i < src_width; i++)
		{
			const int src_ptr = j * src_pitch + i / 8;
			const BYTE src_bit_pos = 7 - i % 8;
			const bool is_bit_set = ((src_bitmap.buffer[src_ptr] & (1 << src_bit_pos)) != 0);

			if (is_bit_set)
			{
				int dest_ptr = (dest_x + i) * dest_bit_count / 8;
				if (is_dest_up == (src_bitmap.pitch > 0))
					dest_ptr += max(dest_height - dest_y - j - 1, 0) * dest_pitch;
				else
					dest_ptr += min(dest_y + j, dest_height) * dest_pitch;

				if (dest_bit_count == 1)
				{
					// source bit position might differ from destination bit position
					const BYTE dest_bit_pos = 7 - (dest_x + i) % 8;

					if (use_zero_color)
						dest_bits[dest_ptr] &= ~(1 << dest_bit_pos);
					else
						dest_bits[dest_ptr] |= (1 << dest_bit_pos);
				}
				else
					memcpy(dest_bits + dest_ptr, &_fg_rgb, dest_bit_count / 8);
			}
		}
	}
}

void _gdimm_text::set_bmp_bits_gray(const FT_Bitmap &src_bitmap,
	BYTE *dest_bits,
	int dest_x, int dest_y,
	int dest_width, int dest_height,
	bool is_dest_up) const
{
	// both bitmaps are 8-bit, 1 byte per pixel
	// the source bitmap is blended with the existing destination bitmap

	const LONG src_width = src_bitmap.width;
	const LONG src_height = src_bitmap.rows;
	const int src_pitch = abs(src_bitmap.pitch);
	const int dest_pitch = get_pitch(dest_width, 8);

	for (int j = 0; j < src_height; j++)
	{
		for (int i = 0; i < src_width; i++)
		{
			const int src_ptr = j * src_pitch + i;
			int dest_ptr = dest_x + i;
			if (is_dest_up == (src_bitmap.pitch > 0))
				dest_ptr += max(dest_height - dest_y - j - 1, 0) * dest_pitch;
			else
				dest_ptr += min(dest_y + j, dest_height - 1) * dest_pitch;

			const BYTE gray_level = src_bitmap.buffer[src_ptr];
			dest_bits[dest_ptr] = (gray_level * gray_level + (255 - gray_level) * dest_bits[dest_ptr]) / 255;
		}
	}
}

void _gdimm_text::set_bmp_bits_lcd(const FT_Bitmap &src_bitmap,
	BYTE *dest_bits,
	int dest_x, int dest_y,
	int dest_width, int dest_height,
	bool is_dest_up,
	WORD dest_bit_count) const
{
	// the source bitmap is 24-bit, 3 bytes per pixel, in order of R, G, B channels
	// the destination bitmaps is 24- or 32-bit, 3(4) bytes per pixel, in order of B, G, R, (A) channels
	// each row is aligned to DWORD

	assert(dest_bit_count >= 8);

	const WORD src_byte_count = 3;
	const LONG src_width = src_bitmap.width / src_byte_count;
	const LONG src_height = src_bitmap.rows;
	const int src_pitch = abs(src_bitmap.pitch);
	const WORD dest_byte_count = dest_bit_count / 8;
	const int dest_pitch = get_pitch(dest_width, dest_bit_count);

	// rows about to be copied
	for (int j = 0; j < src_height; j++)
	{
		// width about to be copied
		for (int i = 0; i < src_width; i++)
		{
			// source byte, always treat as down flow
			const int src_ptr = j * src_pitch + i * src_byte_count;

			// destination byte, compute according to two flow directions
			int dest_ptr = (dest_x + i) * dest_byte_count;
			if (is_dest_up == (src_bitmap.pitch > 0))
				dest_ptr += max(dest_height - dest_y - j - 1, 0) * dest_pitch;
			else
				dest_ptr += min(dest_y + j, dest_height - 1) * dest_pitch;

			const BYTE bmp_r = src_bitmap.buffer[src_ptr];
			const BYTE bmp_g = src_bitmap.buffer[src_ptr+1];
			const BYTE bmp_b = src_bitmap.buffer[src_ptr+2];
			
			dest_bits[dest_ptr] = (bmp_b * _fg_rgb.rgbBlue + (255 - bmp_b) * dest_bits[dest_ptr]) / 255;
			dest_bits[dest_ptr+1] = (bmp_g * _fg_rgb.rgbGreen + (255 - bmp_g) * dest_bits[dest_ptr+1]) / 255;
			dest_bits[dest_ptr+2] = (bmp_r * _fg_rgb.rgbRed + (255 - bmp_r) * dest_bits[dest_ptr+2]) / 255;
			memset(dest_bits + dest_ptr + 3, 0, dest_byte_count - src_byte_count);
		}
	}
}

void _gdimm_text::draw_glyph(const vector<FT_BitmapGlyph> &glyphs,
	const vector<POINT> &positions,
	CONST RECT *clip_rect,
	BITMAPINFO *bmi) const
{
	/*
	1. set up metrics
	1. create canvas DC and bitmap, capture the bitmap of the physical DC by calling BitBlt
	3. draw the glyph bitmap over the canvas bitmap
	4. copy the canvas bitmap back to DC, applying clipping optionally
	*/

	BOOL b_ret;

	// 1.

	int text_width;
	int text_height;
	int text_ascent;
	int text_extra_height;
	get_text_metrics(glyphs, positions, text_width, text_height, text_ascent, text_extra_height);

	const POINT src_origin = positions[0];
	POINT dest_origin = src_origin;
	dest_origin.y -= text_extra_height;
	switch ((TA_LEFT | TA_RIGHT | TA_CENTER) & _text_alignment)
	{
	case TA_LEFT:
		break;
	case TA_RIGHT:
		dest_origin.x -= text_width;
		break;
	case TA_CENTER:
		dest_origin.x -= text_width / 2;
		break;
	}
	switch ((TA_TOP | TA_BOTTOM | TA_BASELINE) & _text_alignment)
	{
	case TA_TOP:
		break;
	case TA_BOTTOM:
		dest_origin.y -= text_height;
		break;
	case TA_BASELINE:
		dest_origin.y -= text_ascent;
		break;
	}

	// 2.

	bmi->bmiHeader.biWidth = text_width;
	bmi->bmiHeader.biHeight = text_height;
	//bmi->bmiHeader.biSizeImage = get_pitch(text_width, bmi->bmiHeader.biBitCount) * text_width;
	bmi->bmiHeader.biSizeImage = 0;
	
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

		RECT bg_rect = {0, 0 , text_width, text_height};
		draw_background(hdc_canvas, &bg_rect, bg_color);
	}
	else
	{
		b_ret = BitBlt(hdc_canvas, 0, 0, text_width, text_height, _hdc_text, dest_origin.x, dest_origin.y, SRCCOPY);
		assert(b_ret);
	}

	// 3.

	for (unsigned int i = 0; i < glyphs.size(); i++)
	{
		/*
		Windows DIB and FreeType Bitmap have different ways to indicate bitmap direction
		biHeight > 0 means the Windows DIB is bottom-up
		biHeight < 0 means the Windows DIB is top-down
		pitch > 0 means the FreeType bitmap is down flow
		pitch > 0 means the FreeType bitmap is up flow
		*/

		const int dest_x = positions[i].x - src_origin.x;
		const int dest_y = max(text_ascent - glyphs[i]->top, 0) + (positions[i].y - src_origin.y);

		switch (glyphs[i]->bitmap.pixel_mode)
		{
		case FT_PIXEL_MODE_MONO:
			set_bmp_bits_mono(glyphs[i]->bitmap,
				dest_bits,
				dest_x, dest_y,
				text_width, text_height,
				bmi->bmiHeader.biHeight > 0,
				bmi->bmiHeader.biBitCount);
			break;
		case FT_PIXEL_MODE_GRAY:
			set_bmp_bits_gray(glyphs[i]->bitmap,
				dest_bits,
				dest_x, dest_y,
				text_width, text_height,
				bmi->bmiHeader.biHeight > 0);
			break;
		case FT_PIXEL_MODE_LCD:
			set_bmp_bits_lcd(glyphs[i]->bitmap,
				dest_bits,
				dest_x, dest_y,
				text_width, text_height,
				bmi->bmiHeader.biHeight > 0,
				bmi->bmiHeader.biBitCount);
			break;
		}
	}

	// 4.

	if (_eto_options & ETO_CLIPPED)
	{
		RECT dest_rect = {dest_origin.x, dest_origin.y, dest_origin.x + text_width, dest_origin.y + text_height};
		IntersectRect(&dest_rect, &dest_rect, clip_rect);
		const LONG dest_x = dest_rect.left - dest_origin.x;
		const LONG dest_y = dest_rect.top - dest_origin.y;
		const LONG dest_width = dest_rect.right - dest_rect.left;
		const LONG dest_height = dest_rect.bottom - dest_rect.top;

		b_ret = BitBlt(_hdc_text,
			dest_rect.left,
			dest_rect.top,
			dest_width,
			dest_height,
			hdc_canvas,
			dest_x,
			dest_y,
			SRCCOPY);
		assert(b_ret);
	}
	else
	{
		b_ret = BitBlt(_hdc_text, dest_origin.x, dest_origin.y, text_width, text_height, hdc_canvas, 0, 0, SRCCOPY);
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

	if (!get_dc_metrics())
		return false;
	
	// since we only deal with fonts with outlines, the glyph clazz must be ft_outline_glyph_class
	// therefore this function is called only once
	if (glyph_clazz == NULL)
		get_glyph_clazz();

	_eto_options = options;

	// get foreground color
	COLORREF fg_color = GetTextColor(_hdc_text);
	assert(fg_color != CLR_INVALID);
	_fg_rgb.rgbBlue = GetBValue(fg_color);
	_fg_rgb.rgbGreen = GetGValue(fg_color);
	_fg_rgb.rgbRed = GetRValue(fg_color);
	
	_char_extra = GetTextCharacterExtra(_hdc_text);
	assert(_char_extra != 0x8000000);
	
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

	test = false;
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
	get_dc_metrics();

	gdimm_font_link::instance().restore_font(_hdc_text);
	return true;
}*/

/*
as it names
return true it processed the text 
return false if it cannot deal with the text (let original ExtTextOutW handles)x
*/
bool _gdimm_text::text_out(const WCHAR *string, unsigned int count, CONST RECT *lprect, CONST INT *lpDx)
{
	BOOL b_ret;

	// for SetTextJustification
	if (string[0] == _outline_metrics->otmTextMetrics.tmBreakChar && count == 1)
		return false;
	
	// is ETO_PDY is set, lpDx contains both x increment and y displacement
	const int dx_factor = ((_eto_options & ETO_PDY) ? 2 : 1);

	// identity matrix
	const MAT2 id_matrix = {{0, 1}, {0, 0}, {0, 0}, {0, 1}};
	UINT ggo_format = GGO_NATIVE | GGO_UNHINTED;
	if (_eto_options & ETO_GLYPH_INDEX)
		ggo_format |= GGO_GLYPH_INDEX;

	// get render mode
	BITMAPINFO *bmi = (BITMAPINFO*) calloc(1, sizeof(BITMAPINFO));
	get_bmi(bmi);
	const FT_Render_Mode render_mode = get_render_mode(bmi->bmiHeader.biBitCount);

	// Windows renders monochrome better than FreeType
	if (render_mode == FT_RENDER_MODE_MONO)
	{
		free(bmi);
		return false;
	}

	vector<FT_BitmapGlyph> glyphs;
	vector<POINT> positions;

	for (unsigned int i = 0; i < count; i++)
	{
		// we do not care about non-printable characters
		if (!(_eto_options & ETO_GLYPH_INDEX) && !iswprint(string[i]))
			continue;

		GLYPHMETRICS glyph_metrics;
		FT_BitmapGlyph bmp_glyph = get_glyph(string[i], ggo_format, id_matrix, render_mode, &glyph_metrics);

		if (_char_extra > 0)
			debug_output_process_name();

		// glyph is NULL if the glyph outline is empty (e.g. space character, non-printing characters)
		if (bmp_glyph != NULL)
		{
			glyphs.push_back(bmp_glyph);
			POINT glyph_pos = {_cursor.x + bmp_glyph->left, _cursor.y};
			positions.push_back(glyph_pos);
		}

		// advance cursor
		if (lpDx == NULL)
			_cursor.x += glyph_metrics.gmCellIncX + _char_extra;
		else
			_cursor.x += lpDx[i * dx_factor];
	}
	
	// if TA_UPDATECP is set, update current position after text out
	if (_update_cursor)
	{
		b_ret = MoveToEx(_hdc_text, _cursor.x, _cursor.y, NULL);
		assert(b_ret);
	}

	const bool has_glyph = !glyphs.empty();
	if (has_glyph)
	{
		draw_glyph(glyphs, positions, lprect, bmi);
		
		for (unsigned int i = 0; i < glyphs.size(); i++)
			FT_Done_Glyph((FT_Glyph) glyphs[i]);
		// the FT_OutlineGlyph is manually constructed, no need to destroy it
	}
	
	free(bmi);
	return has_glyph;
}