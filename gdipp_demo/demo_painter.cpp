#include "stdafx.h"
#include "demo_painter.h"
#include "gdipp_demo.h"
#include <gdipp_lib.h>

gdipp_demo_painter::gdipp_demo_painter()
	: _total_count(total_count),
	_painted_count(0),
	_result_prepared(false),
	_result_font(NULL)
{
	srand(GetTickCount());
}

gdipp_demo_painter::~gdipp_demo_painter()
{
	if (_result_font != NULL)
		DeleteObject(_result_font);
}

void gdipp_demo_painter::paint_demo(CPaintDC &dc)
{
	BOOL b_ret;

	if (_painted_count == 0)
		_start_time = GetTickCount();

	if (_painted_count < _total_count)
	{
		// randomize text metrics
		const LONG text_height = (rand() % 10) + 9;
		const LONG text_weight = (rand() % 8 + 1) * 100;
		const BYTE text_italic = rand() % 2;
		const wstring &font_name = paint_fonts[rand() % paint_fonts.size()];

		const HFONT curr_dc_font = CreateFont(-text_height, 0, 0, 0, text_weight, text_italic, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, font_name.c_str());
		assert(curr_dc_font != NULL);
		dc.SelectFont(curr_dc_font);

		// randomize text color
		dc.SetTextColor(RGB(rand() % 256, rand() % 256, rand() % 256));

		// randomize text background
		const int bk_mode = rand() % 2 + 1;
		dc.SetBkMode(bk_mode);
		if (bk_mode == OPAQUE)
			dc.SetBkColor(RGB(rand() % 256, rand() % 256, rand() % 256));

		wstring paint_str;

		// if randomize text content, use random Unicode characters
		// otherwise use the font name
		if (random_text)
		{
			const int max_text_len = 10;
			paint_str.resize(rand() % max_text_len + 1);

			for (size_t i = 0; i < paint_str.size(); i++)
			{
				unsigned short chr;
				do 
				{
					chr = rand();
				} while (iswcntrl(chr));

				paint_str[i] = chr;
			}
		}
		else
			paint_str = font_name;

		SIZE text_extent = {};
		b_ret = dc.GetTextExtent(paint_str.c_str(), static_cast<int>(paint_str.size()), &text_extent);
		assert(b_ret);

		// randomize text position
		const int x = rand() % (dc.m_ps.rcPaint.right - dc.m_ps.rcPaint.left - text_extent.cx);
		const int y = rand() % (dc.m_ps.rcPaint.bottom - dc.m_ps.rcPaint.top - text_extent.cy);

		b_ret = dc.ExtTextOut(x, y, 0, NULL, paint_str.c_str(), static_cast<UINT>(paint_str.size()), NULL);
		assert(b_ret);

		dc.GetCurrentFont().DeleteObject();

		_painted_count += 1;

		// show the rendered text count in the window title
		wchar_t new_title[GDIPP_DEMO_MAX_STR_LEN];
		wsprintf(new_title, TEXT("Paint - %u"), _painted_count);
		SetWindowText(dc.m_hWnd, new_title);

		// force redraw the client rect
		InvalidateRect(dc.m_hWnd, NULL, FALSE);
	}
	else
	{
		if (!_result_prepared)
		{
			const DWORD elapse_time = GetTickCount() - _start_time;
			swprintf(_result_str, GDIPP_DEMO_MAX_STR_LEN, L"%u milliseconds render time, %.2f ms per text run", elapse_time, static_cast<float>(elapse_time) / _painted_count);

			dc.FillRect(&dc.m_ps.rcPaint, COLOR_BTNFACE);

			_result_font = CreateFontW(-20, 0, 0, 0, FW_REGULAR, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Tahoma");

			_result_prepared = true;
		}

		dc.SetTextColor(RGB(0x20, 0x80, 0x40));
		dc.SetBkColor(GetSysColor(COLOR_BTNFACE));
		dc.SetBkMode(OPAQUE);
		dc.SetTextAlign(TA_LEFT | TA_TOP);
		dc.SelectFont(_result_font);
		dc.ExtTextOut(10, 10, 0, NULL, _result_str, static_cast<UINT>(wcslen(_result_str)), NULL);
		
		ValidateRect(dc.m_hWnd, NULL);
	}
}

void gdipp_demo_painter::stop_painting()
{
	_total_count = -1;
}