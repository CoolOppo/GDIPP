#include "stdafx.h"
#include "text.h"
#include "ft_renderer.h"
#include "ggo_renderer.h"

BOOL gdimm_text::draw_background(HDC hdc, const RECT *bg_rect, COLORREF bg_color)
{
	assert(bg_color != CLR_INVALID);

	BOOL b_ret;
	int i_ret;

	const HBRUSH bg_brush = CreateSolidBrush(bg_color);
	if (bg_brush == NULL)
		return FALSE;

	i_ret = FillRect(hdc, bg_rect, bg_brush);
	if (i_ret == 0)
		return FALSE;

	b_ret = DeleteObject(bg_brush);
	return b_ret;
}

// for given bitmap width and bit count, compute the bitmap pitch
int get_bmp_pitch(int width, WORD bpp)
{
#define FT_PAD_FLOOR( x, n )  ( (x) & ~((n)-1) )
#define FT_PAD_ROUND( x, n )  FT_PAD_FLOOR( (x) + ((n)/2), n )
#define FT_PAD_CEIL( x, n )   FT_PAD_FLOOR( (x) + ((n)-1), n )

	return FT_PAD_CEIL((int) ceil((double)(width * bpp) / 8), sizeof(LONG));
}

BITMAPINFO gdimm_text::get_dc_bmp_info(HDC hdc)
{
	int i_ret;

	const HBITMAP dc_bitmap = (HBITMAP) GetCurrentObject(hdc, OBJ_BITMAP);
	assert(dc_bitmap != NULL);

	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	i_ret = GetDIBits(hdc, dc_bitmap, 0, 0, NULL, &bmi, DIB_RGB_COLORS);
	assert(i_ret != 0);
	assert(bmi.bmiHeader.biBitCount != 0);

	return bmi;
}

int gdimm_text::get_ft_bmp_width(const FT_Bitmap &bitmap)
{
	if (bitmap.pixel_mode == FT_PIXEL_MODE_LCD)
		return bitmap.width / 3;
	else
		return bitmap.width;
}

RECT gdimm_text::get_glyph_bmp_rect(const vector<const FT_BitmapGlyph> &glyphs, const vector<POINT> &glyph_pos, POINT cursor)
{
	RECT bmp_rect = {LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN};

	const size_t last_pos = glyph_pos.size() - 1;

	// condition checks because glyph might be place right-to-left

	if (glyph_pos[last_pos].x >= glyph_pos[0].x)
	{
		// left to right
		bmp_rect.left = glyph_pos[0].x;
		bmp_rect.right = max(glyph_pos[last_pos].x + get_ft_bmp_width(glyphs[last_pos]->bitmap), cursor.x);
	}
	else
	{
		// right to left
		bmp_rect.left = min(glyph_pos[last_pos].x, cursor.x);
		bmp_rect.right = glyph_pos[0].x + get_ft_bmp_width(glyphs[0]->bitmap);
	}

	/*
	// vertical text is not supported yet

	if (glyph_pos[last_pos].y >= glyph_pos[0].y)
	{
		bmp_rect.top = glyph_pos[0].y;
		bmp_rect.bottom = glyph_pos[last_pos].y + glyphs[last_pos]->bitmap.rows;
	}
	else
	{
		bmp_rect.top = glyph_pos[last_pos].y;
		bmp_rect.bottom = glyph_pos[0].y + glyphs[0]->bitmap.rows;
	}
	*/

	return bmp_rect;
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

void gdimm_text::get_gamma_ramps(const wchar_t *font_name, bool is_lcd)
{
	if (is_lcd)
	{
		_gamma_ramps[1] = gamma_instance.get_ramp(_setting_cache->gamma.red);
		_gamma_ramps[2] = gamma_instance.get_ramp(_setting_cache->gamma.green);
		_gamma_ramps[3] = gamma_instance.get_ramp(_setting_cache->gamma.blue);
	}
	else
		_gamma_ramps[0] = gamma_instance.get_ramp(_setting_cache->gamma.gray);
}

// for given DC bitmap bit count, return the corresponding FT_Glyph_To_Bitmap render mode
bool gdimm_text::get_render_mode(const wchar_t *font_name, FT_Render_Mode &render_mode) const
{
	// non-antialiased font
	// draw with monochrome mode
	if (_font_attr.lfQuality == NONANTIALIASED_QUALITY && !_setting_cache->render_non_aa)
	{
		render_mode = FT_RENDER_MODE_MONO;
		return true;
	}

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
		render_mode = (_setting_cache->subpixel_render ? FT_RENDER_MODE_LCD : FT_RENDER_MODE_NORMAL);
		break;
	default:
		// we do not support 16 bpp currently
		return false;
	}

	return true;
}

