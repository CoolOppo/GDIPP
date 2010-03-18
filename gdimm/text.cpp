#include "stdafx.h"
#include "text.h"
#include "font_link.h"
#include "font_man.h"
#include "ft.h"
#include "gamma.h"
#include "setting_cache.h"

const double pi = acos(-1.0);
const FT_Glyph_Class *glyph_clazz = NULL;

// convert 16.16 fixed point to 26.6 format
FT_F26Dot6 to_26dot6(const FIXED &fixed)
{
	return *((FT_F26Dot6*) &fixed) >> 10;
}

// convert floating point to 26.6 format
FT_F26Dot6 to_26dot6(double x)
{
	return (FT_F26Dot6)(x * 64);
}

// convert floating point to 16.16 format
FT_Pos to_16dot16(double x)
{
	return (FT_Pos)(x * 65536);
}

// convert 16.16 fixed float type to integer
LONG from_16dot16(FT_Pos fixed)
{
	return fixed >> 16;
}

// for given bitmap width and bit count, compute the bitmap pitch
int get_pitch(int width, WORD bpp)
{
	return FT_PAD_CEIL((int) ceil((double)(width * bpp) / 8), sizeof(LONG));
}

int _gdimm_text::get_ft_bmp_width(const FT_Bitmap &bitmap)
{
	if (bitmap.pixel_mode == FT_PIXEL_MODE_LCD)
		return bitmap.width / 3;

	return bitmap.width;
}

void _gdimm_text::draw_background(HDC hdc, const RECT *bg_rect, COLORREF bg_color)
{
	assert(bg_color != CLR_INVALID);

	int i_ret;

	const HBRUSH bg_brush = CreateSolidBrush(bg_color);
	assert(bg_brush != NULL);

	i_ret = FillRect(hdc, bg_rect, bg_brush);
	assert(i_ret != 0);

	DeleteObject(bg_brush);
}

void _gdimm_text::oblique_outline(const FT_Outline *outline, double angle)
{
	// the advancement of slant on X-axis direction
	const double slant_adv = tan(pi * angle / 180);

	FT_Matrix oblique_mat = {to_16dot16(1), to_16dot16(slant_adv), 0, to_16dot16(1)};
	FT_Outline_Transform(outline, &oblique_mat);
}

// for given DC bitmap bit count, return the corresponding FT_Glyph_To_Bitmap render mode
bool _gdimm_text::get_render_mode(WORD dc_bpp, const WCHAR *font_name, FT_Render_Mode &render_mode) const
{
	bool render_non_aa = false;
	gdimm_setting_cache::instance().lookup("render_non_aa", font_name, render_non_aa);

	// non-antialiased font
	// draw with monochrome mode
	if (_font_attr.lfQuality == NONANTIALIASED_QUALITY && !render_non_aa)
	{
		render_mode = FT_RENDER_MODE_MONO;
		return true;
	}

	bool subpixel_render = true;
	gdimm_setting_cache::instance().lookup("subpixel_render", font_name, subpixel_render);

	switch (dc_bpp)
	{
	case 1:
		render_mode = FT_RENDER_MODE_MONO;
		break;
	case 8:
		render_mode = FT_RENDER_MODE_NORMAL;
		break;
	case 24:
	case 32:
		render_mode = (subpixel_render ? FT_RENDER_MODE_LCD : FT_RENDER_MODE_NORMAL);
		break;
	default:
		// we do not support 16 bpp currently
		return false;
	}

	return true;
}

FT_UInt32 _gdimm_text::get_load_mode(FT_Render_Mode render_mode, const WCHAR *font_name) const
{
	bool auto_hinting = false;
	gdimm_setting_cache::instance().lookup("auto_hinting", font_name, auto_hinting);
	bool embedded_bitmap = false;
	gdimm_setting_cache::instance().lookup("embedded_bitmap", font_name, embedded_bitmap);
	bool hinting = true;
	gdimm_setting_cache::instance().lookup("hinting", font_name, hinting);
	bool light_mode = true;
	gdimm_setting_cache::instance().lookup("light_mode", font_name, light_mode);

	FT_UInt32 load_flag = FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH |
		(embedded_bitmap ? 0 : FT_LOAD_NO_BITMAP);

	if (hinting)
	{
		load_flag |= (auto_hinting ? FT_LOAD_FORCE_AUTOHINT : FT_LOAD_DEFAULT);

		switch (render_mode)
		{
		case FT_RENDER_MODE_NORMAL:
			load_flag |= (light_mode ? FT_LOAD_TARGET_LIGHT : FT_LOAD_TARGET_NORMAL);
		case FT_RENDER_MODE_MONO:
			load_flag |= FT_LOAD_TARGET_MONO;
		case FT_RENDER_MODE_LCD:
			load_flag |= (light_mode ? FT_LOAD_TARGET_LIGHT : FT_LOAD_TARGET_LCD);
		}
	}
	else
		load_flag |= FT_LOAD_NO_HINTING;

	return load_flag;
}

BITMAPINFO _gdimm_text::get_dc_bmp_info() const
{
	int i_ret;

	const HBITMAP dc_bitmap = (HBITMAP) GetCurrentObject(_hdc_text, OBJ_BITMAP);
	assert(dc_bitmap != NULL);

	BITMAPINFO bmi = {0};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	i_ret = GetDIBits(_hdc_text, dc_bitmap, 0, 0, NULL, &bmi, DIB_RGB_COLORS);
	assert(i_ret != 0);

	return bmi;
}

