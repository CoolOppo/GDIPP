#include "stdafx.h"
#include "os2_metrics.h"
#include "gdipp_lib/helper.h"
#include "gdipp_rpc/gdipp_rpc.h"
#include "gdipp_server/global.h"

namespace gdipp
{

#define OS2_TABLE_TAG mmioFOURCC('O', 'S', '/', '2')

// little-endian <-> big-endian
#define SWAPWORD(x) MAKEWORD(HIBYTE(x), LOBYTE(x))
#define SWAPLONG(x) MAKELONG(SWAPWORD(HIWORD(x)), SWAPWORD(LOWORD(x)))

bool os2_metrics::init(HDC hdc)
{
	// retrieve needed fields from the OS/2 table

	DWORD font_data_size;

	font_data_size = GetFontData(hdc, OS2_TABLE_TAG, offsetof(TT_OS2, xAvgCharWidth), &_xAvgCharWidth, sizeof(_xAvgCharWidth));
	if (font_data_size == GDI_ERROR)
		return false;

	font_data_size = GetFontData(hdc, OS2_TABLE_TAG, offsetof(TT_OS2, usWeightClass), &_usWeightClass, sizeof(_usWeightClass));
	if (font_data_size == GDI_ERROR)
		return false;

	font_data_size = GetFontData(hdc, OS2_TABLE_TAG, offsetof(TT_OS2, usWidthClass), &_usWidthClass, sizeof(_usWidthClass));
	if (font_data_size == GDI_ERROR)
		return false;

	font_data_size = GetFontData(hdc, OS2_TABLE_TAG, offsetof(TT_OS2, fsSelection), &_fsSelection, sizeof(_fsSelection));
	if (font_data_size == GDI_ERROR)
		return false;

	_xAvgCharWidth = SWAPWORD(_xAvgCharWidth);
	_usWeightClass = SWAPWORD(_usWeightClass);
	_usWidthClass = SWAPWORD(_usWidthClass);
	_fsSelection = SWAPWORD(_fsSelection);

	return true;
}

bool os2_metrics::init(void *font_id)
{
	// retrieve needed fields from the OS/2 table

	DWORD font_data_size;

	HDC font_holder = dc_pool_instance.claim();

	font_data_size = font_mgr_instance.lookup_font_data(font_id, OS2_TABLE_TAG, offsetof(TT_OS2, xAvgCharWidth), reinterpret_cast<byte *>(&_xAvgCharWidth), sizeof(_xAvgCharWidth), font_holder);
	if (font_data_size == GDI_ERROR)
		goto failed_init_os2_metrics;

	font_data_size = font_mgr_instance.lookup_font_data(font_id, OS2_TABLE_TAG, offsetof(TT_OS2, usWeightClass), reinterpret_cast<byte *>(&_usWeightClass), sizeof(_usWeightClass), font_holder);
	if (font_data_size == GDI_ERROR)
		goto failed_init_os2_metrics;

	font_data_size = font_mgr_instance.lookup_font_data(font_id, OS2_TABLE_TAG, offsetof(TT_OS2, usWidthClass), reinterpret_cast<byte *>(&_usWidthClass), sizeof(_usWidthClass), font_holder);
	if (font_data_size == GDI_ERROR)
		goto failed_init_os2_metrics;

	font_data_size = font_mgr_instance.lookup_font_data(font_id, OS2_TABLE_TAG, offsetof(TT_OS2, fsSelection), reinterpret_cast<byte *>(&_fsSelection), sizeof(_fsSelection), font_holder);
	if (font_data_size == GDI_ERROR)
		goto failed_init_os2_metrics;

	_xAvgCharWidth = SWAPWORD(_xAvgCharWidth);
	_usWeightClass = SWAPWORD(_usWeightClass);
	_usWidthClass = SWAPWORD(_usWidthClass);
	_fsSelection = SWAPWORD(_fsSelection);

	return true;

failed_init_os2_metrics:
	dc_pool_instance.free(font_holder);
	return false;
}

FT_Short os2_metrics::get_xAvgCharWidth() const
{
	return _xAvgCharWidth;
}

char os2_metrics::get_weight_class() const
{
	return get_gdi_weight_class(_usWeightClass);
}

FT_UShort os2_metrics::get_usWidthClass() const
{
	return _usWidthClass;
}

bool os2_metrics::is_italic() const
{
	return (_fsSelection & 1);
}

}
