#include "stdafx.h"
#include "os2_metrics.h"
#include "text_helper.h"

#define OS2_TABLE_TAG mmioFOURCC('O', 'S', '/', '2')

// little-endian <-> big-endian
#define SWAPWORD(x) MAKEWORD(HIBYTE(x), LOBYTE(x))
#define SWAPLONG(x) MAKELONG(SWAPWORD(HIWORD(x)), SWAPWORD(LOWORD(x)))

bool gdimm_os2_metrics::init(HDC hdc)
{
	// retrieve needed fields from the OS/2 table

	DWORD font_data_size;

	font_data_size = GetFontData(hdc, OS2_TABLE_TAG, offsetof(TT_OS2, xAvgCharWidth), &_xAvgCharWidth, sizeof(_xAvgCharWidth));
	if (font_data_size == GDI_ERROR)
		return false;

	font_data_size = GetFontData(hdc, OS2_TABLE_TAG, offsetof(TT_OS2, usWeightClass), &_usWeightClass, sizeof(_usWeightClass));
	if (font_data_size == GDI_ERROR)
		return false;

	font_data_size = GetFontData(hdc, OS2_TABLE_TAG, offsetof(TT_OS2, fsSelection), &_fsSelection, sizeof(_fsSelection));
	if (font_data_size == GDI_ERROR)
		return false;

	_xAvgCharWidth = SWAPWORD(_xAvgCharWidth);
	_usWeightClass = SWAPWORD(_usWeightClass);
	_fsSelection = SWAPWORD(_fsSelection);

	return true;
}

FT_Short gdimm_os2_metrics::get_xAvgCharWidth()
{
	return _xAvgCharWidth;
}

unsigned char gdimm_os2_metrics::get_weight_class()
{
	return get_gdi_weight_class(_usWeightClass);
}

bool gdimm_os2_metrics::is_italic()
{
	return (_fsSelection & 1);
}