void gdimm_text::set_bmp_bits_mono(const FT_Bitmap &src_bitmap,
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
	const bool use_zero_color = (*(DWORD*) &_fg_rgb == 0);

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
					memcpy(dest_bits + dest_ptr, &_fg_rgb, _bmp_info.bmiHeader.biBitCount / 8);
			}
		}
	}
}

void gdimm_text::set_bmp_bits_gray(const FT_Bitmap &src_bitmap,
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

			//if (_bmp_info.bmiHeader.biBitCount == 32)
			//	dest_bits[dest_ptr+3] = 0;
		}
	}
}

void gdimm_text::set_bmp_bits_lcd(const FT_Bitmap &src_bitmap,
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
			
			const BYTE dest_r = (src_r * _fg_rgb.rgbRed + (255 - src_r) * dest_bits[dest_ptr+2]) / 255;
			const BYTE dest_g = (src_g * _fg_rgb.rgbGreen + (255 - src_g) * dest_bits[dest_ptr+1]) / 255;
			const BYTE dest_b = (src_b * _fg_rgb.rgbBlue + (255 - src_b) * dest_bits[dest_ptr]) / 255;

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
bool gdimm_text::draw_glyphs(const vector<const FT_BitmapGlyph> &glyphs,
	const vector<POINT> &glyph_pos,
	UINT options,
	CONST RECT *lprect) const
{
	BOOL b_ret;

	// 1.

	// origin of the source glyphs
	const POINT src_origin = glyph_pos[0];

	const RECT bmp_rect = get_glyph_bmp_rect(glyphs, glyph_pos, _cursor);
	const LONG bmp_width = bmp_rect.right - bmp_rect.left;

	// respect the height and ascent returned from GDI
	const int cell_height = _outline_metrics->otmTextMetrics.tmHeight;
	const int cell_ascent = _outline_metrics->otmTextMetrics.tmAscent;

	const SIZE dest_size = {bmp_width, cell_height};

	// position where the bitmap will be finally drawn if no clipping is needed
	POINT dest_origin = src_origin;

	switch ((TA_LEFT | TA_RIGHT | TA_CENTER) & _text_alignment)
	{
	case TA_LEFT:
		break;
	case TA_RIGHT:
		dest_origin.x -= dest_size.cx;
		break;
	case TA_CENTER:
		dest_origin.x -= dest_size.cx / 2;
		break;
	}
	switch ((TA_TOP | TA_BOTTOM | TA_BASELINE) & _text_alignment)
	{
	case TA_TOP:
		break;
	case TA_BOTTOM:
		dest_origin.y -= dest_size.cy;
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

	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = dest_size.cx;
	bmi.bmiHeader.biHeight = dest_size.cy;
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
		const RECT bk_rect = {0, 0, dest_size.cx, dest_size.cy};
		b_ret = draw_background(hdc_canvas, &bk_rect, bg_color);
	}
	else if (bk_mode == TRANSPARENT)
	{
		// "If a rotation or shear transformation is in effect in the source device context, BitBlt returns an error"
		b_ret = BitBlt(hdc_canvas, 0, 0, dest_size.cx, dest_size.cy, _hdc_text, dest_origin.x, dest_origin.y, SRCCOPY | NOMIRRORBITMAP);
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

	for (size_t i = 0; i < glyphs.size(); i++)
	{
		/*
		Windows DIB and FreeType Bitmap have different ways to indicate bitmap direction
		biHeight > 0 means the Windows DIB is bottom-up
		biHeight < 0 means the Windows DIB is top-down
		pitch > 0 means the FreeType bitmap is down flow
		pitch > 0 means the FreeType bitmap is up flow
		*/

		const POINT glyph_src_pos = {0, max(glyphs[i]->top - cell_ascent, 0)};
		const POINT glyph_dest_pos = {glyph_pos[i].x - bmp_rect.left, max(cell_ascent - glyphs[i]->top, 0)};

		const POINT shadow_src_pos = {max(glyph_src_pos.x - _setting_cache->shadow.offset_x, 0), max(glyph_src_pos.y - _setting_cache->shadow.offset_y, 0)};
		const POINT shadow_dest_pos = {max(glyph_dest_pos.x + _setting_cache->shadow.offset_x, 0), max(glyph_dest_pos.y + _setting_cache->shadow.offset_y, 0)};

		switch (glyphs[i]->bitmap.pixel_mode)
		{
		case FT_PIXEL_MODE_MONO:
			set_bmp_bits_mono(glyphs[i]->bitmap,
				glyph_src_pos,
				dest_bits,
				glyph_dest_pos,
				dest_size);
			break;
		case FT_PIXEL_MODE_GRAY:
			if (_setting_cache->shadow.alpha > 0)
				set_bmp_bits_gray(glyphs[i]->bitmap,
					shadow_src_pos,
					dest_bits,
					shadow_dest_pos,
					dest_size,
					_setting_cache->shadow.alpha);
			set_bmp_bits_gray(glyphs[i]->bitmap,
				glyph_src_pos,
				dest_bits,
				glyph_dest_pos,
				dest_size,
				255);
			break;
		case FT_PIXEL_MODE_LCD:
			if (_setting_cache->shadow.alpha > 0)
				set_bmp_bits_lcd(glyphs[i]->bitmap,
					shadow_src_pos,
					dest_bits,
					shadow_dest_pos,
					dest_size,
					_setting_cache->shadow.alpha);
			set_bmp_bits_lcd(glyphs[i]->bitmap,
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
		RECT dest_rect = {dest_origin.x, dest_origin.y, dest_origin.x + dest_size.cx, dest_origin.y + dest_size.cy};
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
			dest_size.cx,
			dest_size.cy,
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

bool gdimm_text::init(HDC hdc)
{
	_hdc_text = hdc;

	if (!get_dc_metrics())
		return false;

	// ignore rotated DC
	if (_font_attr.lfEscapement % 3600 != 0)
		return false;

	_font_face = metric_face_name(_outline_metrics);
	_setting_cache = setting_cache_instance.lookup(_font_face);

	if (_setting_cache->max_height != 0 && _setting_cache->max_height < _outline_metrics->otmTextMetrics.tmHeight)
		return false;

	return true;
}

template <typename RENDERER>
bool gdimm_text::render_text(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
	BOOL b_ret;

	_bmp_info = get_dc_bmp_info(_hdc_text);

	FT_Render_Mode render_mode;
	if (!get_render_mode(_font_face, render_mode))
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

	RENDERER font_renderer(this);
	bool render_success = font_renderer.render(options, lprect, lpString, c, lpDx, render_mode);
	if (!render_success)
		return false;

	bool draw_success = false;
	const vector<const FT_BitmapGlyph> &glyphs = font_renderer.get_glyphs();

	if (!glyphs.empty())
	{
		// get foreground color
		COLORREF fg_color = GetTextColor(_hdc_text);
		assert(fg_color != CLR_INVALID);
		_fg_rgb.rgbBlue = GetBValue(fg_color);
		_fg_rgb.rgbGreen = GetGValue(fg_color);
		_fg_rgb.rgbRed = GetRValue(fg_color);

		get_gamma_ramps(_font_face, ((render_mode & FT_RENDER_MODE_LCD) != 0));

		draw_success = draw_glyphs(glyphs, font_renderer.get_glyph_pos(), options, lprect);
	}

	// if TA_UPDATECP is set, update current position after text out
	if (update_cursor && draw_success)
	{
		b_ret = MoveToEx(_hdc_text, _cursor.x, _cursor.y, NULL);
		assert(b_ret);
	}

	return draw_success;
}

bool gdimm_text::text_out(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
	switch (_setting_cache->renderer)
	{
	case GETGLYPHOUTLINE:
		return render_text<ggo_renderer>(x, y, options, lprect, lpString, c, lpDx);
	case FREETYPE:
		return render_text<ft_renderer>(x, y, options, lprect, lpString, c, lpDx);
	default:
		return false;
	}
}