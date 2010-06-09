#include "stdafx.h"
#include "gdi_text.h"
#include "text_helper.h"
#include "gdimm.h"

gdimm_gdi_text::gdimm_gdi_text()
:
_hdc_canvas(NULL)
{
}

gdimm_gdi_text::~gdimm_gdi_text()
{
	// graceful memory DC deletion
	// note that memory DCs are automatically deleted when the current thread is terminated
	if (_hdc_canvas != NULL)
		DeleteDC(_hdc_canvas);
}

void gdimm_gdi_text::set_mono_mask_bits(const FT_BitmapGlyph glyph,
	BYTE *dest_bits,
	POINT dest_pos,
	int dest_width,
	int dest_height,
	int dest_ascent) const
{
	// the source bitmap is 1bpp, 8 pixels per byte, in most-significant order
	// the destination bitmap is 1bpp, 8 pixels per byte, in most-significant order
	// the source bitmap is not blended with the destination bitmap

	const int top_extra_rows = glyph->top - dest_ascent - dest_pos.y;
	const int bottom_end_row = top_extra_rows + dest_height;

	const int bmp_width = glyph->bitmap.width;
	const int bmp_height = glyph->bitmap.rows;
	const int dest_pitch = get_bmp_pitch(dest_width, 1);

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
		
		for (int i = 0; i < bmp_width; i++)
		{
			if (dest_pos.x + i < 0)
				continue;

			if (dest_pos.x + i >= dest_width)
				break;

			// pointers to the current pixel
			const int src_px_ptr = src_row_ptr + i / 8;
			const BYTE src_bit_pos = 7 - i % 8;

			const int dest_px_ptr = dest_row_ptr + (dest_pos.x + i) / 8;
			const BYTE dest_bit_pos = 7 - (dest_pos.x + i) % 8;

			const bool is_bit_set = ((glyph->bitmap.buffer[src_px_ptr] & (1 << src_bit_pos)) != 0);

			if (is_bit_set)
				dest_bits[dest_px_ptr] |= (1 << dest_bit_pos);
			else
				dest_bits[dest_px_ptr] &= ~(1 << dest_bit_pos);
		}
	}
}

void gdimm_gdi_text::set_gray_text_bits(const FT_BitmapGlyph glyph,
	BYTE *dest_bits,
	POINT dest_pos,
	int dest_width,
	int dest_height,
	int dest_ascent) const
{
	// the source bitmap is 8bpp with 256 gray levels
	// the destination bitmap is 32 bpp, in order of B, G, R, A channels
	// each row is aligned to DWORD
	// for LCD destination bitmaps, all color channels have the same value

	const int top_extra_rows = glyph->top - dest_ascent - dest_pos.y;
	const int bottom_end_row = top_extra_rows + dest_height;

	const WORD bmp_byte_per_px = 1;
	const int bmp_width = glyph->bitmap.width / bmp_byte_per_px;
	const int bmp_height = glyph->bitmap.rows;
	const WORD dest_byte_per_px = 4;
	const int dest_pitch = get_bmp_pitch(dest_width, 32);

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

			const BYTE src_alpha = glyph->bitmap.buffer[src_px_ptr];
			const RGBTRIPLE src_color = {_gamma_ramps[2][MulDiv(_text_rgb.rgbtBlue, src_alpha, 255)],
				_gamma_ramps[1][MulDiv(_text_rgb.rgbtGreen, src_alpha, 255)],
				_gamma_ramps[0][MulDiv(_text_rgb.rgbtRed, src_alpha, 255)]};

			// this approach is faster than setting each byte separately
			*(DWORD*)(dest_bits + dest_px_ptr) = ((src_alpha<<24) + (src_color.rgbtRed<<16) + (src_color.rgbtGreen<<8) + (src_color.rgbtBlue));
		}
	}
}

