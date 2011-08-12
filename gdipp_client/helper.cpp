#include "stdafx.h"
#include "helper.h"
#include "gdipp_client/gdimm.h"

namespace gdipp
{

bool dc_context::init(HDC hdc)
{
	outline_metrics = get_dc_metrics(hdc, _metric_buf);
	if (outline_metrics == NULL)
		return false;

	if (!get_dc_bmp_header(hdc, bmp_header))
		return false;

	this->hdc = hdc;

	log_font = get_log_font(hdc);
	log_font.lfWeight = get_gdi_weight_class(static_cast<unsigned short>(log_font.lfWeight));

	return true;
}

COLORREF parse_palette_color(HDC hdc, COLORREF color)
{
	// if input color is CLR_INVALID, return it unchanged.
	if (color == CLR_INVALID)
		return CLR_INVALID;

	COLORREF color_ret = color;

	// if the high-order byte is odd, use the selected palette whose index is specified in the low-order bytes
	// see PALETTEINDEX()
	if (!!(color_ret & 0x01000000))
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

}
