#include "stdafx.h"
#include "text.h"
#include "global.h"
#include "ft.h"
#include "font_man.h"
#include <cmath>
#include <vector>
using namespace std;

#define REFTORGB(ref) (*((COLORRGB*) &ref))

// return the nearest multiple of n that is bigger than x
// borrowed from FreeType
#define FT_ALIGN_UP(x, n) (((x) + ((n)-1)) & ~((n)-1))

// helper inline functions

inline FT_F26Dot6 fix_to_26dot6(const FIXED &fx)
{
	// convert 16.16 fixed float type to 26.6 format
	return *((FT_F26Dot6*) &fx) >> 10;
}

template <typename T>
inline char sign(T n)
{
	/*
	the sign() function
	if n is positive, return 1
	if n is 0, return 0
	if n is negative, return -1
	*/

	if (n == 0)
		return T(0);
	else
		return (n < 0) ? T(-1) : T(1);
}

void _gdimm_text::load_metrics()
{
	// load outline metrics, which also include the text metrics, of the DC

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
	/*
	glyph clazz is a private field, cannot be constructed through FreeType API
	instead, we load the glyph of the default character from the current font
	use the clazz for all subsequent FT_OutlineGlyph
	*/

	const TCHAR *font_full_name = (TCHAR*)(_metric_buf + (UINT) _outline_metrics->otmpFullName);
	unsigned int font_index = gdimm_font_man::instance().lookup_index(_hdc_text, font_full_name);

	FT_Error ft_error;
	const FTC_FaceID face_id = (FTC_FaceID) font_index;

	FT_Face font_face;
	ft_error = FTC_Manager_LookupFace(ft_cache_man, face_id, &font_face);
	assert(ft_error == 0);

	FTC_ScalerRec cache_scale = {face_id, _text_height, _text_height, 1, 0, 0};
	FT_Size font_size;
	ft_error = FTC_Manager_LookupSize(ft_cache_man, &cache_scale, &font_size);
	assert(ft_error == 0);

	ft_error = FT_Load_Char(font_face, _outline_metrics->otmTextMetrics.tmDefaultChar, FT_LOAD_NO_BITMAP);
	assert(ft_error == 0);

	FT_Glyph stub;
	FT_Get_Glyph(font_face->glyph, &stub);
	_glyph_clazz = stub->clazz;
	FT_Done_Glyph(stub);
}

void _gdimm_text::set_bmp_bits_mono(const FT_Bitmap &bitmap, BYTE *dest_bits) const
{
	// the destination bitmap is 1-bit, 8 pixels per byte, highest bit first
	// each row is aligned to DWORD

	const WORD src_byte_count = 3;
	const LONG bmp_width = bitmap.width / src_byte_count;
	const LONG bmp_height = bitmap.rows;
	const int src_pitch = abs(bitmap.pitch);
	const int dest_pitch = FT_ALIGN_UP(bmp_width / 8, sizeof(DWORD));
	static const BYTE zero_color[3] = {0};

	for (int j = 0; j < bmp_height; j++)
	{
		for (int i = 0; i < bmp_width; i++)
		{
			const int src_ptr = j * src_pitch + i * src_byte_count;
			const int dest_ptr = j * dest_pitch + i / 8;
			const int dest_pos = 7 - i % 8;
			char mask_bit = 1;

			// if all 3 RGB channels are 0, the mask bit is 0
			// otherwise, the mask bit is 1
			if (memcmp(bitmap.buffer + src_ptr, zero_color, 3) == 0)
				mask_bit = 0;

			dest_bits[dest_ptr] |= (mask_bit << dest_pos);
		}
	}
}