void gdimm_gdi_text::set_lcd_text_bits(const FT_BitmapGlyph glyph,
	BYTE *dest_bits,
	POINT dest_pos,
	int dest_width,
	int dest_height,
	int dest_ascent,
	WORD alpha) const
{
	// the source bitmap is 24bpp, in order of R, G, B channels
	// the destination bitmaps is 32bpp, in order of B, G, R, A channels
	// each row is aligned to DWORD

	const int top_extra_rows = glyph->top - dest_ascent - dest_pos.y;
	const int bottom_end_row = top_extra_rows + dest_height;

	const WORD bmp_byte_per_px = 3;
	const int bmp_width = glyph->bitmap.width / bmp_byte_per_px;
	const int bmp_height = glyph->bitmap.rows;
	const WORD dest_byte_per_px = 4;
	const int dest_pitch = get_bmp_pitch(dest_width, 32);

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

			// color components of the source bitmap
			const RGBTRIPLE src_color = {_gamma_ramps[2][MulDiv(_text_rgb.rgbtBlue, alpha, 255)],
				_gamma_ramps[1][MulDiv(_text_rgb.rgbtGreen, alpha, 255)],
				_gamma_ramps[0][MulDiv(_text_rgb.rgbtRed, alpha, 255)]};

			// alpha components of the source bitmap
			// apply pixel geometry
			RGBTRIPLE src_alpha;
			if (_context->setting_cache->render_mode.pixel_geometry == PIXEL_GEOMETRY_BGR)
			{
				src_alpha.rgbtRed = MulDiv(glyph->bitmap.buffer[src_px_ptr+2], alpha, 255);
				src_alpha.rgbtGreen = MulDiv(glyph->bitmap.buffer[src_px_ptr+1], alpha, 255);
				src_alpha.rgbtBlue = MulDiv(glyph->bitmap.buffer[src_px_ptr], alpha, 255);
			}
			else
			{
				src_alpha.rgbtRed = MulDiv(glyph->bitmap.buffer[src_px_ptr], alpha, 255);
				src_alpha.rgbtGreen = MulDiv(glyph->bitmap.buffer[src_px_ptr+1], alpha, 255);
				src_alpha.rgbtBlue = MulDiv(glyph->bitmap.buffer[src_px_ptr+2], alpha, 255);
			}

			/*
			GDI's behavior:
			if the destination pixel is modified, reset its alpha value to 0
			otherwise, leave the alpha value untouched
			*/
			BYTE dest_alpha = 0;
			if (src_alpha.rgbtBlue == 0 && src_alpha.rgbtGreen == 0 && src_alpha.rgbtRed == 0)
				dest_alpha = dest_bits[dest_px_ptr+3];

			// alpha blending
			const RGBTRIPLE dest_color = {dest_bits[dest_px_ptr] + MulDiv(src_color.rgbtBlue - dest_bits[dest_px_ptr], src_alpha.rgbtBlue, 255),
				dest_bits[dest_px_ptr+1] + MulDiv(src_color.rgbtGreen - dest_bits[dest_px_ptr+1], src_alpha.rgbtGreen, 255),
				dest_bits[dest_px_ptr+2] + MulDiv(src_color.rgbtRed - dest_bits[dest_px_ptr+2], src_alpha.rgbtRed, 255)};

			// this approach is faster than setting each byte separately
			*(DWORD*)(dest_bits + dest_px_ptr) = ((dest_alpha<<24) + (dest_color.rgbtRed<<16) + (dest_color.rgbtGreen<<8) + (dest_color.rgbtBlue));
		}
	}
}

