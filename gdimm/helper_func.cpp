#include "stdafx.h"
#include "freetype.h"
#include "helper_func.h"
#include "gdimm.h"
#include "lock.h"

FT_F26Dot6 to_26dot6(const FIXED &x)
{
	return *(reinterpret_cast<const FT_F26Dot6 *>(&x)) >> 10;
}

FT_F26Dot6 to_26dot6(FLOAT x)
{
	return static_cast<FT_F26Dot6>(x * 64);
}

LONG from_26dot6(FT_Pos x)
{
	return x >> 6;
}

LONG from_16dot16(FT_Pos x)
{
	return x >> 16;
}

// convert floating point to 16.16 format
FT_Pos to_16dot16(double x)
{
	return static_cast<FT_Pos>(x * 65536);
}

DWORD create_tls_index()
{
	DWORD new_tls_index = TlsAlloc();
	assert(new_tls_index != TLS_OUT_OF_INDEXES);

	return new_tls_index;
}

BOOL free_tls_index(DWORD tls_index)
{
	return TlsFree(tls_index);
}

BYTE division_by_255(short number, short numerator)
{
	// there are many approaches to approximate number * numerator / 255
	// it is a trade-off between efficiency and accuracy

	const int t = number * numerator;
	return (((t + 255) >> 8) + t) >> 8;
}

uint64_t generate_font_trait(const LOGFONTW &log_font, FT_Render_Mode render_mode)
{
	// the LOGFONTW structure and render mode are the minimal set that uniquely determine font metrics used by any renderer

	// exclude the bytes after the face name, which may contain junk data
	const int lf_metric_size = sizeof(log_font) - sizeof(log_font.lfFaceName);
	const int lf_facename_size = static_cast<const int>((wcslen(log_font.lfFaceName) * sizeof(wchar_t)));
	const int lf_total_size = lf_metric_size + lf_facename_size;

#ifdef _M_X64
	return MurmurHash64A(&log_font, lf_total_size, render_mode);
#else
	return MurmurHash64B(&log_font, lf_total_size, render_mode);
#endif // _M_X64
}

POINT get_baseline(UINT alignment, int x, int y, int width, int ascent, int descent)
{
	POINT baseline = {x, y};

	switch ((TA_LEFT | TA_RIGHT | TA_CENTER) & alignment)
	{
	case TA_LEFT:
		break;
	case TA_RIGHT:
		baseline.x -= width;
		break;
	case TA_CENTER:
		baseline.x -= width / 2;
		break;
	}

	switch ((TA_TOP | TA_BOTTOM | TA_BASELINE) & alignment)
	{
	case TA_TOP:
		baseline.y += ascent;
		break;
	case TA_BOTTOM:
		baseline.y -= descent;
		break;
	case TA_BASELINE:
		break;
	}

	return baseline;
}

int get_bmp_pitch(int width, WORD bpp)
{
#define FT_PAD_FLOOR( x, n )  ( (x) & ~((n)-1) )
#define FT_PAD_ROUND( x, n )  FT_PAD_FLOOR( (x) + ((n)/2), n )
#define FT_PAD_CEIL( x, n )   FT_PAD_FLOOR( (x) + ((n)-1), n )

	return FT_PAD_CEIL(static_cast<int>(ceil(static_cast<double>(width * bpp) / 8)), sizeof(LONG));
}

bool get_dc_bmp_header(HDC hdc, BITMAPINFOHEADER &dc_bmp_header)
{
	dc_bmp_header.biSize = sizeof(BITMAPINFOHEADER);

	const HBITMAP dc_bitmap = static_cast<const HBITMAP>(GetCurrentObject(hdc, OBJ_BITMAP));
	if (dc_bitmap == NULL)
	{
		// currently no selected bitmap
		// use DC capability

		dc_bmp_header.biWidth = GetDeviceCaps(hdc, HORZRES);
		dc_bmp_header.biHeight = GetDeviceCaps(hdc, VERTRES);
		dc_bmp_header.biPlanes = GetDeviceCaps(hdc, PLANES);
		dc_bmp_header.biBitCount = GetDeviceCaps(hdc, BITSPIXEL);

		return false;
	}
	else
	{
		dc_bmp_header.biBitCount = 0;
		const int i_ret = GetDIBits(hdc, dc_bitmap, 0, 0, NULL, reinterpret_cast<LPBITMAPINFO>(&dc_bmp_header), DIB_RGB_COLORS);
		assert(i_ret != 0);

		return true;
	}
}