void _gdimm_text::set_bmp_bits_gray(const FT_Bitmap &bitmap, BYTE *dest_bits) const
{
	// the destination bitmap is 8-bit, 1 byte per pixel
	// each row is aligned to DWORD

	const WORD src_byte_count = 3;
	const LONG bmp_width = bitmap.width / src_byte_count;
	const LONG bmp_height = bitmap.rows;
	const int src_pitch = abs(bitmap.pitch);
	const WORD dest_byte_count = 1;
	const int dest_pitch = FT_ALIGN_UP(bmp_width * dest_byte_count, sizeof(DWORD));

	for (int j = 0; j < bmp_height; j++)
	{
		for (int i = 0; i < bmp_width; i++)
		{
			const int src_ptr = j * src_pitch + i * src_byte_count;
			const int dest_ptr = j * dest_pitch + i * dest_byte_count;

			BYTE r = bitmap.buffer[src_ptr];
			BYTE g = bitmap.buffer[src_ptr+1];
			BYTE b = bitmap.buffer[src_ptr+2];
			
			// average of all 3 channels blended with the background
			dest_bits[dest_ptr] = ((r + g + b) / 3 + dest_bits[dest_ptr]) / 2;
		}
	}
}

void _gdimm_text::set_bmp_bits_lcd(const FT_Bitmap &bitmap, BYTE *dest_bits) const
{
	// the destination bitmap is 24-bit, 3 bytes per pixel, in order of B, G, R channels
	// each row is aligned to DWORD

	const WORD src_byte_count = 3;
	const LONG bmp_width = bitmap.width / src_byte_count;
	const LONG bmp_height = bitmap.rows;
	const int src_pitch = abs(bitmap.pitch);
	const WORD dest_byte_count = 3;
	const int dest_pitch = FT_ALIGN_UP(bmp_width * dest_byte_count, sizeof(DWORD));

	for (int j = 0; j < bmp_height; j++)
	{
		for (int i = 0; i < bmp_width; i++)
		{
			const int src_ptr = j * src_pitch + i * src_byte_count;
			const int dest_ptr = j * dest_pitch + i * dest_byte_count;

			const BYTE r = bitmap.buffer[src_ptr];
			const BYTE g = bitmap.buffer[src_ptr+1];
			const BYTE b = bitmap.buffer[src_ptr+2];
			
			// blend with background
			dest_bits[dest_ptr] = (b * _fg_rgb.b + (255 - b) * dest_bits[dest_ptr]) / 255;
			dest_bits[dest_ptr+1] = (g * _fg_rgb.g + (255 - g) * dest_bits[dest_ptr+1]) / 255;
			dest_bits[dest_ptr+2] = (r * _fg_rgb.r + (255 - r) * dest_bits[dest_ptr+2]) / 255;
		}
	}
}

void _gdimm_text::set_bmp_bits_alpha(const FT_Bitmap &bitmap, BYTE *dest_bits) const
{
	// the destination bitmap is 32-bit, 4 bytes per pixel, in order of B, G, R, A channels
	// each row is aligned to DWORD

	const WORD src_byte_count = 3;
	const LONG bmp_width = bitmap.width / src_byte_count;
	const LONG bmp_height = bitmap.rows;
	const int src_pitch = abs(bitmap.pitch);
	const WORD dest_byte_count = 4;
	const int dest_pitch = FT_ALIGN_UP(bmp_width * dest_byte_count, sizeof(DWORD));

	for (int j = 0; j < bmp_height; j++)
	{
		for (int i = 0; i < bmp_width; i++)
		{
			const int src_ptr = j * src_pitch + i * src_byte_count;
			const int dest_ptr = j * dest_pitch + i * dest_byte_count;

			const BYTE r = bitmap.buffer[src_ptr];
			const BYTE g = bitmap.buffer[src_ptr+1];
			const BYTE b = bitmap.buffer[src_ptr+2];
			
			dest_bits[dest_ptr] = (b * _fg_rgb.b + (255 - b) * dest_bits[dest_ptr]) / 255;
			dest_bits[dest_ptr+1] = (g * _fg_rgb.g + (255 - g) * dest_bits[dest_ptr+1]) / 255;
			dest_bits[dest_ptr+2] = (r * _fg_rgb.r + (255 - r) * dest_bits[dest_ptr+2]) / 255;
			//dest_bits[dest_ptr+3] = (r + g + b) / 3;
		}
	}
}

