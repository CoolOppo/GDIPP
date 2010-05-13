#include "stdafx.h"
#include "gdi_text.h"
#include "text_helper.h"

gdimm_gdi_text::gdimm_gdi_text(HDC hdc)
:
gdimm_text(hdc)
{
}

void gdimm_gdi_text::set_bmp_bits_mono(const FT_BitmapGlyph glyph,
	BYTE *dest_bits,
	POINT dest_pos,
	int dest_width,
	int dest_ascent,
	int dest_descent) const
{
	// the source bitmap is 1-bit, 8 pixels per byte, in most-significant order
	// the destination is an non antialiased bitmap with 8, 24 or 32 bpp
	// the source bitmap is not blended with the destination bitmap

	const int top_extra_rows = glyph->top - dest_ascent - dest_pos.y;
	const int bottom_end_row = glyph->top + dest_descent - dest_pos.y;

	const int bmp_width = glyph->bitmap.width;
	const int bmp_height = glyph->bitmap.rows;
	const int dest_pitch = get_bmp_pitch(dest_width, _bmp_info.bmiHeader.biBitCount);
	const bool use_zero_color = (_text_rgb.rgbRed == 0 && _text_rgb.rgbGreen == 0 && _text_rgb.rgbBlue == 0);

	// pointers to the beginning of a row
	// initialize at the first available row
	int src_row_ptr = 0;
	int dest_row_ptr = -top_extra_rows * dest_pitch;

	for (int j = 0; j < bmp_height; j++)
	{
		if (j < top_extra_rows)
			continue;

		if (j >= bottom_end_row)
			break;

		for (int i = 0; i < bmp_width; i++)
		{
			if (dest_pos.x + i < 0)
				continue;

			if (dest_pos.x + i >= dest_width)
				break;

			// pointers to the current pixel
			const int src_px_ptr = src_row_ptr + i / 8;
			const BYTE src_bit_pos = 7 - i % 8;
			const bool is_bit_set = ((glyph->bitmap.buffer[src_px_ptr] & (1 << src_bit_pos)) != 0);

			if (is_bit_set)
			{
				const int dest_px_ptr = dest_row_ptr + (dest_pos.x + i) * _bmp_info.bmiHeader.biBitCount / 8;

				if (_bmp_info.bmiHeader.biBitCount == 1)
				{
					// source bit position might differ from destination bit position
					const BYTE dest_bit_pos = 7 - (dest_pos.x + i) % 8;

					if (use_zero_color)
						dest_bits[dest_px_ptr] &= ~(1 << dest_bit_pos);
					else
						dest_bits[dest_px_ptr] |= (1 << dest_bit_pos);
				}
				else
					memcpy(dest_bits + dest_px_ptr, &_text_rgb, _bmp_info.bmiHeader.biBitCount / 8);
			}
		}
	}
}

