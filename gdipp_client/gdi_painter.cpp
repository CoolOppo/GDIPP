#include "stdafx.h"
#include "gdi_painter.h"
#include "gdipp_client/global.h"

namespace gdipp
{

DWORD tls_index = 0;

bool gdi_painter::begin(const dc_context *context)
{
	if (!painter::begin(context))
		return false;

	// ignore rotated DC
	if (_context->log_font.lfEscapement % 3600 != 0)
		return false;

	if (tls_index == 0)
		tls_index = TlsAlloc();

	_tls = reinterpret_cast<painter_tls *>(TlsGetValue(tls_index));
	if (_tls == NULL)
	{
		_tls = reinterpret_cast<painter_tls *>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(painter_tls)));
		TlsSetValue(tls_index, _tls);

		_tls->hdc_canvas = CreateCompatibleDC(NULL);
		_tls->last_hdc = NULL;
	}

	return true;
}

bool gdi_painter::paint(int x, int y, UINT options, CONST RECT *lprect, const void *text, UINT c, CONST INT *lpDx)
{
	BOOL b_ret, paint_success = FALSE;

	if (((TA_NOUPDATECP | TA_UPDATECP) & _text_alignment) == TA_UPDATECP)
	{
		POINT cp;
		b_ret = GetCurrentPositionEx(_context->hdc, &cp);
		assert(b_ret);

		_cursor.x = cp.x;
		_cursor.y = cp.y;
		_update_cp = true;
	}
	else
	{
		_cursor.x = x;
		_cursor.y = y;
		_update_cp = false;

		if (_tls->last_hdc != NULL)
			_tls->last_hdc = NULL;
	}

	const BYTE *gamma_ramps[3] = {gamma_instance.get_ramp(client_config_instance.gamma.red),
		gamma_instance.get_ramp(client_config_instance.gamma.green),
		gamma_instance.get_ramp(client_config_instance.gamma.blue)};

	_text_rgb_gamma.rgbRed = gamma_ramps[0][GetRValue(_text_color)];
	_text_rgb_gamma.rgbGreen = gamma_ramps[1][GetGValue(_text_color)];
	_text_rgb_gamma.rgbBlue = gamma_ramps[2][GetBValue(_text_color)];

	const gdipp_rpc_bitmap_glyph_run *a_glyph_run = reinterpret_cast<const gdipp_rpc_bitmap_glyph_run *>(text);
	if (lpDx == NULL)
		paint_success = paint_glyph_run(options, lprect, a_glyph_run);
	else
	{
		gdipp_rpc_bitmap_glyph_run adjusted_glyph_run = *a_glyph_run;
		adjust_glyph_bbox(!!(options & ETO_PDY), c, lpDx, &adjusted_glyph_run);
		paint_success = paint_glyph_run(options, lprect, &adjusted_glyph_run);
	}

	// if TA_UPDATECP is set, update current position after text out
	if (_update_cp && paint_success)
	{
		_tls->last_hdc = _context->hdc;

		b_ret = MoveToEx(_context->hdc, _cursor.x, _cursor.y, NULL);
		assert(b_ret);
	}

	return !!paint_success;
}