// get various metrics of the DC
bool _gdimm_text::get_dc_metrics()
{
	// get outline metrics of the DC, which also include the text metrics
	UINT metric_size = GetOutlineTextMetricsW(_hdc_text, 0, NULL);
	if (metric_size == 0)
		return false;

	_metric_buf.resize(metric_size);
	_outline_metrics = (OUTLINETEXTMETRICW*) &_metric_buf[0];
	metric_size = GetOutlineTextMetricsW(_hdc_text, metric_size, _outline_metrics);
	assert(metric_size != 0);

	// get LOGFONT 
	HFONT h_font = (HFONT) GetCurrentObject(_hdc_text, OBJ_FONT);
	GetObject(h_font, sizeof(LOGFONTW), &_font_attr);

	return true;
}

void _gdimm_text::get_glyph_clazz()
{
	/*
	glyph clazz is a private field, cannot be constructed through FreeType API
	instead, we load the glyph of the default character from the current font
	use the clazz for all subsequent FT_OutlineGlyph
	*/

	FT_Error ft_error;

	const HFONT curr_hfont = (HFONT) GetCurrentObject(_hdc_text, OBJ_FONT);
	const long font_id = gdimm_font_man::instance().register_font(curr_hfont, L"");

	const FTC_FaceID ft_face_id = (FTC_FaceID) font_id;

	FT_Face font_face;
	ft_error = FTC_Manager_LookupFace(ft_cache_man, ft_face_id, &font_face);
	assert(ft_error == 0);

	FTC_ScalerRec cache_scale = {ft_face_id, 0, 0, 1, 0, 0};
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

void _gdimm_text::get_gamma_ramps(const WCHAR *font_name, bool is_lcd)
{
	if (is_lcd)
	{
		double gamma_red = 1.0;
		gdimm_setting_cache::instance().lookup("gamma/red", font_name, gamma_red);
		double gamma_green = 1.0;
		gdimm_setting_cache::instance().lookup("gamma/green", font_name, gamma_green);
		double gamma_blue = 1.0;
		gdimm_setting_cache::instance().lookup("gamma/blue", font_name, gamma_blue);

		_gamma_ramps[1] = gdimm_gamma::instance().get_ramp(gamma_red);
		_gamma_ramps[2] = gdimm_gamma::instance().get_ramp(gamma_green);
		_gamma_ramps[3] = gdimm_gamma::instance().get_ramp(gamma_blue);
	}
	else
	{
		double gamma_gray = 1.0;
		gdimm_setting_cache::instance().lookup("gamma/gray", font_name, gamma_gray);

		_gamma_ramps[0] = gdimm_gamma::instance().get_ramp(gamma_gray);
	}
}

FT_BitmapGlyph _gdimm_text::outline_to_bitmap(
	WCHAR ch,
	UINT ggo_format,
	const MAT2 &matrix,
	FT_Render_Mode render_mode,
	double embolden,
	GLYPHMETRICS &glyph_metrics) const
{
	FT_Error ft_error;

	DWORD outline_buf_len = GetGlyphOutline(_hdc_text, ch, ggo_format, &glyph_metrics, 0, NULL, &matrix);
	assert(outline_buf_len != GDI_ERROR);

	// some character's glyph outline is empty (e.g. space), skip
	if (outline_buf_len == 0)
		return NULL;
	
	vector<FT_Vector> curve_points;
	vector<char> curve_tags;
	vector<short> contour_indices;

	BYTE *outline_buf = new BYTE[outline_buf_len];
	outline_buf_len = GetGlyphOutline(_hdc_text, ch, ggo_format, &glyph_metrics, outline_buf_len, outline_buf, &matrix);
	assert(outline_buf_len != GDI_ERROR);

	// parse outline coutours
	DWORD header_off = 0;
	do
	{
		const BYTE *header_ptr = outline_buf + header_off;
		const TTPOLYGONHEADER *header = (TTPOLYGONHEADER*) header_ptr;

		// FreeType uses 26.6 format, while Windows gives logical units
		const FT_Vector start_point = {to_26dot6(header->pfxStart.x), to_26dot6(header->pfxStart.y)};

		DWORD curve_off = sizeof(TTPOLYGONHEADER);
		while (curve_off < header->cb)
		{
			// the starting point of each curve is the last point of the previous curve or the starting point of the contour
			if (curve_off == sizeof(TTPOLYGONHEADER))
				curve_points.push_back(start_point);
			else
				curve_points.push_back(curve_points[curve_points.size() - 1]);
			// the first point is on the curve
			curve_tags.push_back(FT_CURVE_TAG_ON);

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
			}

			for (int j = 0; j < curve->cpfx; j++)
			{
				const FT_Vector curr_point = {to_26dot6(curve->apfx[j].x), to_26dot6(curve->apfx[j].y)};
				curve_points.push_back(curr_point);
				curve_tags.push_back(curr_tag);
			}
			// the last point is on the curve
			curve_tags[curve_tags.size() - 1] = FT_CURVE_TAG_ON;

			curve_off += sizeof(TTPOLYCURVE) + (curve->cpfx - 1) * sizeof(POINTFX);
		}

		contour_indices.push_back(curve_points.size() - 1);
		header_off += header->cb;
	} while (header_off < outline_buf_len);

	delete[] outline_buf;

	assert(curve_points.size() <= FT_OUTLINE_POINTS_MAX);

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
			contour_indices.size(),
			curve_points.size(),
			&curve_points[0],
			&curve_tags[0],
			&contour_indices[0],
			FT_OUTLINE_NONE
		}
	};

	ft_error = FT_Outline_Embolden(&outline_glyph.outline, to_26dot6(embolden));
	assert(ft_error == 0);

	// convert outline to bitmap
	FT_Glyph generic_glyph = (FT_Glyph) &outline_glyph;
	ft_error = FT_Glyph_To_Bitmap(&generic_glyph, render_mode, NULL, FALSE);
	assert(ft_error == 0);

	return (FT_BitmapGlyph) generic_glyph;
}