void _gdimm_text::draw_glyph(const FT_BitmapGlyph glyph) const
{
	// the source bitmap is 24-bit, 3 bytes per pixel, in order of R, G, B channels
	// each row is aligned to DWORD

	const FT_Bitmap bitmap = glyph->bitmap;
	const WORD src_byte_count = 3;
	const LONG bmp_width = bitmap.width / src_byte_count;
	const LONG bmp_height = bitmap.rows;
	const char src_pitch_sign = sign(bitmap.pitch);
	int lines_ret;
	BOOL b_ret;

	const int x = _cursor.x + glyph->left;
	const int y = _cursor.y + _text_height - glyph->top;

	/*
	the way we draw bitmap to DC is
	1. create compatible DC and bitmap of the physical DC
	2. capture the bitmap of the physical DC by calling BitBlt
	3. get BITMAPINFO of the physical DC bitmap by calling GetDIBits
	4. fix BITMAPINFO is needed
	5. get the physical DC bitmap bits by calling GetDIBits
	6. draw the glyph bitmap over the physical DC bitmap
	7. draw the updated physical DC bitmap back to the physical DC
	*/

	BITMAPINFO *bmi = (BITMAPINFO*) calloc(1, sizeof(BITMAPINFO));
	bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

	const HDC hdc_canvas = CreateCompatibleDC(_hdc_text);
	assert(hdc_canvas != NULL);

	const HBITMAP dest_bitmap = CreateCompatibleBitmap(_hdc_text, bmp_width, bmp_height);
	assert(dest_bitmap != NULL);

	SelectObject(hdc_canvas, dest_bitmap);
	b_ret = BitBlt(hdc_canvas, 0, 0, bmp_width, bmp_height, _hdc_text, x, y, SRCCOPY);
	assert(b_ret);

	lines_ret = GetDIBits(hdc_canvas, dest_bitmap, 0, bmp_height, NULL, bmi, DIB_RGB_COLORS);
	assert(lines_ret != 0);
	assert(bmi->bmiHeader.biSizeImage != 0);

	if (bmi->bmiHeader.biClrUsed > 1)
	{
		// the next GetDIBits will alloc as many RGBQUAD to the color table as biClrUsed specifies
		// where the color table only reserves 1 RGBQUAD
		// need to realloc the BITMAPINFO

		bmi = (BITMAPINFO*) realloc(bmi, sizeof(BITMAPINFOHEADER) + bmi->bmiHeader.biClrUsed * sizeof(RGBQUAD));
		assert(bmi != NULL);
	}
	else if (bmi->bmiHeader.biCompression == BI_BITFIELDS)
	{
		// the next GetDIBits will alloc 3 RGBQUAD to the color table
		// need to realloc the BITMAPINFO

		bmi = (BITMAPINFO*) realloc(bmi, sizeof(BITMAPINFOHEADER) + 3 * sizeof(RGBQUAD));
		assert(bmi != NULL);
	}

	bmi->bmiHeader.biHeight *= -src_pitch_sign;

	BYTE *dest_bits = new BYTE[bmi->bmiHeader.biSizeImage];
	lines_ret = GetDIBits(hdc_canvas, dest_bitmap, 0, bmp_height, dest_bits, bmi, DIB_RGB_COLORS);
	assert(lines_ret == bmp_height);

	switch (bmi->bmiHeader.biBitCount)
	{
	case 1:
		set_bmp_bits_mono(bitmap, dest_bits);
		break;
	case 8:
		set_bmp_bits_gray(bitmap, dest_bits);
		break;
	case 24:
		set_bmp_bits_lcd(bitmap, dest_bits);
		break;
	case 32:
		set_bmp_bits_alpha(bitmap, dest_bits);
		break;
	}

	lines_ret = SetDIBitsToDevice(_hdc_text, x, y, bmp_width, bmp_height, 0, 0, 0, bmp_height, dest_bits, bmi, DIB_RGB_COLORS);
	assert(lines_ret == bmp_height);

	delete[] dest_bits;
	DeleteObject(dest_bitmap);
	DeleteDC(hdc_canvas);
	free(bmi);
}