void gdi_painter::adjust_glyph_bbox(bool is_pdy, UINT count, CONST INT *lpDx, gdipp_rpc_bitmap_glyph_run *a_glyph_run)
{
	/*
	the DX array stores the distance from the left border of the current glyph to the next glyph
	the count of such array should be no less than the count of glyphs
	the last element indicates the distance between the right border of the control box of the glyph run and the left border of the last glyph
	if PDY flag is set, every 2 elements specifies the distance of the glyph in both X and Y axis

	gdipp matches the metrics of the glyph control box against the DX array, then adjusts black box accordingly
	DX array is application specific, therefore the boxes after adjustment are not cached
	*/

	assert(lpDx != NULL);

	const BYTE dx_skip = (is_pdy ? 2 : 1);
	INT curr_left = 0;
	INT last_right;

	std::list<RECT>::iterator ctrl_iter, black_iter;
	UINT i;
	for (ctrl_iter = a_glyph_run->ctrl_boxes.begin(), black_iter = a_glyph_run->black_boxes.begin(), i = 0;
		i < count; ++ctrl_iter, ++black_iter, ++i)
	{
		assert(ctrl_iter != a_glyph_run->ctrl_boxes.end());

		if (i != 0)
		{
			// distance to shift right
			const int distance_shift = curr_left - ctrl_iter->left;

			ctrl_iter->left += distance_shift;
			ctrl_iter->right += distance_shift;
			black_iter->left += distance_shift;
			black_iter->right += distance_shift;
		}

		curr_left += lpDx[i * dx_skip];
		last_right = black_iter->right;
	}

	a_glyph_run->glyphs.push_back(NULL);

	const RECT ctrl_right = {curr_left, 0, curr_left, 0};
	a_glyph_run->ctrl_boxes.push_back(ctrl_right);

	last_right = max(last_right, curr_left);
	const RECT black_right = {last_right, 0, last_right, 0};
	a_glyph_run->black_boxes.push_back(black_right);
}

void gdi_painter::set_mono_mask_bits(const FT_BitmapGlyph glyph,
	const RECT &src_rect,
	BYTE *dest_bits,
	const RECT &dest_rect,
	int dest_pitch,
	bool project_bottom) const
{
	// the source bitmap is 1bpp, 8 pixels per byte, in most-significant order
	// the destination bitmap is 1bpp, 8 pixels per byte, in most-significant order
	// the source bitmap is not blended with the destination bitmap

	int src_row_ptr = src_rect.top * glyph->bitmap.pitch;
	int dest_row_ptr = dest_rect.top * dest_pitch;

	for (int src_curr_row = src_rect.top; src_curr_row < glyph->bitmap.rows; ++src_curr_row, src_row_ptr += glyph->bitmap.pitch)
	{
		int src_curr_column, dest_curr_column;
		for (src_curr_column = src_rect.left, dest_curr_column = dest_rect.left; src_curr_column < src_rect.right; ++src_curr_column, ++dest_curr_column)
		{
			// pointers to the current pixel
			// pointing to the beginning of a row
			const int src_px_ptr = src_row_ptr + src_curr_column / 8;
			const int dest_px_ptr = dest_row_ptr + dest_curr_column / 8;
			const BYTE src_bit_pos = 7 - src_curr_column % 8;
			const BYTE dest_bit_pos = 7 - dest_curr_column % 8;

			const bool is_bit_set = !!(glyph->bitmap.buffer[src_px_ptr] & (1 << src_bit_pos));

			if (is_bit_set)
				dest_bits[dest_px_ptr] |= (1 << dest_bit_pos);
			else
				dest_bits[dest_px_ptr] &= ~(1 << dest_bit_pos);
		}

		if (src_curr_row < src_rect.bottom - 1)
			dest_row_ptr += dest_pitch;
		else if (!project_bottom)
			break;
	}
}