void gdimm_gdi_text::set_bmp_bits_gray(const FT_BitmapGlyph glyph,
	BYTE *dest_bits,
	POINT dest_pos,
	int dest_width,
	int dest_ascent,
	int dest_descent,
	WORD alpha) const
{
	// the source bitmap is 8-bit with 256 gray levels
	// the destination bitmaps has 8, 24 or 32 bpp
	// each row is aligned to DWORD
	// for LCD destination bitmaps, all color channels have the same value

	assert(_bmp_info.bmiHeader.biBitCount >= 8);

	const int top_extra_rows = glyph->top - dest_ascent - dest_pos.y;
	const int bottom_end_row = glyph->top + dest_descent - dest_pos.y;

	const WORD bmp_byte_per_px = 1;
	const int bmp_width = glyph->bitmap.width / bmp_byte_per_px;
	const int bmp_height = glyph->bitmap.rows;
	const WORD dest_byte_per_px = _bmp_info.bmiHeader.biBitCount / 8;
	const int dest_pitch = get_bmp_pitch(dest_width, _bmp_info.bmiHeader.biBitCount);
	const bool zero_alpha = (_bmp_info.bmiHeader.biBitCount == 32 && _setting_cache->zero_alpha);

	// pointers to the beginning of a row
	// initialize at the first available row
	int src_row_ptr = 0;
	int dest_row_ptr = -top_extra_rows * dest_pitch;

	for (int j = 0; j < bmp_height; j++, src_row_ptr += glyph->bitmap.pitch, dest_row_ptr += dest_pitch)
	{
		if (j < top_extra_rows)
			continue;

		if (j >= bottom_end_row)
			break;

		// pointers to the current pixel
		int src_px_ptr = src_row_ptr;
		int dest_px_ptr = dest_row_ptr + dest_pos.x * dest_byte_per_px;

		for (int i = 0; i < bmp_width; i++, src_px_ptr += bmp_byte_per_px, dest_px_ptr += dest_byte_per_px)
		{
			if (dest_pos.x + i < 0)
				continue;

			if (dest_pos.x + i >= dest_width)
				break;

			const BYTE src_gray = _gamma_ramps[0][glyph->bitmap.buffer[src_px_ptr]];

			if (_bmp_info.bmiHeader.biBitCount == 8)
			{
				// average gray level of the foreground color
				const BYTE fg_gray = (_text_rgb.rgbRed + _text_rgb.rgbGreen + _text_rgb.rgbBlue) / 765;
				const BYTE dest_gray = (src_gray * fg_gray + (255 - src_gray) * dest_bits[dest_px_ptr]) / 255;

				dest_bits[dest_px_ptr] = (alpha * dest_gray + (255 - alpha) * dest_bits[dest_px_ptr]) / 255;
			}
			else
			{
				const BYTE dest_r = (src_gray * _text_rgb.rgbRed + (255 - src_gray) * dest_bits[dest_px_ptr+2]) / 255;
				const BYTE dest_g = (src_gray * _text_rgb.rgbGreen + (255 - src_gray) * dest_bits[dest_px_ptr+1]) / 255;
				const BYTE dest_b = (src_gray * _text_rgb.rgbBlue + (255 - src_gray) * dest_bits[dest_px_ptr]) / 255;

				dest_bits[dest_px_ptr] = (alpha * dest_b + (255 - alpha) * dest_bits[dest_px_ptr]) / 255;
				dest_bits[dest_px_ptr+1] = (alpha * dest_g + (255 - alpha) * dest_bits[dest_px_ptr+1]) / 255;
				dest_bits[dest_px_ptr+2] = (alpha * dest_r + (255 - alpha) * dest_bits[dest_px_ptr+2]) / 255;
			}

			if (zero_alpha)
				dest_bits[dest_px_ptr+3] = 0;
		}
	}
}

void gdimm_gdi_text::set_bmp_bits_lcd(const FT_BitmapGlyph glyph,
	BYTE *dest_bits,
	POINT dest_pos,
	int dest_width,
	int dest_ascent,
	int dest_descent,
	WORD alpha) const
{
	// the source bitmap is 24-bit, in order of R, G, B channels
	// the destination bitmaps has 24 or 32 bpp, in order of B, G, R, (A) channels
	// each row is aligned to DWORD

	assert(_bmp_info.bmiHeader.biBitCount >= 24);

	const int top_extra_rows = glyph->top - dest_ascent - dest_pos.y;
	const int bottom_end_row = glyph->top + dest_descent - dest_pos.y;

	const WORD bmp_byte_per_px = 3;
	const int bmp_width = glyph->bitmap.width / bmp_byte_per_px;
	const int bmp_height = glyph->bitmap.rows;
	const WORD dest_byte_per_px = _bmp_info.bmiHeader.biBitCount / 8;
	const int dest_pitch = get_bmp_pitch(dest_width, _bmp_info.bmiHeader.biBitCount);
	const bool zero_alpha = (_bmp_info.bmiHeader.biBitCount == 32 && _setting_cache->zero_alpha);

	// pointers to the beginning of a row
	// initialize at the first available row
	int src_row_ptr = 0;
	int dest_row_ptr = -top_extra_rows * dest_pitch;

	for (int j = 0; j < bmp_height; j++, src_row_ptr += glyph->bitmap.pitch, dest_row_ptr += dest_pitch)
	{
		if (j < top_extra_rows)
			continue;

		if (j >= bottom_end_row)
			break;

		// pointers to the current pixel
		int src_px_ptr = src_row_ptr;
		int dest_px_ptr = dest_row_ptr + dest_pos.x * dest_byte_per_px;

		for (int i = 0; i < bmp_width; i++, src_px_ptr += bmp_byte_per_px, dest_px_ptr += dest_byte_per_px)
		{
			if (dest_pos.x + i < 0)
				continue;

			if (dest_pos.x + i >= dest_width)
				break;

			const BYTE src_r = _gamma_ramps[1][glyph->bitmap.buffer[src_px_ptr]];
			const BYTE src_g = _gamma_ramps[2][glyph->bitmap.buffer[src_px_ptr+1]];
			const BYTE src_b = _gamma_ramps[3][glyph->bitmap.buffer[src_px_ptr+2]];
			
			const BYTE dest_r = (src_r * _text_rgb.rgbRed + (255 - src_r) * dest_bits[dest_px_ptr+2]) / 255;
			const BYTE dest_g = (src_g * _text_rgb.rgbGreen + (255 - src_g) * dest_bits[dest_px_ptr+1]) / 255;
			const BYTE dest_b = (src_b * _text_rgb.rgbBlue + (255 - src_b) * dest_bits[dest_px_ptr]) / 255;

			// same algorithm as AlphaBlend
			dest_bits[dest_px_ptr] = (alpha * dest_b + (255 - alpha) * dest_bits[dest_px_ptr]) / 255;
			dest_bits[dest_px_ptr+1] = (alpha * dest_g + (255 - alpha) * dest_bits[dest_px_ptr+1]) / 255;
			dest_bits[dest_px_ptr+2] = (alpha * dest_r + (255 - alpha) * dest_bits[dest_px_ptr+2]) / 255;

			if (zero_alpha)
				dest_bits[dest_px_ptr+3] = 0;
		}
	}
}