void _gdimm_text::set_bmp_bits_mono(
	const FT_Bitmap &src_bitmap,
	int x_in_dest, int y_in_dest,
	BYTE *dest_bits,
	int dest_width, int dest_height,
	WORD dest_bpp) const
{
	// the source bitmap is 1-bit, 8 pixels per byte, in most-significant order
	// the destination is an non antialiased bitmap with 8, 24 or 32 bpp
	// the source bitmap is not blended with the destination bitmap

	const LONG src_width = src_bitmap.width;
	const LONG src_height = src_bitmap.rows;
	const int src_pitch = abs(src_bitmap.pitch);
	const int dest_pitch = get_pitch(dest_width, dest_bpp);
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
				int dest_ptr = (x_in_dest + i) * dest_bpp / 8;
				if (src_bitmap.pitch > 0)
					dest_ptr += max(dest_height - y_in_dest - j - 1, 0) * dest_pitch;
				else
					dest_ptr += min(y_in_dest + j, dest_height) * dest_pitch;

				if (dest_bpp == 1)
				{
					// source bit position might differ from destination bit position
					const BYTE dest_bit_pos = 7 - (x_in_dest + i) % 8;

					if (use_zero_color)
						dest_bits[dest_ptr] &= ~(1 << dest_bit_pos);
					else
						dest_bits[dest_ptr] |= (1 << dest_bit_pos);
				}
				else
					memcpy(dest_bits + dest_ptr, &_fg_rgb, dest_bpp / 8);
			}
		}
	}
}

void _gdimm_text::set_bmp_bits_gray(
	const FT_Bitmap &src_bitmap,
	int x_in_dest, int y_in_dest,
	BYTE *dest_bits,
	int dest_width, int dest_height,
	WORD dest_bpp,
	WORD alpha) const
{
	// the source bitmap is 8-bit with 256 gray levels
	// the destination bitmaps has 8, 24 or 32 bpp
	// each row is aligned to DWORD
	// for LCD destination bitmaps, all color channels have the same value

	assert(dest_bpp >= 8);

	const LONG src_width = src_bitmap.width;
	const LONG src_height = src_bitmap.rows;
	const int src_pitch = abs(src_bitmap.pitch);
	const WORD dest_byte_per_px = dest_bpp / 8;
	const int dest_pitch = get_pitch(dest_width, dest_bpp);

	for (int j = 0; j < src_height; j++)
	{
		for (int i = 0; i < src_width; i++)
		{
			const int src_ptr = j * src_pitch + i;

			int dest_ptr = (x_in_dest + i) * dest_byte_per_px;
			if (src_bitmap.pitch > 0)
				dest_ptr += max(dest_height - y_in_dest - j - 1, 0) * dest_pitch;
			else
				dest_ptr += min(y_in_dest + j, dest_height - 1) * dest_pitch;

			const BYTE src_gray = _gamma_ramps[0][src_bitmap.buffer[src_ptr]];

			if (dest_bpp == 8)
			{
				// average gray level of the foreground color
				const BYTE fg_gray = (_fg_rgb.rgbRed + _fg_rgb.rgbGreen + _fg_rgb.rgbBlue) / 765;
				const BYTE dest_gray = (src_gray * fg_gray + (255 - src_gray) * dest_bits[dest_ptr]) / 255;

				dest_bits[dest_ptr] = (alpha * dest_gray + (255 - alpha) * dest_bits[dest_ptr]) / 255;
			}
			else
			{
				const BYTE dest_r = (src_gray * _fg_rgb.rgbRed + (255 - src_gray) * dest_bits[dest_ptr+2]) / 255;
				const BYTE dest_g = (src_gray * _fg_rgb.rgbGreen + (255 - src_gray) * dest_bits[dest_ptr+1]) / 255;
				const BYTE dest_b = (src_gray * _fg_rgb.rgbBlue + (255 - src_gray) * dest_bits[dest_ptr]) / 255;

				dest_bits[dest_ptr] = (alpha * dest_b + (255 - alpha) * dest_bits[dest_ptr]) / 255;
				dest_bits[dest_ptr+1] = (alpha * dest_g + (255 - alpha) * dest_bits[dest_ptr+1]) / 255;
				dest_bits[dest_ptr+2] = (alpha * dest_r + (255 - alpha) * dest_bits[dest_ptr+2]) / 255;
			}

			//if (dest_bpp == 32)
			//	dest_bits[dest_ptr+3] = 0;
		}
	}
}