void gdi_painter::set_gray_text_bits(const FT_BitmapGlyph glyph,
	const RECT &src_rect,
	BYTE *dest_bits,
	const RECT &dest_rect,
	int dest_pitch,
	bool project_bottom) const
{
	// the source bitmap is 8bpp with 256 gray levels
	// the destination bitmap is 32 bpp, in order of B, G, R, A channels
	// each row is aligned to DWORD
	// for LCD destination bitmaps, all color channels have the same value

	const WORD src_byte_per_px = 1;
	const WORD dest_byte_per_px = 4;

	int src_row_ptr = src_rect.top * glyph->bitmap.pitch;
	int dest_row_ptr = dest_rect.top * dest_pitch;

	for (int src_curr_row = src_rect.top; src_curr_row < glyph->bitmap.rows; ++src_curr_row, src_row_ptr += glyph->bitmap.pitch)
	{
		// pointers to the current pixel
		// pointing to the beginning of a row
		int src_px_ptr = src_row_ptr + src_rect.left * src_byte_per_px;
		int dest_px_ptr = dest_row_ptr + dest_rect.left * dest_byte_per_px;

		for (int src_curr_column = src_rect.left; src_curr_column < src_rect.right; ++src_curr_column, src_px_ptr += src_byte_per_px, dest_px_ptr += dest_byte_per_px)
		{
			const BYTE src_alpha = glyph->bitmap.buffer[src_px_ptr];
			const RGBQUAD src_color = {division_by_255(_text_rgb_gamma.rgbBlue, src_alpha),
				division_by_255(_text_rgb_gamma.rgbGreen, src_alpha),
				division_by_255(_text_rgb_gamma.rgbRed, src_alpha),
				src_alpha};

			// this approach is faster than setting each byte separately
			*reinterpret_cast<DWORD *>(dest_bits + dest_px_ptr) = *reinterpret_cast<const DWORD *>(&src_color);
		}

		if (src_curr_row < src_rect.bottom - 1)
			dest_row_ptr += dest_pitch;
		else if (!project_bottom)
			break;
	}
}

void gdi_painter::set_lcd_text_bits(const FT_BitmapGlyph glyph,
	const RECT &src_rect,
	BYTE *dest_bits,
	const RECT &dest_rect,
	int dest_pitch,
	bool project_bottom,
	BYTE alpha) const
{
	// the source bitmap is 24bpp, in order of R, G, B channels
	// the destination bitmaps is 32bpp, in order of B, G, R, A channels
	// each row is aligned to DWORD

	const WORD src_byte_per_px = 3;
	const WORD dest_byte_per_px = 4;

	int src_row_ptr = src_rect.top * glyph->bitmap.pitch;
	int dest_row_ptr = dest_rect.top * dest_pitch;

	for (int src_curr_row = src_rect.top; src_curr_row < glyph->bitmap.rows; ++src_curr_row, src_row_ptr += glyph->bitmap.pitch)
	{
		// pointers to the current pixel
		// pointing to the beginning of a row
		int src_px_ptr = src_row_ptr + src_rect.left * src_byte_per_px;
		int dest_px_ptr = dest_row_ptr + dest_rect.left * dest_byte_per_px;

		for (int src_curr_column = src_rect.left; src_curr_column < src_rect.right; ++src_curr_column, src_px_ptr += src_byte_per_px, dest_px_ptr += dest_byte_per_px)
		{
			// color components of the source bitmap
			RGBQUAD src_color = _text_rgb_gamma;

			// alpha components of the source bitmap
			// apply pixel geometry
			RGBQUAD src_alpha = {};
			if (_context->setting_cache->render_mode.pixel_geometry == PIXEL_GEOMETRY_BGR)
			{
				src_alpha.rgbRed = glyph->bitmap.buffer[src_px_ptr+2];
				src_alpha.rgbGreen = glyph->bitmap.buffer[src_px_ptr+1];
				src_alpha.rgbBlue = glyph->bitmap.buffer[src_px_ptr];
			}
			else
			{
				src_alpha.rgbRed = glyph->bitmap.buffer[src_px_ptr];
				src_alpha.rgbGreen = glyph->bitmap.buffer[src_px_ptr+1];
				src_alpha.rgbBlue = glyph->bitmap.buffer[src_px_ptr+2];
			}

			// apply shadow alpha
			if (alpha != 255)
			{
				src_color.rgbRed = division_by_255(src_color.rgbRed, alpha);
				src_color.rgbGreen = division_by_255(src_color.rgbGreen, alpha);
				src_color.rgbBlue = division_by_255(src_color.rgbBlue, alpha);
				src_alpha.rgbRed = division_by_255(src_alpha.rgbRed, alpha);
				src_alpha.rgbGreen = division_by_255(src_alpha.rgbGreen, alpha);
				src_alpha.rgbBlue = division_by_255(src_alpha.rgbBlue, alpha);
			}

			// alpha blending
			RGBQUAD dest_color = {dest_bits[dest_px_ptr] + division_by_255(src_color.rgbBlue - dest_bits[dest_px_ptr], src_alpha.rgbBlue),
				dest_bits[dest_px_ptr+1] + division_by_255(src_color.rgbGreen - dest_bits[dest_px_ptr+1], src_alpha.rgbGreen),
				dest_bits[dest_px_ptr+2] + division_by_255(src_color.rgbRed - dest_bits[dest_px_ptr+2], src_alpha.rgbRed),
				0};

			/*
			GDI's behavior:
			if the destination pixel is modified, reset its alpha value to 0
			otherwise, leave the alpha value untouched
			*/
			if (*reinterpret_cast<DWORD *>(&src_alpha) == 0)
				dest_color.rgbReserved = dest_bits[dest_px_ptr+3];

			*reinterpret_cast<DWORD *>(dest_bits + dest_px_ptr) = *reinterpret_cast<DWORD *>(&dest_color);
		}

		if (src_curr_row < src_rect.bottom - 1)
			dest_row_ptr += dest_pitch;
		else if (!project_bottom)
			break;
	}
}