bool gdimm_gdi_text::draw_mono(const text_metrics &metrics,	UINT options, CONST RECT *lprect) const
{
	BOOL b_ret;

	/*
	the flow direction of the original bitmap is unrecoverable
	it seems BitBlt would automatically convert the direction of its source bitmap if necessary
	FreeType returns top-down bitmap, so we use top-down to simplify algorithm
	note that the semantic of the direction from GDI is opposite to FreeType
	*/

	/*
	Windows DIB and FreeType Bitmap have different ways to indicate bitmap direction
	biHeight > 0 means the Windows DIB is bottom-up
	biHeight < 0 means the Windows DIB is top-down
	pitch > 0 means the FreeType bitmap is down flow
	pitch > 0 means the FreeType bitmap is up flow
	*/

	BITMAPINFOHEADER bmp_header = {};
	bmp_header.biSize = sizeof(BITMAPINFOHEADER);
	bmp_header.biWidth = metrics.width;
	bmp_header.biHeight = -metrics.height;
	bmp_header.biPlanes = 1;
	bmp_header.biBitCount = 1;
	bmp_header.biCompression = BI_RGB;

	BYTE *mask_bits;
	const HBITMAP mask_bitmap = CreateDIBSection(_context->hdc, (BITMAPINFO *)&bmp_header, DIB_RGB_COLORS, (VOID **)&mask_bits, NULL, 0);
	assert(mask_bitmap != NULL);

	/*
	both ETO_OPAQUE and OPAQUE background mode need background filled
	for ETO_OPAQUE, direct FillRect to the physical DC
	for OPAQUE background mode, draw the background on canvas DC (it might be clipped eventually)
	*/

	if (options & ETO_OPAQUE)
		draw_background(_context->hdc, lprect, _bg_color);
	
	const int bk_mode = GetBkMode(_context->hdc);
	if (bk_mode == OPAQUE)
	{
		RECT bk_rect;
		bk_rect.left = metrics.origin.x;
		bk_rect.top =  metrics.origin.y;
		bk_rect.right = bk_rect.left + metrics.width;
		bk_rect.bottom = bk_rect.top + metrics.height;
		draw_background(_context->hdc, &bk_rect, _bg_color);
	}

	for (size_t i = 0; i < _glyphs.size(); i++)
	{
		assert(_glyphs[i]->bitmap.pitch >= 0);

		const POINT solid_pos = {_glyph_pos[i].x + (metrics.baseline.x - metrics.origin.x),
			((metrics.baseline.y - metrics.ascent) - metrics.origin.y)};
		
		set_mono_mask_bits(_glyphs[i],
			mask_bits,
			solid_pos,
			metrics.width,
			metrics.height,
			metrics.ascent);
	}

	// monochrome bitmap has no shadow (because no alpha blending)

	HBRUSH text_brush = CreateSolidBrush(_text_color);
	assert(text_brush != NULL);
	HBRUSH prev_brush = (HBRUSH) SelectObject(_context->hdc, text_brush);

	// foreground ROP: source brush
	// background ROP: destination color
	const BOOL draw_success = MaskBlt(_context->hdc,
		metrics.origin.x,
		metrics.origin.y,
		metrics.width,
		metrics.height,
		_context->hdc,
		0,
		0,
		mask_bitmap,
		0,
		0,
		MAKEROP4(PATCOPY, 0x00AA0029));

	SelectObject(_context->hdc, prev_brush);
	b_ret = DeleteObject(text_brush);
	assert(b_ret);
	b_ret = DeleteObject(mask_bitmap);
	assert(b_ret);

	return !!draw_success;
}

bool gdimm_gdi_text::draw_gray(const text_metrics &metrics, UINT options, CONST RECT *lprect) const
{
	BOOL b_ret;

	BITMAPINFOHEADER bmp_header = {};
	bmp_header.biSize = sizeof(BITMAPINFOHEADER);
	bmp_header.biWidth = metrics.width;
	bmp_header.biHeight = -metrics.height;
	bmp_header.biPlanes = 1;
	bmp_header.biBitCount = 32;
	bmp_header.biCompression = BI_RGB;

	BYTE *text_bits;
	const HBITMAP text_bitmap = CreateDIBSection(_context->hdc, (BITMAPINFO *)&bmp_header, DIB_RGB_COLORS, (VOID **)&text_bits, NULL, 0);
	assert(text_bitmap != NULL);

	if (options & ETO_OPAQUE)
		draw_background(_context->hdc, lprect, _bg_color);

	const int bk_mode = GetBkMode(_context->hdc);
	if (bk_mode == OPAQUE)
	{
		RECT bk_rect;
		bk_rect.left = metrics.origin.x;
		bk_rect.top =  metrics.origin.y;
		bk_rect.right = bk_rect.left + metrics.width;
		bk_rect.bottom = bk_rect.top + metrics.height;
		draw_background(_context->hdc, &bk_rect, _bg_color);
	}

	for (size_t i = 0; i < _glyphs.size(); i++)
	{
		assert(_glyphs[i]->bitmap.pitch >= 0);

		const POINT solid_pos = {_glyph_pos[i].x + (metrics.baseline.x - metrics.origin.x),
			((metrics.baseline.y - metrics.ascent) - metrics.origin.y)};

		set_gray_text_bits(_glyphs[i],
			text_bits,
			solid_pos,
			metrics.width,
			metrics.height,
			metrics.ascent);
	}

	HDC _hdc_canvas = CreateCompatibleDC(_context->hdc);
	assert(_hdc_canvas != NULL);
	SelectObject(_hdc_canvas, text_bitmap);

	BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};

	// AlphaBlend converts the source bitmap pixel format to match destination bitmap pixel format
	if (_context->setting_cache->shadow.alpha != 0)
	{
		bf.SourceConstantAlpha = (BYTE) _context->setting_cache->shadow.alpha;
		b_ret = AlphaBlend(_context->hdc,
			metrics.origin.x + _context->setting_cache->shadow.offset_x,
			metrics.origin.y + _context->setting_cache->shadow.offset_y,
			metrics.width,
			metrics.height,
			_hdc_canvas,
			0,
			0,
			metrics.width,
			metrics.height,
			bf);
		if (!b_ret)
			return false;
	}

	bf.SourceConstantAlpha = 255;
	const BOOL ab_success = AlphaBlend(_context->hdc,
		metrics.origin.x,
		metrics.origin.y,
		metrics.width,
		metrics.height,
		_hdc_canvas,
		0,
		0,
		metrics.width,
		metrics.height,
		bf);

	b_ret = DeleteObject(text_bitmap);
	assert(b_ret);
	b_ret = DeleteDC(_hdc_canvas);
	assert(b_ret);

	return !!ab_success;
}