void _gdimm_text::set_bmp_bits_lcd(
	const FT_Bitmap &src_bitmap,
	int x_in_dest, int y_in_dest,
	BYTE *dest_bits,
	int dest_width, int dest_height,
	WORD dest_bpp,
	WORD alpha,
	bool zero_alpha) const
{
	// the source bitmap is 24-bit, in order of R, G, B channels
	// the destination bitmaps has 24 or 32 bpp, in order of B, G, R, (A) channels
	// each row is aligned to DWORD

	assert(dest_bpp >= 24);

	const WORD src_byte_per_px = 3;
	const LONG src_width = src_bitmap.width / src_byte_per_px;
	const LONG src_height = src_bitmap.rows;
	const int src_pitch = abs(src_bitmap.pitch);
	const WORD dest_byte_per_px = dest_bpp / 8;
	const int dest_pitch = get_pitch(dest_width, dest_bpp);

	// rows about to be copied
	for (int j = 0; j < src_height; j++)
	{
		// width about to be copied
		for (int i = 0; i < src_width; i++)
		{
			// source byte, always treat as down flow
			const int src_ptr = j * src_pitch + i * src_byte_per_px;

			// destination byte, compute according to two flow directions
			int dest_ptr = (x_in_dest + i) * dest_byte_per_px;
			if (src_bitmap.pitch > 0)
				dest_ptr += max(dest_height - y_in_dest - j - 1, 0) * dest_pitch;
			else
				dest_ptr += min(y_in_dest + j, dest_height - 1) * dest_pitch;

			const BYTE src_r = _gamma_ramps[1][src_bitmap.buffer[src_ptr]];
			const BYTE src_g = _gamma_ramps[2][src_bitmap.buffer[src_ptr+1]];
			const BYTE src_b = _gamma_ramps[3][src_bitmap.buffer[src_ptr+2]];
			
			const BYTE dest_r = (src_r * _fg_rgb.rgbRed + (255 - src_r) * dest_bits[dest_ptr+2]) / 255;
			const BYTE dest_g = (src_g * _fg_rgb.rgbGreen + (255 - src_g) * dest_bits[dest_ptr+1]) / 255;
			const BYTE dest_b = (src_b * _fg_rgb.rgbBlue + (255 - src_b) * dest_bits[dest_ptr]) / 255;

			// same algorithm as the AlphaBlend API
			dest_bits[dest_ptr] = (alpha * dest_b + (255 - alpha) * dest_bits[dest_ptr]) / 255;
			dest_bits[dest_ptr+1] = (alpha * dest_g + (255 - alpha) * dest_bits[dest_ptr+1]) / 255;
			dest_bits[dest_ptr+2] = (alpha * dest_r + (255 - alpha) * dest_bits[dest_ptr+2]) / 255;

			if (dest_bpp == 32 && zero_alpha)
				dest_bits[dest_ptr+3] = 0;
		}
	}
}