BOOL gdi_painter::paint_mono(UINT options, CONST RECT *lprect, const gdipp_rpc_bitmap_glyph_run *a_glyph_run, const glyph_run_metrics &grm) const
{
	BOOL b_ret, paint_success;

	// the data source is FreeType bitmap with black/white mask
	// create GDI mask bitmap and use MaskBlt to complete text painting

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

	const SIZE bbox_visible_size = {grm.visible_rect.right - grm.visible_rect.left, grm.visible_rect.bottom - grm.visible_rect.top};
	const BITMAPINFO bmp_info = {sizeof(BITMAPINFOHEADER), bbox_visible_size.cx, -bbox_visible_size.cy, 1, 1, BI_RGB};

	BYTE *mask_bits;
	const HBITMAP mask_bitmap = CreateDIBSection(_context->hdc, &bmp_info, DIB_RGB_COLORS, reinterpret_cast<VOID **>(&mask_bits), NULL, 0);
	assert(mask_bitmap != NULL);

	const int bk_mode = GetBkMode(_context->hdc);
	if (bk_mode == OPAQUE)
		paint_background(_context->hdc, &grm.visible_rect, _bg_color);

	std::list<FT_Glyph>::const_iterator glyph_iter;
	std::list<RECT>::const_iterator black_iter;
	for (glyph_iter = a_glyph_run->glyphs.begin(), black_iter = a_glyph_run->black_boxes.begin();
		glyph_iter != a_glyph_run->glyphs.end(); ++glyph_iter, ++black_iter)
	{
		FT_BitmapGlyph bmp_glyph = reinterpret_cast<FT_BitmapGlyph>(*glyph_iter);
		if (bmp_glyph == NULL)
			continue;

		assert(bmp_glyph->bitmap.pitch >= 0);

		// the bounding box of the current glyph in the DC bitmap
		RECT glyph_rect;
		glyph_rect.left = grm.baseline.x + black_iter->left;
		glyph_rect.top = grm.baseline.y - bmp_glyph->top;
		glyph_rect.right = grm.baseline.x + black_iter->right;
		glyph_rect.bottom = glyph_rect.top + bmp_glyph->bitmap.rows;

		// only paint the visible part of the source to the new bitmap
		RECT glyph_rect_in_bbox;
		const BOOL is_glyph_in_bbox = IntersectRect(&glyph_rect_in_bbox, &glyph_rect, &grm.visible_rect);
		if (is_glyph_in_bbox)
		{
			const RECT src_rect = {glyph_rect_in_bbox.left - glyph_rect.left,
				glyph_rect_in_bbox.top - glyph_rect.top,
				glyph_rect_in_bbox.right - glyph_rect.left,
				glyph_rect_in_bbox.bottom - glyph_rect.top};
			const RECT dest_rect = {glyph_rect_in_bbox.left - grm.visible_rect.left,
				glyph_rect_in_bbox.top - grm.visible_rect.top,
				glyph_rect_in_bbox.right - grm.visible_rect.left,
				glyph_rect_in_bbox.bottom - grm.visible_rect.top};
			const int dest_pitch = get_bmp_pitch(bbox_visible_size.cx, 1);

			set_mono_mask_bits(bmp_glyph, src_rect, mask_bits, dest_rect, dest_pitch, bbox_visible_size.cy == grm.extent.cy);
		}
	}

	// obviously shadow for monochrome bitmap is not possible

	HBRUSH text_brush = CreateSolidBrush(_text_color);
	assert(text_brush != NULL);
	const HBRUSH prev_brush = static_cast<const HBRUSH>(SelectObject(_context->hdc, text_brush));

	// foreground ROP: source brush
	// background ROP: destination color
	paint_success = MaskBlt(_context->hdc,
		grm.visible_rect.left,
		grm.visible_rect.top,
		bbox_visible_size.cx,
		bbox_visible_size.cy,
		_context->hdc,
		0,
		0,
		mask_bitmap,
		0,
		0,
		MAKEROP4(PATCOPY, 0x00AA0029));

	text_brush = static_cast<HBRUSH>(SelectObject(_context->hdc, prev_brush));
	b_ret = DeleteObject(text_brush);
	assert(b_ret);
	b_ret = DeleteObject(mask_bitmap);
	assert(b_ret);

	return paint_success;
}