/*
1. set up metrics, apply clipping if necessary
2. create canvas DC and bitmap, capture the bitmap of the physical DC by calling BitBlt
3. draw the glyph bitmap over the canvas bitmap
4. copy the canvas bitmap back to DC
return true if successfully draw the bitmap, otherwise return false
*/
bool gdimm_gdi_text::draw_glyphs(int x,
	int y,
	UINT options,
	CONST RECT *lprect) const
{
	BOOL b_ret;

	assert(!_glyphs.empty());
	assert(_glyphs.size() == _glyph_pos.size());

	// 1.

	// actual rect occupied by the glyphs
	RECT glyph_run_rect = get_glyph_run_rect(_glyphs, _glyph_pos);

	LONG cell_width = glyph_run_rect.right - glyph_run_rect.left;
	LONG cell_height = _outline_metrics->otmTextMetrics.tmHeight;
	LONG cell_ascent = _outline_metrics->otmTextMetrics.tmAscent;
	LONG cell_descent = _outline_metrics->otmTextMetrics.tmDescent;

	// adjusted baseline where the bitmap will be finally drawn before applying clipping
	const POINT baseline = get_baseline(_text_alignment,
		x,
		y,
		cell_width,
		cell_ascent,
		cell_descent);

	RECT bmp_rect = {baseline.x,
		baseline.y - cell_ascent,
		baseline.x + cell_width,
		baseline.y + cell_descent};
	
	// apply clipping
	if (options & ETO_CLIPPED)
	{
		if (!IntersectRect(&bmp_rect, &bmp_rect, lprect))
			bmp_rect = *lprect;
		
		cell_width = bmp_rect.right - bmp_rect.left;
		cell_height = bmp_rect.bottom - bmp_rect.top;
		cell_ascent = baseline.y - bmp_rect.top;
		cell_descent = bmp_rect.bottom - baseline.y;
	}
	
	// 2.

	/*
	the flow direction of the original bitmap is unrecoverable
	it seems BitBlt would automatically convert the direction of its source bitmap if necessary
	FreeType returns top-down bitmap, so we use top-down to simplify algorithm
	note that the semantic of the direction from GDI is opposite to FreeType
	*/

	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = cell_width;
	bmi.bmiHeader.biHeight = -cell_height;
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

	const COLORREF bg_color = GetBkColor(_hdc_text);

	if (options & ETO_OPAQUE)
		draw_background(_hdc_text, lprect, bg_color);
	
	const int bk_mode = GetBkMode(_hdc_text);
	if (bk_mode == OPAQUE)
	{
		const RECT bk_rect = {0, 0, cell_width, cell_height};
		b_ret = draw_background(hdc_canvas, &bk_rect, bg_color);
	}
	else if (bk_mode == TRANSPARENT)
	{
		// "If a rotation or shear transformation is in effect in the source device context, BitBlt returns an error"
		b_ret = BitBlt(hdc_canvas, 0, 0, cell_width, cell_height, _hdc_text, bmp_rect.left, bmp_rect.top, SRCCOPY);
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

	for (size_t i = 0; i < _glyphs.size(); i++)
	{
		/*
		Windows DIB and FreeType Bitmap have different ways to indicate bitmap direction
		biHeight > 0 means the Windows DIB is bottom-up
		biHeight < 0 means the Windows DIB is top-down
		pitch > 0 means the FreeType bitmap is down flow
		pitch > 0 means the FreeType bitmap is up flow
		*/
		
		const POINT solid_pos = {_glyph_pos[i].x + (baseline.x - bmp_rect.left), 0};
		const POINT shadow_pos = {solid_pos.x + _setting_cache->shadow.offset_x, solid_pos.y + _setting_cache->shadow.offset_y};

		switch (_glyphs[i]->bitmap.pixel_mode)
		{
		case FT_PIXEL_MODE_MONO:
			set_bmp_bits_mono(_glyphs[i],
				dest_bits,
				solid_pos,
				cell_width,
				cell_ascent,
				cell_descent);
			break;
		case FT_PIXEL_MODE_GRAY:
			if (_setting_cache->shadow.alpha > 0)
				set_bmp_bits_gray(_glyphs[i],
				dest_bits,
				shadow_pos,
				cell_width,
				cell_ascent,
				cell_descent,
				_setting_cache->shadow.alpha);
			set_bmp_bits_gray(_glyphs[i],
				dest_bits,
				solid_pos,
				cell_width,
				cell_ascent,
				cell_descent,
				255);
			break;
		case FT_PIXEL_MODE_LCD:
			if (_setting_cache->shadow.alpha > 0)
				set_bmp_bits_lcd(_glyphs[i],
					dest_bits,
					shadow_pos,
					cell_width,
					cell_ascent,
					cell_descent,
					_setting_cache->shadow.alpha);
			set_bmp_bits_lcd(_glyphs[i],
				dest_bits,
				solid_pos,
				cell_width,
				cell_ascent,
				cell_descent,
				255);
			break;
		}
	}

	// 4.

	b_ret = BitBlt(_hdc_text,
		bmp_rect.left,
		bmp_rect.top,
		cell_width,
		cell_height,
		hdc_canvas,
		0,
		0,
		SRCCOPY);
	assert(b_ret);

	b_ret = DeleteObject(dest_bitmap);
	assert(b_ret);

	b_ret = DeleteDC(hdc_canvas);
	assert(b_ret);
	
	return true;
}

bool gdimm_gdi_text::init()
{
	if (!gdimm_text::init())
		return false;

	// ignore rotated DC
	if (_font_attr.lfEscapement % 3600 != 0)
		return false;

	if (_setting_cache->max_height != 0 && _setting_cache->max_height < _outline_metrics->otmTextMetrics.tmHeight)
		return false;

	return true;
}

bool gdimm_gdi_text::text_out(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
	BOOL b_ret;

	FT_Render_Mode render_mode;
	if (!get_render_mode(_font_face, _font_attr.lfQuality, _bmp_info.bmiHeader.biBitCount, render_mode))
		return false;

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

	_char_extra = GetTextCharacterExtra(_hdc_text);
	assert(_char_extra != 0x8000000);

	bool render_success = render(options, lprect, lpString, c, lpDx, render_mode);
	if (!render_success)
		return false;

	bool draw_success = false;
	if (!_glyphs.empty())
	{
		// get foreground color
		_text_rgb.rgbBlue = GetBValue(_text_color);
		_text_rgb.rgbGreen = GetGValue(_text_color);
		_text_rgb.rgbRed = GetRValue(_text_color);

		if (render_mode == FT_RENDER_MODE_NORMAL)
			_gamma_ramps[0] = gamma_instance.get_ramp(_setting_cache->gamma.gray);
		else if (render_mode & FT_RENDER_MODE_LCD)
		{
			_gamma_ramps[1] = gamma_instance.get_ramp(_setting_cache->gamma.red);
			_gamma_ramps[2] = gamma_instance.get_ramp(_setting_cache->gamma.green);
			_gamma_ramps[3] = gamma_instance.get_ramp(_setting_cache->gamma.blue);
		}

		draw_success = draw_glyphs(x, y, options, lprect);
	}

	// if TA_UPDATECP is set, update current position after text out
	if (update_cursor && draw_success)
	{
		b_ret = MoveToEx(_hdc_text, _cursor.x, _cursor.y, NULL);
		assert(b_ret);
	}

	return draw_success;
}