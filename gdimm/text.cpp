#include "stdafx.h"
#include "text.h"
#include "ft_renderer.h"
#include "ggo_renderer.h"
#include "gdimm.h"

// for given bitmap width and bit count, compute the bitmap pitch
int get_pitch(int width, WORD bpp)
{
	return FT_PAD_CEIL((int) ceil((double)(width * bpp) / 8), sizeof(LONG));
}

int gdimm_text::get_ft_bmp_width(const FT_Bitmap &bitmap)
{
	if (bitmap.pixel_mode == FT_PIXEL_MODE_LCD)
		return bitmap.width / 3;

	return bitmap.width;
}

BITMAPINFO gdimm_text::get_dc_bmp_info(HDC hdc)
{
	int i_ret;

	const HBITMAP dc_bitmap = (HBITMAP) GetCurrentObject(hdc, OBJ_BITMAP);
	assert(dc_bitmap != NULL);

	BITMAPINFO bmi = {0};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	i_ret = GetDIBits(hdc, dc_bitmap, 0, 0, NULL, &bmi, DIB_RGB_COLORS);
	assert(i_ret != 0);
	assert(bmi.bmiHeader.biBitCount != 0);

	return bmi;
}

void gdimm_text::draw_background(HDC hdc, const RECT *bg_rect, COLORREF bg_color)
{
	assert(bg_color != CLR_INVALID);

	BOOL b_ret;
	int i_ret;

	const HBRUSH bg_brush = CreateSolidBrush(bg_color);
	assert(bg_brush != NULL);

	i_ret = FillRect(hdc, bg_rect, bg_brush);
	assert(i_ret != 0);

	b_ret = DeleteObject(bg_brush);
	assert(b_ret);
}

// get various metrics of the DC
bool gdimm_text::get_dc_metrics()
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
	assert(h_font != NULL);
	GetObject(h_font, sizeof(LOGFONTW), &_font_attr);

	return true;
}