BOOL gdi_painter::paint_gray(UINT options, CONST RECT *lprect, const gdipp_rpc_bitmap_glyph_run *a_glyph_run, const glyph_run_metrics &grm) const
{
	// the data source is FreeType bitmap with 256 gray levels
	// create GDI alpha bitmap and use AlphaBlend() to paint both solid and shadow text

	BOOL b_ret, paint_success;

	const SIZE bbox_visible_size = {grm.visible_rect.right - grm.visible_rect.left, grm.visible_rect.bottom - grm.visible_rect.top};

	const BITMAPINFO bmp_info = {sizeof(BITMAPINFOHEADER), bbox_visible_size.cx, -bbox_visible_size.cy, 1, 32, BI_RGB};
	const HBITMAP text_bitmap = CreateDIBSection(_context->hdc, &bmp_info, DIB_RGB_COLORS, reinterpret_cast<VOID **>(&_tls->text_bits), NULL, 0);
	assert(text_bitmap != NULL);
	const HBITMAP prev_bitmap = reinterpret_cast<const HBITMAP>(SelectObject(_tls->hdc_canvas, text_bitmap));
	b_ret = DeleteObject(prev_bitmap);
	assert(b_ret);

	const int bk_mode = GetBkMode(_context->hdc);
	if (bk_mode == OPAQUE)
		paint_background(_context->hdc, &grm.visible_rect, _bg_color);

	const int dest_pitch = get_bmp_pitch(bbox_visible_size.cx, 32);

	std::list<FT_Glyph>::const_iterator glyph_iter;
	std::list<RECT>::const_iterator black_iter;
	for (glyph_iter = a_glyph_run->glyphs.begin(), black_iter = a_glyph_run->black_boxes.begin();
		glyph_iter != a_glyph_run->glyphs.end(); ++glyph_iter, ++black_iter)
	{
		FT_BitmapGlyph bmp_glyph = reinterpret_cast<FT_BitmapGlyph>(*glyph_iter);
		if (bmp_glyph == NULL)
			continue;

		assert(bmp_glyph->bitmap.pitch >= 0);

		RECT glyph_rect;
		glyph_rect.left = grm.baseline.x + black_iter->left;
		glyph_rect.top = grm.baseline.y - bmp_glyph->top;
		glyph_rect.right = grm.baseline.x + black_iter->right;
		glyph_rect.bottom = glyph_rect.top + bmp_glyph->bitmap.rows;

		RECT glyph_rect_in_bbox;
		const BOOL is_glyph_in_bbox = IntersectRect(&glyph_rect_in_bbox, &glyph_rect, &grm.visible_rect);
		if (is_glyph_in_bbox)
		{
			const RECT src_rect = {glyph_rect_in_bbox.left - glyph_rect.left,
				glyph_rect_in_bbox.top - glyph_rect.top,
				glyph_rect_in_bbox.right - glyph_rect.left,
				glyph_rect_in_bbox.bottom - glyph_rect.top};
			const RECT dest_rect = {glyph_rect_in_bbox.left - grm.visible_rect.left,
				glyph_rect_in_bbox.top - grm.visible_rect.top,
				glyph_rect_in_bbox.right - grm.visible_rect.left,
				glyph_rect_in_bbox.bottom - grm.visible_rect.top};

			set_gray_text_bits(bmp_glyph, src_rect, _tls->text_bits, dest_rect, dest_pitch, bbox_visible_size.cy == grm.extent.cy);
		}
	}

	BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};

	// AlphaBlend converts the source bitmap pixel format to match destination bitmap pixel format
	if (_context->setting_cache->shadow.alpha != 0)
	{
		bf.SourceConstantAlpha = _context->setting_cache->shadow.alpha;
		b_ret = AlphaBlend(_context->hdc,
			grm.visible_rect.left + _context->setting_cache->shadow.offset_x,
			grm.visible_rect.top + _context->setting_cache->shadow.offset_y,
			bbox_visible_size.cx,
			bbox_visible_size.cy,
			_tls->hdc_canvas,
			0,
			0,
			bbox_visible_size.cx,
			bbox_visible_size.cy,
			bf);
		if (!b_ret)
			return false;
	}

	bf.SourceConstantAlpha = 255;
	paint_success = AlphaBlend(_context->hdc,
		grm.visible_rect.left,
		grm.visible_rect.top,
		bbox_visible_size.cx,
		bbox_visible_size.cy,
		_tls->hdc_canvas,
		0,
		0,
		bbox_visible_size.cx,
		bbox_visible_size.cy,
		bf);

	return paint_success;
}

