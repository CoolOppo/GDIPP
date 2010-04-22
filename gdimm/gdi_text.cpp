#include "stdafx.h"
#include "gdi_text.h"
#include "text_helper.h"

POINT gdimm_gdi_text::get_adjusted_origin(const POINT &origin, int width) const
{
	POINT adj_origin = origin;

	switch ((TA_LEFT | TA_RIGHT | TA_CENTER) & _text_alignment)
	{
	case TA_LEFT:
		break;
	case TA_RIGHT:
		adj_origin.x -= width;
		break;
	case TA_CENTER:
		adj_origin.x -= width / 2;
		break;
	}

	switch ((TA_TOP | TA_BOTTOM | TA_BASELINE) & _text_alignment)
	{
	case TA_TOP:
		break;
	case TA_BOTTOM:
		adj_origin.y -= _outline_metrics->otmTextMetrics.tmHeight;
		break;
	case TA_BASELINE:
		adj_origin.y -= _outline_metrics->otmTextMetrics.tmAscent;
		break;
	}

	return adj_origin;
}

void gdimm_gdi_text::set_bmp_bits_mono(const FT_Bitmap &src_bitmap,
	const POINT &src_pos,
	BYTE *dest_bits,
	const POINT &dest_pos,
	const SIZE &dest_size) const
{
	// the source bitmap is 1-bit, 8 pixels per byte, in most-significant order
	// the destination is an non antialiased bitmap with 8, 24 or 32 bpp
	// the source bitmap is not blended with the destination bitmap

	const LONG src_width = src_bitmap.width;
	const LONG src_height = min(src_bitmap.rows - src_pos.y, dest_size.cy);
	const int src_pitch = abs(src_bitmap.pitch);
	const int dest_pitch = get_bmp_pitch(dest_size.cx, _bmp_info.bmiHeader.biBitCount);
	const bool use_zero_color = (*(DWORD*) &_text_rgb == 0);

	for (int j = 0; j < src_height; j++)
	{
		for (int i = 0; i < src_width; i++)
		{
			const int src_ptr = (src_pos.y + j) * src_pitch + (src_pos.x + i) / 8;
			const BYTE src_bit_pos = 7 - (src_pos.x + i) % 8;
			const bool is_bit_set = ((src_bitmap.buffer[src_ptr] & (1 << src_bit_pos)) != 0);

			if (is_bit_set)
			{
				int dest_ptr = (dest_pos.x + i) * _bmp_info.bmiHeader.biBitCount / 8;
				if (dest_ptr < 0)
					continue;

				if (src_bitmap.pitch > 0)
					dest_ptr += max(dest_size.cy - dest_pos.y - j - 1, 0) * dest_pitch;
				else
					dest_ptr += min(dest_pos.y + j, dest_size.cy) * dest_pitch;

				if (_bmp_info.bmiHeader.biBitCount == 1)
				{
					// source bit position might differ from destination bit position
					const BYTE dest_bit_pos = 7 - (dest_pos.x + i) % 8;

					if (use_zero_color)
						dest_bits[dest_ptr] &= ~(1 << dest_bit_pos);
					else
						dest_bits[dest_ptr] |= (1 << dest_bit_pos);
				}
				else
					memcpy(dest_bits + dest_ptr, &_text_rgb, _bmp_info.bmiHeader.biBitCount / 8);
			}
		}
	}
}

void gdimm_gdi_text::set_bmp_bits_gray(const FT_Bitmap &src_bitmap,
	const POINT &src_pos,
	BYTE *dest_bits,
	const POINT &dest_pos,
	const SIZE &dest_size,
	WORD bmp_alpha) const
{
	// the source bitmap is 8-bit with 256 gray levels
	// the destination bitmaps has 8, 24 or 32 bpp
	// each row is aligned to DWORD
	// for LCD destination bitmaps, all color channels have the same value

	assert(_bmp_info.bmiHeader.biBitCount >= 8);

	const LONG src_width = src_bitmap.width;
	const LONG src_height = min(src_bitmap.rows - src_pos.y, dest_size.cy);
	const int src_pitch = abs(src_bitmap.pitch);
	const WORD dest_byte_per_px = _bmp_info.bmiHeader.biBitCount / 8;
	const int dest_pitch = get_bmp_pitch(dest_size.cx, _bmp_info.bmiHeader.biBitCount);

	for (int j = 0; j < src_height; j++)
	{
		for (int i = 0; i < src_width; i++)
		{
			const int src_ptr = (src_pos.y + j) * src_pitch + (src_pos.x + i);

			int dest_ptr = (dest_pos.x + i) * dest_byte_per_px;
			if (dest_ptr < 0)
				continue;

			if (src_bitmap.pitch > 0)
				dest_ptr += max(dest_size.cy - dest_pos.y - j - 1, 0) * dest_pitch;
			else
				dest_ptr += min(dest_pos.y + j, dest_size.cy - 1) * dest_pitch;

			const BYTE src_gray = _gamma_ramps[0][src_bitmap.buffer[src_ptr]];

			if (_bmp_info.bmiHeader.biBitCount == 8)
			{
				// average gray level of the foreground color
				const BYTE fg_gray = (_text_rgb.rgbRed + _text_rgb.rgbGreen + _text_rgb.rgbBlue) / 765;
				const BYTE dest_gray = (src_gray * fg_gray + (255 - src_gray) * dest_bits[dest_ptr]) / 255;

				dest_bits[dest_ptr] = (bmp_alpha * dest_gray + (255 - bmp_alpha) * dest_bits[dest_ptr]) / 255;
			}
			else
			{
				const BYTE dest_r = (src_gray * _text_rgb.rgbRed + (255 - src_gray) * dest_bits[dest_ptr+2]) / 255;
				const BYTE dest_g = (src_gray * _text_rgb.rgbGreen + (255 - src_gray) * dest_bits[dest_ptr+1]) / 255;
				const BYTE dest_b = (src_gray * _text_rgb.rgbBlue + (255 - src_gray) * dest_bits[dest_ptr]) / 255;

				dest_bits[dest_ptr] = (bmp_alpha * dest_b + (255 - bmp_alpha) * dest_bits[dest_ptr]) / 255;
				dest_bits[dest_ptr+1] = (bmp_alpha * dest_g + (255 - bmp_alpha) * dest_bits[dest_ptr+1]) / 255;
				dest_bits[dest_ptr+2] = (bmp_alpha * dest_r + (255 - bmp_alpha) * dest_bits[dest_ptr+2]) / 255;
			}

			//if (_bmp_info.bmiHeader.biBitCount == 32)
			//	dest_bits[dest_ptr+3] = 0;
		}
	}
}