/*
1. set up metrics
1. create canvas DC and bitmap, capture the bitmap of the physical DC by calling BitBlt
3. draw the glyph bitmap over the canvas bitmap
4. copy the canvas bitmap back to DC, apply clipping if necessary
return true if successfully draw the bitmap, otherwise return false
*/
bool _gdimm_text::draw_glyphs(
	const vector<FT_BitmapGlyph> &glyphs,
	const vector<POINT> &glyph_pos,
	int max_glyph_height,
	CONST RECT *lprect,
	const BITMAPINFO &dc_bmp_info) const
{
	BOOL b_ret;

	// 1.

	// origin of the source glyphs
	const POINT src_origin = glyph_pos[0];

	// height of the text from GDI. the source bitmap's height should be close to it
	const int cell_height = _outline_metrics->otmTextMetrics.tmHeight;

	// actual width of the source bitmap
	const size_t last_glyph = glyphs.size() - 1;
	const int text_width = glyph_pos[last_glyph].x + get_ft_bmp_width(glyphs[last_glyph]->bitmap) - src_origin.x;
	const int bmp_width = max(text_width, _cursor.x - glyph_pos[0].x);

	// height of the bitmap that will be drawn
	const int bmp_height = max(max_glyph_height, cell_height);

	const int bmp_extra_height = max(bmp_height - cell_height, 0);

	// ascent that will be applied when drawing the bitmap
	const int bmp_ascent = _outline_metrics->otmTextMetrics.tmAscent + bmp_extra_height;

	// position where the bitmap will be finally drawn if no clipping is needed
	POINT dest_origin = src_origin;

	// make sure the bottom of the bitmap is within the cell
	dest_origin.y -= bmp_extra_height;

	switch ((TA_LEFT | TA_RIGHT | TA_CENTER) & _text_alignment)
	{
	case TA_LEFT:
		break;
	case TA_RIGHT:
		dest_origin.x -= bmp_width;
		break;
	case TA_CENTER:
		dest_origin.x -= bmp_width / 2;
		break;
	}
	switch ((TA_TOP | TA_BOTTOM | TA_BASELINE) & _text_alignment)
	{
	case TA_TOP:
		break;
	case TA_BOTTOM:
		dest_origin.y -= bmp_height;
		break;
	case TA_BASELINE:
		dest_origin.y -= bmp_ascent;
		break;
	}

	// 2.

	/*
	there is no "official" way to get the direction of the bitmap in the text HDC
	it seems BitBlt would automatically convert the direction of its source bitmap if necessary
	therefore we just use bottom-up, as it is more compatible
	*/

	BITMAPINFO bmi = {0};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = bmp_width;
	bmi.bmiHeader.biHeight = bmp_height;
	bmi.bmiHeader.biPlanes = dc_bmp_info.bmiHeader.biPlanes;
	bmi.bmiHeader.biBitCount = dc_bmp_info.bmiHeader.biBitCount;
	bmi.bmiHeader.biCompression = BI_RGB;

	BYTE *dest_bits;
	const HBITMAP dest_bitmap = CreateDIBSection(_hdc_text, &bmi, DIB_RGB_COLORS, (VOID**) &dest_bits, NULL, 0);
	assert(dest_bitmap != NULL);

	const HDC hdc_canvas = CreateCompatibleDC(_hdc_text);
	assert(hdc_canvas != NULL);
	SelectObject(hdc_canvas, dest_bitmap);

	/*
	both ETO_OPAQUE and OPAQUE background mode need background filled
	for ETO_OPAQUE, direct FillRect to the physical DC
	for OPAQUE background mode, draw the background on canvas DC (it might be clipped eventually)
	*/

	if (_eto_options & ETO_OPAQUE)
		draw_background(_hdc_text, lprect, _bg_color);
	
	const int bk_mode = GetBkMode(_hdc_text);
	if (bk_mode == OPAQUE)
	{
		RECT bk_rect = {0, 0, bmp_width, bmp_height};
		draw_background(hdc_canvas, &bk_rect, _bg_color);
		b_ret = TRUE;
	}
	else if (bk_mode == TRANSPARENT)
	{
		// "If a rotation or shear transformation is in effect in the source device context, BitBlt returns an error"
		b_ret = BitBlt(hdc_canvas, 0, 0, bmp_width, bmp_height, _hdc_text, dest_origin.x, dest_origin.y, SRCCOPY | NOMIRRORBITMAP);
	}
	else
		b_ret = FALSE;

	if (!b_ret)	
	{
		DeleteObject(dest_bitmap);
		DeleteDC(hdc_canvas);
		return false;
	}

	// 3.

	const WCHAR *font_face = metric_face_name(&_metric_buf[0]);
	LONG shadow_off_x = 1;
	gdimm_setting_cache::instance().lookup("shadow/offset_x", font_face, shadow_off_x);
	LONG shadow_off_y = 1;
	gdimm_setting_cache::instance().lookup("shadow/offset_y", font_face, shadow_off_y);
	WORD shadow_alpha = 32;
	gdimm_setting_cache::instance().lookup("shadow/alpha", font_face, shadow_alpha);
	bool zero_alpha = false;
	gdimm_setting_cache::instance().lookup("zero_alpha", font_face, zero_alpha);

	for (size_t i = 0; i < glyphs.size(); i++)
	{
		/*
		Windows DIB and FreeType Bitmap have different ways to indicate bitmap direction
		biHeight > 0 means the Windows DIB is bottom-up
		biHeight < 0 means the Windows DIB is top-down
		pitch > 0 means the FreeType bitmap is down flow
		pitch > 0 means the FreeType bitmap is up flow
		*/

		const int x_in_dest = glyph_pos[i].x - src_origin.x;
		const int y_in_dest = max(bmp_ascent - glyphs[i]->top, 0);

		const int shadow_x = max(x_in_dest + shadow_off_y, 0);
		const int shadow_y = max(y_in_dest + shadow_off_y, 0);

		switch (glyphs[i]->bitmap.pixel_mode)
		{
		case FT_PIXEL_MODE_MONO:
			set_bmp_bits_mono(glyphs[i]->bitmap,
				x_in_dest, y_in_dest,
				dest_bits,
				bmp_width, bmp_height,
				dc_bmp_info.bmiHeader.biBitCount);
			break;
		case FT_PIXEL_MODE_GRAY:
			if (shadow_alpha > 0 && (shadow_x != 0 || shadow_y != 0))
				set_bmp_bits_gray(glyphs[i]->bitmap,
					shadow_x, shadow_y,
					dest_bits,
					bmp_width, bmp_height,
					dc_bmp_info.bmiHeader.biBitCount,
					shadow_alpha);
			set_bmp_bits_gray(glyphs[i]->bitmap,
				x_in_dest, y_in_dest,
				dest_bits,
				bmp_width, bmp_height,
				dc_bmp_info.bmiHeader.biBitCount,
				255);
			break;
		case FT_PIXEL_MODE_LCD:
			if (shadow_alpha > 0 && (shadow_x != 0 || shadow_y != 0))
				set_bmp_bits_lcd(glyphs[i]->bitmap,
					shadow_x, shadow_y,
					dest_bits,
					bmp_width, bmp_height,
					dc_bmp_info.bmiHeader.biBitCount,
					shadow_alpha,
					zero_alpha);
			set_bmp_bits_lcd(glyphs[i]->bitmap,
				x_in_dest, y_in_dest,
				dest_bits,
				bmp_width, bmp_height,
				dc_bmp_info.bmiHeader.biBitCount,
				255,
				zero_alpha);
			break;
		}
	}

	// 4.

	if (_eto_options & ETO_CLIPPED)
	{
		RECT dest_rect = {dest_origin.x, dest_origin.y, dest_origin.x + bmp_width, dest_origin.y + bmp_height};
		IntersectRect(&dest_rect, &dest_rect, lprect);
		const LONG dest_width = dest_rect.right - dest_rect.left;
		const LONG dest_height = dest_rect.bottom - dest_rect.top;
		const LONG dest_x = dest_rect.left - dest_origin.x;
		const LONG dest_y = dest_rect.top - dest_origin.y;

		b_ret = BitBlt(_hdc_text,
			dest_rect.left,
			dest_rect.top,
			dest_width,
			dest_height,
			hdc_canvas,
			dest_x,
			dest_y,
			SRCCOPY | NOMIRRORBITMAP);
		assert(b_ret);
	}
	else
	{
		b_ret = BitBlt(_hdc_text, dest_origin.x, dest_origin.y, bmp_width, bmp_height, hdc_canvas, 0, 0, SRCCOPY | NOMIRRORBITMAP);
		assert(b_ret);
	}

	DeleteObject(dest_bitmap);
	DeleteDC(hdc_canvas);
	return true;
}