BOOL gdi_painter::paint_lcd(UINT options, CONST RECT *lprect, const gdipp_rpc_bitmap_glyph_run *a_glyph_run, const glyph_run_metrics &grm) const
{
	// the data source is FreeType bitmap with 256 gray levels in R, G, B channels
	// create GDI bitmap and optionally copy from DC, then use BitBlt() to paint both solid and shadow text

	BOOL b_ret, paint_success = TRUE;

	const SIZE bbox_visible_size = {grm.visible_rect.right - grm.visible_rect.left, grm.visible_rect.bottom - grm.visible_rect.top};

	/*
	optimization for UPDATECP when background mode is TRANSPARENT
	for the first time UPDATECP is seen, prepare the background in the canvas for the entire line instead of just the text
	for the second and upward times UPDATECP is seen, directly use the background data in canvas, thus save the costly BitBlt
	canvas is stored in TLS, and data in it is inherited across painting sessions
	when background mode is OPAQUE, do NOT use this optimization
	*/

	LONG canvas_width = bbox_visible_size.cx;
	LONG canvas_offset = grm.visible_rect.left;
	bool new_bitmap = true;

	const int bk_mode = GetBkMode(_context->hdc);
	if (bk_mode == TRANSPARENT)
	{
		if (_update_cp)
		{
			canvas_width = _context->bmp_header.biWidth;
			canvas_offset = 0;
			new_bitmap = (_tls->last_hdc != _context->hdc);
		}
	}
	else if (bk_mode != OPAQUE)
	{
		// unknown background mode
		return FALSE;
	}

	if (new_bitmap)
	{
		const BITMAPINFO bmp_info = {sizeof(BITMAPINFOHEADER), canvas_width, -bbox_visible_size.cy, 1, 32, BI_RGB};
		const HBITMAP text_bitmap = CreateDIBSection(_context->hdc, &bmp_info, DIB_RGB_COLORS, reinterpret_cast<VOID **>(&_tls->text_bits), NULL, 0);
		assert(text_bitmap != NULL);
		const HBITMAP prev_bitmap = reinterpret_cast<const HBITMAP>(SelectObject(_tls->hdc_canvas, text_bitmap));
		b_ret = DeleteObject(prev_bitmap);
		assert(b_ret);
	}

	if (bk_mode == OPAQUE)
	{
		const RECT bk_rect = {0, 0, canvas_width, bbox_visible_size.cy};
		paint_success = paint_background(_tls->hdc_canvas, &bk_rect, _bg_color);
	}
	else if (new_bitmap)
	{
		assert(bk_mode == TRANSPARENT);

		// BitBlt overwrites rather than overlays, therefore retrieve the original DC bitmap if transparent
		// "If a rotation or shear transformation is in effect in the source device context, BitBlt returns an error"
		paint_success = BitBlt(_tls->hdc_canvas,
			0,
			0,
			canvas_width,
			bbox_visible_size.cy,
			_context->hdc,
			canvas_offset,
			grm.visible_rect.top,
			SRCCOPY);
	}

	if (paint_success)
	{
		const int dest_pitch = get_bmp_pitch(canvas_width, 32);

		std::list<FT_Glyph>::const_iterator glyph_iter;
		std::list<RECT>::const_iterator black_iter;
		for (glyph_iter = a_glyph_run->glyphs.begin(), black_iter = a_glyph_run->black_boxes.begin();
			glyph_iter != a_glyph_run->glyphs.end(); ++glyph_iter, ++black_iter)
		{
			FT_BitmapGlyph bmp_glyph = reinterpret_cast<FT_BitmapGlyph>(*glyph_iter);
			if (bmp_glyph == NULL)
				continue;

			assert(bmp_glyph->bitmap.pitch >= 0);

			// the rect of the current glyph in the source bitmap
			RECT solid_glyph_rect;
			solid_glyph_rect.left = grm.baseline.x + black_iter->left;
			solid_glyph_rect.top = grm.baseline.y - bmp_glyph->top;
			solid_glyph_rect.right = grm.baseline.x + black_iter->right;
			solid_glyph_rect.bottom = solid_glyph_rect.top + bmp_glyph->bitmap.rows;

			RECT solid_rect_in_bbox;
			if (IntersectRect(&solid_rect_in_bbox, &solid_glyph_rect, &grm.visible_rect))
			{
				if (_context->setting_cache->shadow.alpha > 0)
				{
					const RECT shadow_glyph_rect = {solid_glyph_rect.left + _context->setting_cache->shadow.offset_x,
						solid_glyph_rect.top + _context->setting_cache->shadow.offset_y,
						solid_glyph_rect.right + _context->setting_cache->shadow.offset_x,
						solid_glyph_rect.bottom + _context->setting_cache->shadow.offset_y};

					RECT shadow_rect_in_bbox;
					if (IntersectRect(&shadow_rect_in_bbox, &shadow_glyph_rect, &grm.visible_rect))
					{
						const RECT shadow_src_rect = {shadow_rect_in_bbox.left - shadow_glyph_rect.left,
							shadow_rect_in_bbox.top - shadow_glyph_rect.top,
							shadow_rect_in_bbox.right - shadow_glyph_rect.left,
							shadow_rect_in_bbox.bottom - shadow_glyph_rect.top};
						const RECT shadow_dest_rect = {shadow_rect_in_bbox.left - grm.visible_rect.left,
							shadow_rect_in_bbox.top - grm.visible_rect.top,
							shadow_rect_in_bbox.right - grm.visible_rect.left,
							shadow_rect_in_bbox.bottom - grm.visible_rect.top};

						set_lcd_text_bits(bmp_glyph, shadow_src_rect, _tls->text_bits, shadow_dest_rect, dest_pitch, false, _context->setting_cache->shadow.alpha);
					}
				}

				// the visible rect part of the current glyph in the source bitmap
				const RECT solid_src_rect = {solid_rect_in_bbox.left - solid_glyph_rect.left,
					solid_rect_in_bbox.top - solid_glyph_rect.top,
					solid_rect_in_bbox.right - solid_glyph_rect.left,
					solid_rect_in_bbox.bottom - solid_glyph_rect.top};

				// the corresponding rect in the destination bitmap
				const RECT solid_dest_rect = {solid_rect_in_bbox.left - canvas_offset,
					solid_rect_in_bbox.top - grm.visible_rect.top,
					solid_rect_in_bbox.right - canvas_offset,
					solid_rect_in_bbox.bottom - grm.visible_rect.top};

				set_lcd_text_bits(bmp_glyph, solid_src_rect, _tls->text_bits, solid_dest_rect, dest_pitch, bbox_visible_size.cy == grm.extent.cy, 255);
			}
		}

		paint_success = BitBlt(_context->hdc,
			grm.visible_rect.left,
			grm.visible_rect.top,
			bbox_visible_size.cx,
			bbox_visible_size.cy,
			_tls->hdc_canvas,
			grm.visible_rect.left - canvas_offset,
			0,
			SRCCOPY);
	}

	return paint_success;
}