OUTLINETEXTMETRICW *get_dc_metrics(HDC hdc, vector<BYTE> &metric_buf)
{
	// get outline metrics of the DC, which also include the text metrics

	UINT metric_size = GetOutlineTextMetricsW(hdc, 0, NULL);
	if (metric_size == 0)
		return NULL;

	metric_buf.resize(metric_size);
	OUTLINETEXTMETRICW *outline_metrics = reinterpret_cast<OUTLINETEXTMETRICW *>(&metric_buf[0]);
	metric_size = GetOutlineTextMetricsW(hdc, metric_size, outline_metrics);
	assert(metric_size != 0);

	return outline_metrics;
}

char get_gdi_weight_class(unsigned short weight)
{
	/*
	emulate GDI behavior:
	weight 1 to 550 are rendered as Regular
	551 to 611 are Semibold
	612 to infinity are Bold

	weight 0 is DONTCARE
	*/

	const LONG weight_class_max[] = {0, 550, 611};
	const char max_weight_class = sizeof(weight_class_max) / sizeof(LONG);

	for (char i = 0; i < max_weight_class; i++)
	{
		if (weight <= weight_class_max[i])
			return i;
	}

	return max_weight_class;
}

int get_glyph_bmp_width(const FT_Bitmap &bitmap)
{
	if (bitmap.pixel_mode == FT_PIXEL_MODE_LCD)
		return bitmap.width / 3;
	else
		return bitmap.width;
}

LONG get_glyph_run_width(const glyph_run *a_glyph_run, bool is_control_width)
{
	assert(a_glyph_run != NULL);
 
	list<RECT>::const_iterator first_box_iter;
	list<RECT>::const_reverse_iterator last_box_iter;

	if (is_control_width)
	{
		// use control box metrics
		first_box_iter = a_glyph_run->ctrl_boxes.begin();
		last_box_iter = a_glyph_run->ctrl_boxes.rbegin();
	}
	else
	{
		// use black box metrics
		first_box_iter = a_glyph_run->black_boxes.begin();
		last_box_iter = a_glyph_run->black_boxes.rbegin();
	}

	if (a_glyph_run->ctrl_boxes.back().left >= a_glyph_run->ctrl_boxes.front().left)
		return last_box_iter->right - first_box_iter->left;
	else
		return first_box_iter->right - last_box_iter->left;
}

LOGFONTW get_log_font(HDC hdc)
{
	HFONT h_font = reinterpret_cast<HFONT>(GetCurrentObject(hdc, OBJ_FONT));
	assert(h_font != NULL);

	LOGFONTW font_attr;
	GetObject(h_font, sizeof(LOGFONTW), &font_attr);

	return font_attr;
}

bool operator<(const LOGFONTW &lf1, const LOGFONTW &lf2)
{
	return memcmp(&lf1, &lf2, sizeof(LOGFONTW)) < 0;
}

bool get_render_mode(const font_setting_cache *font_setting, WORD dc_bmp_bpp, BYTE font_quality, FT_Render_Mode &render_mode)
{
	// return true if successfully find an appropriate render mode
	// otherwise return false

	if (font_setting->render_mode.mono == 2)
	{
		render_mode = FT_RENDER_MODE_MONO;
		return true;
	}

	if (font_setting->render_mode.gray == 2)
	{
		render_mode = FT_RENDER_MODE_NORMAL;
		return true;
	}

	if (font_setting->render_mode.subpixel == 2)
	{
		render_mode = FT_RENDER_MODE_LCD;
		return true;
	}

	if (!font_setting->render_mode.aliased_text && font_quality == NONANTIALIASED_QUALITY)
		return false;

	if (font_setting->render_mode.mono == 1 && dc_bmp_bpp == 1)
	{
		render_mode = FT_RENDER_MODE_MONO;
		return true;
	}

	if (font_setting->render_mode.gray == 1 && dc_bmp_bpp == 8)
	{
		render_mode = FT_RENDER_MODE_NORMAL;
		return true;
	}

	// we do not support 16 bpp currently

	if (font_setting->render_mode.subpixel == 1 && dc_bmp_bpp >= 24)
	{
		render_mode = FT_RENDER_MODE_LCD;
		return true;
	}

	return false;
}