bool gdimm_gdi_text::draw_lcd(const text_metrics &metrics, UINT options, CONST RECT *lprect) const
{
	BOOL b_ret, draw_success;

	BITMAPINFOHEADER bmp_header = {};
	bmp_header.biSize = sizeof(BITMAPINFOHEADER);
	bmp_header.biWidth = metrics.width;
	bmp_header.biHeight = -metrics.height;
	bmp_header.biPlanes = 1;
	bmp_header.biBitCount = 32;
	bmp_header.biCompression = BI_RGB;

	BYTE *text_bits;
	HBITMAP text_bitmap = CreateDIBSection(_context->hdc, (BITMAPINFO *)&bmp_header, DIB_RGB_COLORS, (VOID **)&text_bits, NULL, 0);
	assert(text_bitmap != NULL);

	SelectObject(_hdc_canvas, text_bitmap);
	
	if (options & ETO_OPAQUE)
		draw_background(_context->hdc, lprect, _bg_color);

	const int bk_mode = GetBkMode(_context->hdc);
	if (bk_mode == OPAQUE)
	{
		const RECT bk_rect = {0, 0, metrics.width, metrics.height};
		draw_success = draw_background(_hdc_canvas, &bk_rect, _bg_color);
	}
	else if (bk_mode == TRANSPARENT)
	{
		// "If a rotation or shear transformation is in effect in the source device context, BitBlt returns an error"
		draw_success = BitBlt(_hdc_canvas,
			0,
			0,
			metrics.width,
			metrics.height,
			_context->hdc,
			metrics.origin.x,
			metrics.origin.y,
			SRCCOPY);
	}

	if (draw_success)
	{
		for (size_t i = 0; i < _glyphs.size(); i++)
		{
			assert(_glyphs[i]->bitmap.pitch >= 0);

			const POINT solid_pos = {_glyph_pos[i].x + (metrics.baseline.x - metrics.origin.x),
				((metrics.baseline.y - metrics.ascent) - metrics.origin.y)};

			if (_context->setting_cache->shadow.alpha > 0)
			{
				set_lcd_text_bits(_glyphs[i],
					text_bits,
					solid_pos,
					metrics.width,
					metrics.height,
					metrics.ascent,
					_context->setting_cache->shadow.alpha);
			}

			set_lcd_text_bits(_glyphs[i],
				text_bits,
				solid_pos,
				metrics.width,
				metrics.height,
				metrics.ascent,
				255);
		}

		draw_success = BitBlt(_context->hdc,
			metrics.origin.x,
			metrics.origin.y,
			metrics.width,
			metrics.height,
			_hdc_canvas,
			0,
			0,
			SRCCOPY);
	}

	b_ret = DeleteObject(text_bitmap);
	assert(b_ret);

	return !!draw_success;
}