// for given DC bitmap bit count, return the corresponding FT_Glyph_To_Bitmap render mode
bool gdimm_text::get_render_mode(const WCHAR *font_name, FT_Render_Mode &render_mode) const
{
	bool render_non_aa = false;
	setting_cache_instance.lookup("render_non_aa", font_name, render_non_aa);

	// non-antialiased font
	// draw with monochrome mode
	if (_font_attr.lfQuality == NONANTIALIASED_QUALITY && !render_non_aa)
	{
		render_mode = FT_RENDER_MODE_MONO;
		return true;
	}

	bool subpixel_render = true;
	setting_cache_instance.lookup("subpixel_render", font_name, subpixel_render);

	switch (_bmp_info.bmiHeader.biBitCount)
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

void gdimm_text::get_gamma_ramps(const WCHAR *font_name, bool is_lcd)
{
	if (is_lcd)
	{
		double gamma_red = 1.0;
		setting_cache_instance.lookup("gamma/red", font_name, gamma_red);
		double gamma_green = 1.0;
		setting_cache_instance.lookup("gamma/green", font_name, gamma_green);
		double gamma_blue = 1.0;
		setting_cache_instance.lookup("gamma/blue", font_name, gamma_blue);

		_gamma_ramps[1] = gamma_instance.get_ramp(gamma_red);
		_gamma_ramps[2] = gamma_instance.get_ramp(gamma_green);
		_gamma_ramps[3] = gamma_instance.get_ramp(gamma_blue);
	}
	else
	{
		double gamma_gray = 1.0;
		setting_cache_instance.lookup("gamma/gray", font_name, gamma_gray);

		_gamma_ramps[0] = gamma_instance.get_ramp(gamma_gray);
	}
}

void gdimm_text::set_bmp_bits_mono(
	const FT_Bitmap &src_bitmap,
	int x_in_src, int y_in_src,
	BYTE *dest_bits,
	int x_in_dest, int y_in_dest,
	int dest_width, int dest_height,
	WORD dest_bpp) const
{
	// the source bitmap is 1-bit, 8 pixels per byte, in most-significant order
	// the destination is an non antialiased bitmap with 8, 24 or 32 bpp
	// the source bitmap is not blended with the destination bitmap

	const LONG src_width = src_bitmap.width;
	const LONG src_height = min(src_bitmap.rows - y_in_src, dest_height);
	const int src_pitch = abs(src_bitmap.pitch);
	const int dest_pitch = get_pitch(dest_width, dest_bpp);
	const bool use_zero_color = (*(DWORD*) &_fg_rgb == 0);

	for (int j = 0; j < src_height; j++)
	{
		for (int i = 0; i < src_width; i++)
		{
			const int src_ptr = (y_in_src + j) * src_pitch + (x_in_src + i) / 8;
			const BYTE src_bit_pos = 7 - (x_in_src + i) % 8;
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

void gdimm_text::set_bmp_bits_gray(
	const FT_Bitmap &src_bitmap,
	int x_in_src, int y_in_src,
	BYTE *dest_bits,
	int x_in_dest, int y_in_dest,
	int dest_width, int dest_height,
	WORD dest_bpp,
	WORD bmp_alpha) const
{
	// the source bitmap is 8-bit with 256 gray levels
	// the destination bitmaps has 8, 24 or 32 bpp
	// each row is aligned to DWORD
	// for LCD destination bitmaps, all color channels have the same value

	assert(dest_bpp >= 8);

	const LONG src_width = src_bitmap.width;
	const LONG src_height = min(src_bitmap.rows - y_in_src, dest_height);
	const int src_pitch = abs(src_bitmap.pitch);
	const WORD dest_byte_per_px = dest_bpp / 8;
	const int dest_pitch = get_pitch(dest_width, dest_bpp);

	for (int j = 0; j < src_height; j++)
	{
		for (int i = 0; i < src_width; i++)
		{
			const int src_ptr = (y_in_src + j) * src_pitch + (x_in_src + i);

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

				dest_bits[dest_ptr] = (bmp_alpha * dest_gray + (255 - bmp_alpha) * dest_bits[dest_ptr]) / 255;
			}
			else
			{
				const BYTE dest_r = (src_gray * _fg_rgb.rgbRed + (255 - src_gray) * dest_bits[dest_ptr+2]) / 255;
				const BYTE dest_g = (src_gray * _fg_rgb.rgbGreen + (255 - src_gray) * dest_bits[dest_ptr+1]) / 255;
				const BYTE dest_b = (src_gray * _fg_rgb.rgbBlue + (255 - src_gray) * dest_bits[dest_ptr]) / 255;

				dest_bits[dest_ptr] = (bmp_alpha * dest_b + (255 - bmp_alpha) * dest_bits[dest_ptr]) / 255;
				dest_bits[dest_ptr+1] = (bmp_alpha * dest_g + (255 - bmp_alpha) * dest_bits[dest_ptr+1]) / 255;
				dest_bits[dest_ptr+2] = (bmp_alpha * dest_r + (255 - bmp_alpha) * dest_bits[dest_ptr+2]) / 255;
			}

			//if (dest_bpp == 32)
			//	dest_bits[dest_ptr+3] = 0;
		}
	}
}

void gdimm_text::set_bmp_bits_lcd(
	const FT_Bitmap &src_bitmap,
	int x_in_src, int y_in_src,
	BYTE *dest_bits,
	int x_in_dest, int y_in_dest,
	int dest_width, int dest_height,
	WORD dest_bpp,
	WORD bmp_alpha,
	bool zero_alpha) const
{
	// the source bitmap is 24-bit, in order of R, G, B channels
	// the destination bitmaps has 24 or 32 bpp, in order of B, G, R, (A) channels
	// each row is aligned to DWORD

	assert(dest_bpp >= 24);

	const WORD src_byte_per_px = 3;
	const LONG src_width = src_bitmap.width / src_byte_per_px;
	const LONG src_height = min(src_bitmap.rows - y_in_src, dest_height);
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
			const int src_ptr = (y_in_src + j) * src_pitch + (x_in_src + i) * src_byte_per_px;

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
			dest_bits[dest_ptr] = (bmp_alpha * dest_b + (255 - bmp_alpha) * dest_bits[dest_ptr]) / 255;
			dest_bits[dest_ptr+1] = (bmp_alpha * dest_g + (255 - bmp_alpha) * dest_bits[dest_ptr+1]) / 255;
			dest_bits[dest_ptr+2] = (bmp_alpha * dest_r + (255 - bmp_alpha) * dest_bits[dest_ptr+2]) / 255;

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
bool gdimm_text::draw_glyphs(
	const vector<FT_BitmapGlyph> &glyphs,
	const vector<POINT> &glyph_pos,
	UINT options,
	CONST RECT *lprect) const
{
	BOOL b_ret;

	// 1.

	// origin of the source glyphs
	const POINT src_origin = glyph_pos[0];

	// actual width of the source bitmap
	const size_t last_glyph = glyphs.size() - 1;
	const int text_width = glyph_pos[last_glyph].x + get_ft_bmp_width(glyphs[last_glyph]->bitmap) - src_origin.x;
	const int bmp_width = abs(max(text_width, _cursor.x - glyph_pos[0].x));

	// respect the height and ascent returned from GDI
	const int cell_height = _outline_metrics->otmTextMetrics.tmHeight;
	const int cell_ascent = _outline_metrics->otmTextMetrics.tmAscent;

	// position where the bitmap will be finally drawn if no clipping is needed
	POINT dest_origin = src_origin;

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
		dest_origin.y -= cell_height;
		break;
	case TA_BASELINE:
		dest_origin.y -= cell_ascent;
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
	bmi.bmiHeader.biHeight = cell_height;
	bmi.bmiHeader.biPlanes = _bmp_info.bmiHeader.biPlanes;
	bmi.bmiHeader.biBitCount = _bmp_info.bmiHeader.biBitCount;
	bmi.bmiHeader.biCompression = BI_RGB;

	BYTE *dest_bits;
	const HBITMAP dest_bitmap = CreateDIBSection(_hdc_text, &bmi, DIB_RGB_COLORS, (VOID**) &dest_bits, NULL, 0);
	assert(dest_bitmap != NULL);

	HDC hdc_canvas = CreateCompatibleDC(_hdc_text);
	assert(hdc_canvas != NULL);
	SelectObject(hdc_canvas, dest_bitmap);

	/*
	both ETO_OPAQUE and OPAQUE background mode need background filled
	for ETO_OPAQUE, direct FillRect to the physical DC
	for OPAQUE background mode, draw the background on canvas DC (it might be clipped eventually)
	*/

	if (options & ETO_OPAQUE)
		draw_background(_hdc_text, lprect, _bg_color);
	
	const int bk_mode = GetBkMode(_hdc_text);
	if (bk_mode == OPAQUE)
	{
		RECT bk_rect = {0, 0, bmp_width, cell_height};
		draw_background(hdc_canvas, &bk_rect, _bg_color);
		b_ret = TRUE;
	}
	else if (bk_mode == TRANSPARENT)
	{
		// "If a rotation or shear transformation is in effect in the source device context, BitBlt returns an error"
		b_ret = BitBlt(hdc_canvas, 0, 0, bmp_width, cell_height, _hdc_text, dest_origin.x, dest_origin.y, SRCCOPY | NOMIRRORBITMAP);
	}
	else
		b_ret = FALSE;

	if (!b_ret)	
	{
		b_ret = DeleteObject(dest_bitmap);
		assert(b_ret);

		b_ret = DeleteDC(hdc_canvas);
		assert(b_ret);

		return false;
	}

	// 3.

	const WCHAR *font_face = metric_face_name(&_metric_buf[0]);
	LONG shadow_off_x = 1;
	setting_cache_instance.lookup("shadow/offset_x", font_face, shadow_off_x);
	LONG shadow_off_y = 1;
	setting_cache_instance.lookup("shadow/offset_y", font_face, shadow_off_y);
	WORD shadow_alpha = 32;
	setting_cache_instance.lookup("shadow/alpha", font_face, shadow_alpha);
	bool zero_alpha = false;
	setting_cache_instance.lookup("zero_alpha", font_face, zero_alpha);

	for (size_t i = 0; i < glyphs.size(); i++)
	{
		/*
		Windows DIB and FreeType Bitmap have different ways to indicate bitmap direction
		biHeight > 0 means the Windows DIB is bottom-up
		biHeight < 0 means the Windows DIB is top-down
		pitch > 0 means the FreeType bitmap is down flow
		pitch > 0 means the FreeType bitmap is up flow
		*/

		const int bmp_x_in_src = 0;
		const int bmp_y_in_src = max(glyphs[i]->top - cell_ascent, 0);
		const int bmp_x_in_dest = glyph_pos[i].x - src_origin.x;
		const int bmp_y_in_dest = max(cell_ascent - glyphs[i]->top, 0);

		const int shadow_x_in_src = max(bmp_x_in_src - shadow_off_x, 0);
		const int shadow_y_in_src = max(bmp_y_in_src - shadow_off_y, 0);
		const int shadow_x_in_dest = max(bmp_x_in_dest + shadow_off_x, 0);
		const int shadow_y_in_dest = max(bmp_y_in_dest + shadow_off_y, 0);

		switch (glyphs[i]->bitmap.pixel_mode)
		{
		case FT_PIXEL_MODE_MONO:
			set_bmp_bits_mono(glyphs[i]->bitmap,
				bmp_x_in_src, bmp_y_in_src,
				dest_bits,
				bmp_x_in_dest, bmp_y_in_dest,
				bmp_width, cell_height,
				_bmp_info.bmiHeader.biBitCount);
			break;
		case FT_PIXEL_MODE_GRAY:
			if (shadow_alpha > 0)
				set_bmp_bits_gray(glyphs[i]->bitmap,
					shadow_x_in_src, shadow_y_in_src,
					dest_bits,
					shadow_x_in_dest, shadow_y_in_dest,
					bmp_width, cell_height,
					_bmp_info.bmiHeader.biBitCount,
					shadow_alpha);
			set_bmp_bits_gray(glyphs[i]->bitmap,
				bmp_x_in_src, bmp_y_in_src,
				dest_bits,
				bmp_x_in_dest, bmp_y_in_dest,
				bmp_width, cell_height,
				_bmp_info.bmiHeader.biBitCount,
				255);
			break;
		case FT_PIXEL_MODE_LCD:
			if (shadow_alpha > 0)
				set_bmp_bits_lcd(glyphs[i]->bitmap,
					shadow_x_in_src, shadow_y_in_src,
					dest_bits,
					shadow_x_in_dest, shadow_y_in_dest,
					bmp_width, cell_height,
					_bmp_info.bmiHeader.biBitCount,
					shadow_alpha,
					zero_alpha);
			set_bmp_bits_lcd(glyphs[i]->bitmap,
				bmp_x_in_src, bmp_y_in_src,
				dest_bits,
				bmp_x_in_dest, bmp_y_in_dest,
				bmp_width, cell_height,
				_bmp_info.bmiHeader.biBitCount,
				255,
				zero_alpha);
			break;
		}
	}

	// 4.

	if (options & ETO_CLIPPED)
	{
		RECT dest_rect = {dest_origin.x, dest_origin.y, dest_origin.x + bmp_width, dest_origin.y + cell_height};
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
		b_ret = BitBlt(_hdc_text, dest_origin.x, dest_origin.y, bmp_width, cell_height, hdc_canvas, 0, 0, SRCCOPY | NOMIRRORBITMAP);
		assert(b_ret);
	}

	b_ret = DeleteObject(dest_bitmap);
	assert(b_ret);

	b_ret = DeleteDC(hdc_canvas);
	assert(b_ret);

	return true;
}

bool gdimm_text::init(HDC hdc)
{
	_hdc_text = hdc;

	if (!get_dc_metrics())
		return false;

	const WCHAR *font_face = metric_face_name(_outline_metrics);
	if (setting_instance.is_font_excluded(font_face))
		return false;

	// ignore rotated DC
	if (_font_attr.lfEscapement % 3600 != 0)
		return false;

	LONG max_height = 72;
	setting_cache_instance.lookup("max_height", font_face, max_height);
	if (max_height != 0 && max_height < _outline_metrics->otmTextMetrics.tmHeight)
		return false;

	_bmp_info = get_dc_bmp_info(_hdc_text);

	return true;
}

bool gdimm_text::text_out(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
	BOOL b_ret;

	const WCHAR *font_face = metric_face_name(_outline_metrics);

	FT_Render_Mode render_mode;
	if (!get_render_mode(font_face, render_mode))
		return false;

	_text_alignment = GetTextAlign(_hdc_text);
	assert(_text_alignment != GDI_ERROR);
	BOOL update_cursor;

	if (((TA_NOUPDATECP | TA_UPDATECP) & _text_alignment) == TA_UPDATECP)
	{
		POINT cp;
		GetCurrentPositionEx(_hdc_text, &cp);
		_cursor.x = cp.x;
		_cursor.y = cp.y;
		update_cursor = true;
	}
	else
	{
		_cursor.x = x;
		_cursor.y = y;
		update_cursor = false;
	}

	bool freetype_loader = true;
	setting_cache_instance.lookup("freetype_loader", metric_face_name(_outline_metrics), freetype_loader);
	gdimm_renderer *renderer;

	if (freetype_loader)
		renderer = new ft_renderer(this);
	else
		renderer = new ggo_renderer(this);

	bool render_success = renderer->render(options, lprect, lpString, c, lpDx, render_mode);
	if (!render_success)
	{
		delete renderer;
		return false;
	}

	bool draw_success = false;
	const vector<FT_BitmapGlyph> &glyphs = renderer->get_glyphs();

	if (!glyphs.empty())
	{
		// get foreground color
		COLORREF fg_color = GetTextColor(_hdc_text);
		assert(fg_color != CLR_INVALID);
		_fg_rgb.rgbBlue = GetBValue(fg_color);
		_fg_rgb.rgbGreen = GetGValue(fg_color);
		_fg_rgb.rgbRed = GetRValue(fg_color);

		// not every DC has background color
		_bg_color = GetBkColor(_hdc_text);

		get_gamma_ramps(font_face, ((render_mode & FT_RENDER_MODE_LCD) != 0));

		draw_success = draw_glyphs(glyphs, renderer->get_glyph_pos(), options, lprect);
	}

	// if TA_UPDATECP is set, update current position after text out
	if (update_cursor && draw_success)
	{
		b_ret = MoveToEx(_hdc_text, _cursor.x, _cursor.y, NULL);
		assert(b_ret);
	}

	delete renderer;
	return draw_success;
}