bool _gdimm_text::init(HDC hdc, int x, int y)
{
	TEXTMETRIC text_metric;
	BOOL b_ret = GetTextMetrics(hdc, &text_metric);
	assert(b_ret);

	// not support non-TrueType font
	if ((text_metric.tmPitchAndFamily & TMPF_TRUETYPE) != TMPF_TRUETYPE)
		return false;

	_hdc_text = hdc;

	/*
	if UPDATECP is set, check if the HDC is seen before
	if yes, use the stored cursor; if not use the specified position
	if UPDATECP is not set, use the specified position, and check if the HDC is seen before
	if yes, remove the stored corresponding cursor
	*/

	const UINT align = GetTextAlign(hdc);
	assert(align != GDI_ERROR);
	map<HDC, FT_Vector>::const_iterator dcc_iter = dc_cursors.find(hdc);
	if ((align & TA_UPDATECP) == TA_UPDATECP)
	{
		if (dcc_iter == dc_cursors.end())
		{
			_cursor.x = x;
			_cursor.y = y;
		}
		else
			_cursor = dc_cursors[hdc];

		_update_cursor = true;
	}
	else
	{
		if (dcc_iter != dc_cursors.end())
			dc_cursors.erase(dcc_iter);

		_cursor.x = x;
		_cursor.y = y;
		_update_cursor = false;
	}

	// get foreground color
	_fg_color = GetTextColor(hdc);
	assert(_fg_color != CLR_INVALID);
	_fg_rgb = REFTORGB(_fg_color);

	// load metrics
	_metric_buf = NULL;
	load_metrics();

	get_glyph_clazz();

	return true;
}

void _gdimm_text::text_out(const WCHAR *str, unsigned int count, CONST RECT *lprect, CONST INT *lpDx, BOOL is_glyph_index, BOOL is_pdy)
{
	// is ETO_PDY is set, lpDx contains both x increment and y displacement
	const int dx_factor = (is_pdy ? 2 : 1);

	// identity matrix
	const MAT2 id_mat = {{0, 1}, {0, 0}, {0, 0}, {0, 1}};
	UINT glyph_format = GGO_NATIVE | GGO_UNHINTED;
	if (is_glyph_index != 0)
		glyph_format |= GGO_GLYPH_INDEX;

	for (unsigned int i = 0; i < count; i++)
	{
		GLYPHMETRICS glyph_metrics;

		// get glyph metrics
		DWORD outline_buf_len = GetGlyphOutline(_hdc_text, str[i], glyph_format, &glyph_metrics, 0, NULL, &id_mat);
		assert(outline_buf_len != GDI_ERROR);

		// some character's glyph outline is empty (e.g. space), skip
		if (outline_buf_len > 0)
		{
			BYTE *outline_buf = new BYTE[outline_buf_len];
			outline_buf_len = GetGlyphOutline(_hdc_text, str[i], glyph_format, &glyph_metrics, outline_buf_len, outline_buf, &id_mat);
			assert(outline_buf_len != GDI_ERROR);

			vector<FT_Vector> points;
			vector<char> tags;
			vector<short> contour_pos;

			// parse outline structures
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
			3. FT_Glyph_To_Bitmap: first construct FT_OutlineGlyph from FT_Outline, then render glyph to get FT_Bitmap
			   when constructing FreeType glyph, the private clazz field must be provided
			   support 24-bit bitmap

			we use method 3
			*/
			
			const FT_OutlineGlyphRec outline_glyph = 
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
			FT_Error ft_error = FT_Glyph_To_Bitmap(&generic_glyph, FT_RENDER_MODE_LCD, NULL, FALSE);
			assert(ft_error == 0);

			FT_BitmapGlyph bmp_glyph = (FT_BitmapGlyph) generic_glyph;
			draw_glyph(bmp_glyph);

			FT_Done_Glyph(generic_glyph);
			// the FT_OutlineGlyph is manually constructed, no need to destroy it
		}

		// advance cursor
		if (lpDx == NULL)
			_cursor.x += glyph_metrics.gmCellIncX;
		else
			_cursor.x += lpDx[i * dx_factor];

		_cursor.y += glyph_metrics.gmCellIncY;
	}

	// if UPDATECP is set, update current position after text out
	if (_update_cursor)
	{
		dc_cursors[_hdc_text] = _cursor;
		BOOL b_ret = MoveToEx(_hdc_text, _cursor.x, _cursor.y, NULL);
		assert(b_ret);
	}
}