bool gdimm_gdi_text::draw_text(UINT options, CONST RECT *lprect)
{
	assert(!_glyphs.empty());
	assert(_glyphs.size() == _glyph_pos.size());

	// actual rect occupied by the glyphs
	RECT glyph_run_rect = get_ft_glyph_run_rect(_glyphs, _glyph_pos);
	text_metrics metrics;

	metrics.width = glyph_run_rect.right - glyph_run_rect.left;
	metrics.height = _context->outline_metrics->otmTextMetrics.tmHeight;
	metrics.ascent = _context->outline_metrics->otmTextMetrics.tmAscent;
	metrics.descent = _context->outline_metrics->otmTextMetrics.tmDescent;

	// adjusted baseline where the bitmap will be finally drawn before applying clipping
	metrics.baseline = get_baseline(_text_alignment,
		_cursor.x,
		_cursor.y,
		metrics.width,
		metrics.ascent,
		metrics.descent);

	metrics.origin.x = metrics.baseline.x;
	metrics.origin.y = metrics.baseline.y - metrics.ascent;

	_cursor.x += metrics.width;
	_cursor.y += metrics.height;
	
	// apply clipping
	if (options & ETO_CLIPPED)
	{
		RECT bmp_rect = {metrics.origin.x,
			metrics.origin.y,
			metrics.origin.x + metrics.width,
			metrics.origin.y + metrics.height};
		if (!IntersectRect(&bmp_rect, &bmp_rect, lprect))
			bmp_rect = *lprect;

		metrics.width = bmp_rect.right - bmp_rect.left;
		metrics.height = bmp_rect.bottom - bmp_rect.top;
		metrics.origin.x = bmp_rect.left;
		metrics.origin.y = bmp_rect.top;
	}
	
	switch (_render_mode)
	{
		case FT_RENDER_MODE_NORMAL:
		case FT_RENDER_MODE_LIGHT:
			return draw_gray(metrics, options, lprect);
		case FT_RENDER_MODE_MONO:
			return draw_mono(metrics, options, lprect);
		case FT_RENDER_MODE_LCD:
			return draw_lcd(metrics, options, lprect);
		default:
			return false;
	}
}

bool gdimm_gdi_text::begin(const gdimm_text_context *context)
{
	if (!gdimm_text::begin(context))
		return false;

	// ignore rotated DC
	if (_font_attr.lfEscapement % 3600 != 0)
		return false;

	if (_hdc_canvas == NULL)
	{
		_hdc_canvas = CreateCompatibleDC(NULL);
		assert(_hdc_canvas != NULL);
	}

	_glyphs.clear();
	_glyph_pos.clear();

	return true;
}

bool gdimm_gdi_text::text_out(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
	BOOL b_ret;

	BOOL update_cursor;
	if (((TA_NOUPDATECP | TA_UPDATECP) & _text_alignment) == TA_UPDATECP)
	{
		POINT cp;
		b_ret = GetCurrentPositionEx(_context->hdc, &cp);
		assert(b_ret);

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

	bool render_success = render(options, lpString, c, lpDx);
	if (!render_success)
		return false;

	render_success = !_glyphs.empty();
	if (render_success)
	{
		// get foreground color
		_text_rgb.rgbtBlue = GetBValue(_text_color);
		_text_rgb.rgbtGreen = GetGValue(_text_color);
		_text_rgb.rgbtRed = GetRValue(_text_color);

		_gamma_ramps[0] = gamma_instance.get_ramp(_context->setting_cache->gamma.red);
		_gamma_ramps[1] = gamma_instance.get_ramp(_context->setting_cache->gamma.green);
		_gamma_ramps[2] = gamma_instance.get_ramp(_context->setting_cache->gamma.blue);

		render_success = draw_text(options, lprect);
	}

	// if TA_UPDATECP is set, update current position after text out
	if (update_cursor && render_success)
	{
		b_ret = MoveToEx(_context->hdc, _cursor.x, _cursor.y, NULL);
		assert(b_ret);
	}

	return render_success;
}