/*
as its name
return true it processed the text 
return false if it cannot deal with the text (let original ExtTextOutW handle)
*/
bool _gdimm_text::text_out_ggo(const WCHAR *lpString, UINT c, CONST RECT *lprect, CONST INT *lpDx)
{
	BOOL b_ret;

	const WCHAR *font_face = metric_face_name(_outline_metrics);
	const BITMAPINFO dc_bmp_info = get_dc_bmp_info();

	FT_Render_Mode render_mode;
	if (!get_render_mode(dc_bmp_info.bmiHeader.biBitCount, font_face, render_mode))
		return false;

	// Windows renders monochrome bitmap better than FreeType
	bool render_mono = false;
	gdimm_setting_cache::instance().lookup("render_mono", font_face, render_mono);
	if (render_mode == FT_RENDER_MODE_MONO && !render_mono)
		return false;
	
	// is ETO_PDY is set, lpDx contains both x increment and y displacement
	const int dx_factor = ((_eto_options & ETO_PDY) ? 2 : 1);

	// identity matrix
	const MAT2 id_matrix = {{0, 1}, {0, 0}, {0, 0}, {0, 1}};

	/*
	GetGlyphOutline is capable of returning cubic B¨¦zier curves
	although it generally require less points to define a curve with cubic rather than quadratic B¨¦zier curves,
	TrueType fonts internally store curves with quadratic B¨¦zier curves
	GetGlyphOutline has to do conversion, which takes time, and generates more points
	therefore, quadratic B¨¦zier curves are more favored
	*/
	UINT ggo_format = GGO_NATIVE;
	if (_eto_options & ETO_GLYPH_INDEX)
		ggo_format |= GGO_GLYPH_INDEX;

	bool hinting = true;
	gdimm_setting_cache::instance().lookup("hinting", font_face, hinting);
	if (hinting)
		ggo_format |= GGO_UNHINTED;

	double embolden = 0.0;
	gdimm_setting_cache::instance().lookup("embolden", font_face, embolden);

	vector<FT_BitmapGlyph> glyphs;
	vector<POINT> glyph_pos;
	int max_glyph_height = 0;

	for (unsigned int i = 0; i < c; i++)
	{
		// we do not care about non-printable characters
		// solution for Windows Vista/7 Date
		if (!(_eto_options & ETO_GLYPH_INDEX) && !iswprint(lpString[i]))
			continue;

		GLYPHMETRICS glyph_metrics;
		FT_BitmapGlyph bmp_glyph = outline_to_bitmap(
			lpString[i],
			ggo_format,
			id_matrix,
			render_mode,
			embolden,
			glyph_metrics);

		if (bmp_glyph != NULL)
		{
			POINT adjusted_pos = _cursor;
			adjusted_pos.x += bmp_glyph->left;

			glyphs.push_back(bmp_glyph);
			glyph_pos.push_back(adjusted_pos);
			max_glyph_height = max(max_glyph_height, bmp_glyph->bitmap.rows);
		}

		POINT glyph_advance = {0, glyph_metrics.gmCellIncY};
		const LONG char_advance = glyph_metrics.gmCellIncX + _char_extra;

		if (lpDx == NULL)
			glyph_advance.x = char_advance;
		else
		{
			glyph_advance.x = lpDx[i * dx_factor];

			// the last element in lpDx may be 0
			// in that case, we use the character advancement
			if (i == c - 1 && glyph_advance.x == 0)
				glyph_advance.x = char_advance;
		}

		_cursor.x += glyph_advance.x;
		_cursor.y += glyph_advance.y;
	}
	
	// if TA_UPDATECP is set, update current position after text out
	if (_update_cursor)
	{
		b_ret = MoveToEx(_hdc_text, _cursor.x, _cursor.y, NULL);
		assert(b_ret);
	}

	bool draw_success = false;
	if (!glyphs.empty())
	{
		get_gamma_ramps(metric_face_name(_outline_metrics), ((render_mode & FT_RENDER_MODE_LCD) != 0));

		draw_success = draw_glyphs(glyphs, glyph_pos, max_glyph_height, lprect, dc_bmp_info);
		
		for (size_t i = 0; i < glyphs.size(); i++)
		{
			FT_Done_Glyph((FT_Glyph) glyphs[i]);
			// the FT_OutlineGlyph is manually constructed, no need to destroy it
		}
	}

	return draw_success;
}