BOOL gdi_painter::paint_glyph_run(UINT options, CONST RECT *lprect, const gdipp_rpc_bitmap_glyph_run *a_glyph_run)
{
	/*
	both ETO_OPAQUE and OPAQUE background mode need background filled
	for ETO_OPAQUE, direct FillRect to the physical DC
	for OPAQUE background mode, draw the background on canvas DC (it might be clipped eventually)
	*/
	if (options & ETO_OPAQUE)
		paint_background(_context->hdc, lprect, _bg_color);

	glyph_run_metrics grm;
	// actual bounding box occupied by the glyphs
	grm.extent.cx = get_glyph_run_width(a_glyph_run, false);

	// nothing to paint
	if (grm.extent.cx == 0)
		return FALSE;

	grm.extent.cy = _context->outline_metrics->otmTextMetrics.tmHeight;
	const LONG bbox_ascent = _context->outline_metrics->otmTextMetrics.tmAscent;
	const LONG bbox_descent = _context->outline_metrics->otmTextMetrics.tmDescent;

	// adjusted baseline where the bitmap will be finally drawn before applying clipping
	grm.baseline = get_baseline(_text_alignment,
		_cursor.x,
		_cursor.y,
		grm.extent.cx,
		bbox_ascent,
		bbox_descent);

	grm.visible_rect.left = grm.baseline.x + a_glyph_run->black_boxes.front().left;
	grm.visible_rect.top = grm.baseline.y - bbox_ascent;
	grm.visible_rect.right = grm.visible_rect.left + grm.extent.cx;
	grm.visible_rect.bottom = grm.visible_rect.top + grm.extent.cy;

	// advance cursor by the width of the control box of the glyph run
	_cursor.x += a_glyph_run->ctrl_boxes.back().right - a_glyph_run->ctrl_boxes.front().left;

	// apply clipping
	if (options & ETO_CLIPPED && !IntersectRect(&grm.visible_rect, &grm.visible_rect, lprect))
		return FALSE;

	switch (_render_mode)
	{
		case FT_RENDER_MODE_NORMAL:
		case FT_RENDER_MODE_LIGHT:
			return paint_gray(options, lprect, a_glyph_run, grm);
		case FT_RENDER_MODE_MONO:
			return paint_mono(options, lprect, a_glyph_run, grm);
		case FT_RENDER_MODE_LCD:
			return paint_lcd(options, lprect, a_glyph_run, grm);
		default:
			return FALSE;
	}
}

}