const FT_Glyph make_empty_outline_glyph()
{
	FT_Glyph empty_glyph;

	FT_Error ft_error;

	FT_GlyphSlotRec glyph_slot = {};
	glyph_slot.library = ft_lib;
	glyph_slot.format = FT_GLYPH_FORMAT_OUTLINE;

	ft_error = FT_Get_Glyph(&glyph_slot, &empty_glyph);
	if (ft_error != 0)
		return NULL;

	return empty_glyph;
}

bool mb_to_wc(const char *multi_byte_str, int count, wstring &wide_char_str)
{
	int wc_str_len = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, multi_byte_str, count, NULL, 0);
	if (wc_str_len == 0)
		return false;

	wide_char_str.resize(wc_str_len);
	wc_str_len = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, multi_byte_str, count, &wide_char_str[0], wc_str_len);
	if (wc_str_len == 0)
		return false;

	return true;
}

BOOL paint_background(HDC hdc, const RECT *bg_rect, COLORREF bg_color)
{
	int i_ret;

	if (bg_color == CLR_INVALID)
		return FALSE;

	const HBRUSH bg_brush = CreateSolidBrush(bg_color);
	if (bg_brush == NULL)
		return FALSE;

	i_ret = FillRect(hdc, bg_rect, bg_brush);
	if (i_ret == 0)
		return FALSE;

	DeleteObject(bg_brush);
	return TRUE;
}

COLORREF parse_palette_color(HDC hdc, COLORREF color)
{
	// if input color is CLR_INVALID, return it unchanged.
	if (color == CLR_INVALID)
		return CLR_INVALID;

	COLORREF color_ret = color;

	// if the high-order byte is odd, use the selected palette whose index is specified in the low-order bytes
	// see PALETTEINDEX()
	if ((color_ret & 0x01000000) != 0)
	{
		const HPALETTE dc_palette = static_cast<const HPALETTE>(GetCurrentObject(hdc, OBJ_PAL));

		PALETTEENTRY pal_entry;
		const UINT entries = GetPaletteEntries(dc_palette, (color_ret & 0x00ffffff), 1, &pal_entry);

		// if the DC has no palette entry, this is an invalid color
		if (entries != 0)
			color_ret = RGB(pal_entry.peRed, pal_entry.peGreen, pal_entry.peBlue);
	}

	return color_ret;
}

const wchar_t *metric_family_name(const BYTE *metric_buf)
{
	return reinterpret_cast<const wchar_t *>(metric_buf + reinterpret_cast<const UINT>((reinterpret_cast<const OUTLINETEXTMETRICW *>(metric_buf)->otmpFamilyName)));
}

const wchar_t *metric_face_name(const BYTE *metric_buf)
{
	return reinterpret_cast<const wchar_t *>(metric_buf + reinterpret_cast<const UINT>((reinterpret_cast<const OUTLINETEXTMETRICW *>(metric_buf)->otmpFaceName)));
}

const wchar_t *metric_style_name(const BYTE *metric_buf)
{
	return reinterpret_cast<const wchar_t *>(metric_buf + reinterpret_cast<const UINT>((reinterpret_cast<const OUTLINETEXTMETRICW *>(metric_buf)->otmpStyleName)));
}

const wchar_t *metric_family_name(const OUTLINETEXTMETRICW *outline_metric)
{
	return reinterpret_cast<const wchar_t *>(reinterpret_cast<const BYTE *>(outline_metric) + reinterpret_cast<const UINT>(outline_metric->otmpFamilyName));
}

const wchar_t *metric_face_name(const OUTLINETEXTMETRICW *outline_metric)
{
	return reinterpret_cast<const wchar_t *>(reinterpret_cast<const BYTE *>(outline_metric) + reinterpret_cast<const UINT>(outline_metric->otmpFaceName));
}

const wchar_t *metric_style_name(const OUTLINETEXTMETRICW *outline_metric)
{
	return reinterpret_cast<const wchar_t *>(reinterpret_cast<const BYTE *>(outline_metric) + reinterpret_cast<const UINT>(outline_metric->otmpStyleName));
}