/*
as its name
return true it processed the text 
return false if it cannot deal with the text (let original ExtTextOutW handle)
*/
bool _gdimm_text::text_out_ft(LPCWSTR lpString, UINT c, CONST RECT *lprect, CONST INT *lpDx)
{
	BOOL b_ret;
	FT_Error ft_error;

	const WCHAR *font_face = metric_face_name(_outline_metrics);
	const BITMAPINFO dc_bmp_info = get_dc_bmp_info();

	// the bitmap of the DC has been unavailable
	if (dc_bmp_info.bmiHeader.biBitCount == 0)
		return false;

	FT_Render_Mode render_mode;
	if (!get_render_mode(dc_bmp_info.bmiHeader.biBitCount, font_face, render_mode))
		return false;

	// Windows renders monochrome bitmap better than FreeType
	bool render_mono = false;
	gdimm_setting_cache::instance().lookup("render_mono", font_face, render_mono);
	if (render_mode == FT_RENDER_MODE_MONO && !render_mono)
		return false;

	// is ETO_PDY is set, lpDx contains both x increment and y displacement
	const int dx_factor = ((_eto_options & ETO_PDY) ? 2 : 1);

	// font face setup
	const WCHAR *dc_font_face = metric_face_name(_outline_metrics);
	const WCHAR *dc_font_family = metric_family_name(_outline_metrics);
	const WCHAR *final_font_face = dc_font_face;
	const WCHAR *final_font_family = dc_font_family;
	FT_UInt32 load_flags = get_load_mode(render_mode, dc_font_face);

	const HFONT curr_hfont = (HFONT) GetCurrentObject(_hdc_text, OBJ_FONT);
	assert(curr_hfont != NULL);

	long font_id = gdimm_font_man::instance().register_font(curr_hfont, dc_font_face);
	FTC_FaceID ft_face_id = (FTC_FaceID) font_id;

	const size_t link_count = gdimm_font_link::instance().get_link_count(dc_font_family);
	int link_index = 0;

	/*
	glyph metrics setup

	while the height in FreeType scaler has the same meaning as the height value in LOGFONT structure, the widths are different
	what we know is, when the width in LOGFONT is the xAvgCharWidth defined in the TrueType OS/2 table,
	the corresponding FreeType scaler width is the height
	therefore we need conversion when LOGFONT width is not 0
	simple calculation yields new_freetype_width = logfont_width * em_square / xAvgCharWidth
	note that the tmAveCharWidth field in TEXTMETRIC is the actual LOGFONT width, never be 0
	*/

	const LONG glyph_height = _outline_metrics->otmTextMetrics.tmHeight - _outline_metrics->otmTextMetrics.tmInternalLeading;
	LONG glyph_width;

	if (_font_attr.lfWidth == 0)
		glyph_width = glyph_height;
	else
	{
		// get the OS/2 table of the current font
		FT_Face ft_face;
		FTC_Manager_LookupFace(ft_cache_man, ft_face_id, &ft_face);
		TT_OS2 *os2_table = (TT_OS2*) FT_Get_Sfnt_Table(ft_face, ft_sfnt_os2);

		// compare the xAvgCharWidth against the current average char width
		glyph_width = MulDiv(_outline_metrics->otmTextMetrics.tmAveCharWidth, _outline_metrics->otmEMSquare, os2_table->xAvgCharWidth);
	}
	FTC_ScalerRec ft_scaler = {ft_face_id, glyph_width, glyph_height, 1, 0, 0};

	vector<FT_BitmapGlyph> glyphs;
	vector<POINT> glyph_pos;
	int max_glyph_height = 0;
	
	for (unsigned int i = 0; i < c; i++)
	{
		WORD glyph_index;
		bool font_linked = false;

		if (_eto_options & ETO_GLYPH_INDEX)
			glyph_index = lpString[i];
		else
		{
			// we do not care about non-printable characters
			// solution for Windows Vista/7 Date
			if (!iswprint(lpString[i]))
				continue;

			int link_times = 0;
			while (true)
			{
				// character code -> glyph index lookup
				glyph_index = gdimm_font_man::instance().get_glyph_index(lpString[i]);

				// break if either find the glyph index, or exhaust the font link count
				if (glyph_index != 0xffff || link_times == link_count)
					break;

				// do font linking
				final_font_family = gdimm_font_link::instance().lookup(dc_font_family, link_index);
				link_index++;
				link_times++;
				font_linked = true;

				// round-robin
				if (final_font_family == NULL)
				{
					link_index = 0;
					final_font_family = dc_font_family;
				}

				font_id = gdimm_font_man::instance().lookup_font(_font_attr, final_font_family, final_font_face);
				if (font_id < 0)
					return false;

				ft_face_id = (FTC_FaceID) font_id;
			}
		}

		// glyph index -> glyph outline lookup

		// use load mode for the new font family
		ft_scaler.face_id = ft_face_id;

		// if font linked, get the new render mode and load mode for the linked font family
		if (font_linked)
		{
			get_render_mode(dc_bmp_info.bmiHeader.biBitCount, final_font_face, render_mode);
			load_flags = get_load_mode(render_mode, final_font_face);
		}
		
		FT_Glyph glyph, cached_glyph;
		ft_error = FTC_ImageCache_LookupScaler(ft_glyph_cache, &ft_scaler, load_flags, glyph_index, &cached_glyph, NULL);
		if (ft_error != 0)
			return false;

		// some fonts are embedded with pre-rendered glyph bitmap
		// in that case, use original ExtTextOutW
		if (cached_glyph->format != FT_GLYPH_FORMAT_OUTLINE)
			return false;

		FT_Glyph_Copy(cached_glyph, &glyph);
		FT_Outline *glyph_outline = &((FT_OutlineGlyph) glyph)->outline;

		// if italic style is requested, and the font has no native italic glyphs,
		// do oblique transformation
		if (_font_attr.lfItalic && _outline_metrics->otmItalicAngle == 0)
			oblique_outline(glyph_outline, 20);

		// glyph outline -> glyph bitmap conversion
		double embolden = 0.0;
		gdimm_setting_cache::instance().lookup("embolden", final_font_face, embolden);
		if (embolden != 0.0)
		{
			ft_error = FT_Outline_Embolden(glyph_outline, to_26dot6(embolden));
			assert(ft_error == 0);
		}

		FT_Glyph_To_Bitmap(&glyph, render_mode, NULL, TRUE);
		FT_BitmapGlyph bmp_glyph = (FT_BitmapGlyph) glyph;
		max_glyph_height = max(max_glyph_height, bmp_glyph->bitmap.rows);

		if (bmp_glyph != NULL)
		{
			POINT adjusted_pos = _cursor;
			adjusted_pos.x += bmp_glyph->left;

			glyphs.push_back(bmp_glyph);
			glyph_pos.push_back(adjusted_pos);
		}

		POINT glyph_advance = {0, from_16dot16(bmp_glyph->root.advance.y)};
		const LONG char_advance = from_16dot16(bmp_glyph->root.advance.x) + _char_extra;

		if (lpDx == NULL)
			glyph_advance.x = char_advance;
		else
		{
			glyph_advance.x = lpDx[i * dx_factor];

			// the last element in lpDx may be 0
			// in that case, we use the character advancement
			if (i == c - 1 && glyph_advance.x == 0)
				glyph_advance.x = char_advance;
		}

		_cursor.x += glyph_advance.x;
		_cursor.y += glyph_advance.y;
	}
	
	// if TA_UPDATECP is set, update current position after text out
	if (_update_cursor)
	{
		b_ret = MoveToEx(_hdc_text, _cursor.x, _cursor.y, NULL);
		assert(b_ret);
	}

	bool draw_success = false;
	if (!glyphs.empty())
	{
		get_gamma_ramps(final_font_face, ((render_mode & FT_RENDER_MODE_LCD) != 0));

		draw_success = draw_glyphs(glyphs, glyph_pos, max_glyph_height, lprect, dc_bmp_info);

		for (size_t i = 0; i < glyphs.size(); i++)
			FT_Done_Glyph((FT_Glyph) glyphs[i]);
	}
	
	return draw_success;
}