void gdimm_gdi_text::set_bmp_bits_lcd(const FT_Bitmap &src_bitmap,
	const POINT &src_pos,
	BYTE *dest_bits,
	const POINT &dest_pos,
	const SIZE &dest_size,
	WORD bmp_alpha) const
{
	// the source bitmap is 24-bit, in order of R, G, B channels
	// the destination bitmaps has 24 or 32 bpp, in order of B, G, R, (A) channels
	// each row is aligned to DWORD

	assert(_bmp_info.bmiHeader.biBitCount >= 24);

	const WORD src_byte_per_px = 3;
	const LONG src_width = src_bitmap.width / src_byte_per_px;
	const LONG src_height = min(src_bitmap.rows - src_pos.y, dest_size.cy);
	const int src_pitch = abs(src_bitmap.pitch);
	const WORD dest_byte_per_px = _bmp_info.bmiHeader.biBitCount / 8;
	const int dest_pitch = get_bmp_pitch(dest_size.cx, _bmp_info.bmiHeader.biBitCount);

	// rows about to be copied
	for (int j = 0; j < src_height; j++)
	{
		// width about to be copied
		for (int i = 0; i < src_width; i++)
		{
			// source byte, always treat as down flow
			const int src_ptr = (src_pos.y + j) * src_pitch + (src_pos.x + i) * src_byte_per_px;

			// destination byte, compute according to two flow directions
			int dest_ptr = (dest_pos.x + i) * dest_byte_per_px;
			if (dest_ptr < 0)
				continue;

			if (src_bitmap.pitch > 0)
				dest_ptr += max(dest_size.cy - dest_pos.y - j - 1, 0) * dest_pitch;
			else
				dest_ptr += min(dest_pos.y + j, dest_size.cy - 1) * dest_pitch;

			const BYTE src_r = _gamma_ramps[1][src_bitmap.buffer[src_ptr]];
			const BYTE src_g = _gamma_ramps[2][src_bitmap.buffer[src_ptr+1]];
			const BYTE src_b = _gamma_ramps[3][src_bitmap.buffer[src_ptr+2]];
			
			const BYTE dest_r = (src_r * _text_rgb.rgbRed + (255 - src_r) * dest_bits[dest_ptr+2]) / 255;
			const BYTE dest_g = (src_g * _text_rgb.rgbGreen + (255 - src_g) * dest_bits[dest_ptr+1]) / 255;
			const BYTE dest_b = (src_b * _text_rgb.rgbBlue + (255 - src_b) * dest_bits[dest_ptr]) / 255;

			// same algorithm as the AlphaBlend API
			dest_bits[dest_ptr] = (bmp_alpha * dest_b + (255 - bmp_alpha) * dest_bits[dest_ptr]) / 255;
			dest_bits[dest_ptr+1] = (bmp_alpha * dest_g + (255 - bmp_alpha) * dest_bits[dest_ptr+1]) / 255;
			dest_bits[dest_ptr+2] = (bmp_alpha * dest_r + (255 - bmp_alpha) * dest_bits[dest_ptr+2]) / 255;

			if (_bmp_info.bmiHeader.biBitCount == 32 && _setting_cache->zero_alpha)
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
bool gdimm_gdi_text::draw_glyphs(UINT options,
	CONST RECT *lprect) const
{
	BOOL b_ret;

	// 1.

	// origin of the source glyphs
	const POINT src_origin = _glyph_pos[0];

	const RECT bmp_rect = get_glyph_bmp_rect(_glyphs, _glyph_pos, _cursor);
	const LONG bmp_width = bmp_rect.right - bmp_rect.left;

	// respect the height and ascent returned from GDI
	const int cell_height = _outline_metrics->otmTextMetrics.tmHeight;
	const int cell_ascent = _outline_metrics->otmTextMetrics.tmAscent;

	// position where the bitmap will be finally drawn if no clipping is needed
	const POINT dest_origin = get_adjusted_origin(src_origin, bmp_width);

	// 2.

	/*
	there is no "official" way to get the direction of the bitmap in the text HDC
	it seems BitBlt would automatically convert the direction of its source bitmap if necessary
	therefore we just use bottom-up, as it is more compatible
	*/

	BITMAPINFO bmi = {};
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

	const COLORREF bg_color = GetBkColor(_hdc_text);

	if (options & ETO_OPAQUE)
		draw_background(_hdc_text, lprect, bg_color);
	
	const int bk_mode = GetBkMode(_hdc_text);
	if (bk_mode == OPAQUE)
	{
		const RECT bk_rect = {0, 0, bmp_width, cell_height};
		b_ret = draw_background(hdc_canvas, &bk_rect, bg_color);
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

	for (size_t i = 0; i < _glyphs.size(); i++)
	{
		/*
		Windows DIB and FreeType Bitmap have different ways to indicate bitmap direction
		biHeight > 0 means the Windows DIB is bottom-up
		biHeight < 0 means the Windows DIB is top-down
		pitch > 0 means the FreeType bitmap is down flow
		pitch > 0 means the FreeType bitmap is up flow
		*/

		const POINT glyph_src_pos = {0, max(_glyphs[i]->top - cell_ascent, 0)};
		const POINT glyph_dest_pos = {_glyph_pos[i].x - bmp_rect.left, max(cell_ascent - _glyphs[i]->top, 0)};
		const POINT shadow_src_pos = {max(glyph_src_pos.x - _setting_cache->shadow.offset_x, 0), max(glyph_src_pos.y - _setting_cache->shadow.offset_y, 0)};
		const POINT shadow_dest_pos = {max(glyph_dest_pos.x + _setting_cache->shadow.offset_x, 0), max(glyph_dest_pos.y + _setting_cache->shadow.offset_y, 0)};
		const SIZE dest_size = {bmp_width, cell_height};

		switch (_glyphs[i]->bitmap.pixel_mode)
		{
		case FT_PIXEL_MODE_MONO:
			set_bmp_bits_mono(_glyphs[i]->bitmap,
				glyph_src_pos,
				dest_bits,
				glyph_dest_pos,
				dest_size);
			break;
		case FT_PIXEL_MODE_GRAY:
			if (_setting_cache->shadow.alpha > 0)
				set_bmp_bits_gray(_glyphs[i]->bitmap,
					shadow_src_pos,
					dest_bits,
					shadow_dest_pos,
					dest_size,
					_setting_cache->shadow.alpha);
			set_bmp_bits_gray(_glyphs[i]->bitmap,
				glyph_src_pos,
				dest_bits,
				glyph_dest_pos,
				dest_size,
				255);
			break;
		case FT_PIXEL_MODE_LCD:
			if (_setting_cache->shadow.alpha > 0)
				set_bmp_bits_lcd(_glyphs[i]->bitmap,
					shadow_src_pos,
					dest_bits,
					shadow_dest_pos,
					dest_size,
					_setting_cache->shadow.alpha);
			set_bmp_bits_lcd(_glyphs[i]->bitmap,
				glyph_src_pos,
				dest_bits,
				glyph_dest_pos,
				dest_size,
				255);
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
		b_ret = BitBlt(_hdc_text,
			dest_origin.x,
			dest_origin.y,
			bmp_width,
			cell_height,
			hdc_canvas,
			0,
			0,
			SRCCOPY | NOMIRRORBITMAP);
		assert(b_ret);
	}

	b_ret = DeleteObject(dest_bitmap);
	assert(b_ret);

	b_ret = DeleteDC(hdc_canvas);
	assert(b_ret);

	return true;
}

bool gdimm_gdi_text::init(HDC hdc)
{
	if (!gdimm_text::init(hdc))
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

		if (render_mode & FT_RENDER_MODE_NORMAL)
			_gamma_ramps[0] = gamma_instance.get_ramp(_setting_cache->gamma.gray);
		else if (render_mode & FT_RENDER_MODE_LCD)
		{
			_gamma_ramps[1] = gamma_instance.get_ramp(_setting_cache->gamma.red);
			_gamma_ramps[2] = gamma_instance.get_ramp(_setting_cache->gamma.green);
			_gamma_ramps[3] = gamma_instance.get_ramp(_setting_cache->gamma.blue);
		}

		draw_success = draw_glyphs(options, lprect);
	}

	// if TA_UPDATECP is set, update current position after text out
	if (update_cursor && draw_success)
	{
		b_ret = MoveToEx(_hdc_text, _cursor.x, _cursor.y, NULL);
		assert(b_ret);
	}

	return draw_success;
}