bool _gdimm_text::init(HDC hdc, int x, int y, UINT options)
{
	_hdc_text = hdc;

	if (!get_dc_metrics())
		return false;

	const WCHAR *font_face = metric_face_name(_outline_metrics);
	if (gdipp_setting::instance().is_font_excluded(font_face))
		return false;

	// ignore rotated DC
	if (_font_attr.lfEscapement % 3600 != 0)
		return false;

	LONG max_height = 72;
	gdimm_setting_cache::instance().lookup("max_height", font_face, max_height);
	if (max_height != 0 && max_height < _outline_metrics->otmTextMetrics.tmHeight)
		return false;

	_eto_options = options;

	// get foreground color
	COLORREF fg_color = GetTextColor(_hdc_text);
	assert(fg_color != CLR_INVALID);
	_fg_rgb.rgbBlue = GetBValue(fg_color);
	_fg_rgb.rgbGreen = GetGValue(fg_color);
	_fg_rgb.rgbRed = GetRValue(fg_color);

	// notice that not all DC has background color
	_bg_color = GetBkColor(hdc);

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

	return true;
}

bool _gdimm_text::text_out(LPCWSTR lpString, UINT c, CONST RECT *lprect, CONST INT *lpDx)
{
	bool freetype_loader = true;
	gdimm_setting_cache::instance().lookup("freetype_loader", metric_face_name(_outline_metrics), freetype_loader);

	if (freetype_loader)
		return text_out_ft(lpString, c, lprect, lpDx);
	else
	{
		// since we only deal with fonts with outlines, the glyph clazz must be ft_outline_glyph_class
		// therefore this function is called only once
		if (glyph_clazz == NULL)
			get_glyph_clazz();

		return text_out_ggo(lpString, c, lprect, lpDx);
	}
}