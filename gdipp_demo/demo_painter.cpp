#include "stdafx.h"
#include "demo_painter.h"
#include "gdipp_config/constant_demo.h"
#include "gdipp_demo/global.h"

namespace gdipp
{

demo_painter::demo_painter()
	: _painted_cycles(0),
	_result_prepared(false),
	_result_font(NULL)
{
	srand(GetTickCount());

	_total_cycles = config_instance.get_number(L"/gdipp/demo/cycles", demo_config::CYCLES);
}

demo_painter::~demo_painter()
{
	if (_result_font != NULL)
		DeleteObject(_result_font);
}

void demo_painter::paint_demo(CPaintDC &dc)
{
	BOOL b_ret;

	if (_painted_cycles == 0)
		_start_time = GetTickCount();

	if (_painted_cycles < _total_cycles)
	{
		std::vector<const wchar_t *> demo_fonts;
		b_ret = config_instance.get_string_list(L"/gdipp/demo/fonts", demo_fonts);
		assert(b_ret);

		// randomize text metrics
		const LONG text_height = (rand() % 10) + 9;
		const LONG text_weight = (rand() % 8 + 1) * 100;
		const BYTE text_italic = rand() % 2;
		const std::wstring &font_name = demo_fonts[rand() % demo_fonts.size()];

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

		std::wstring paint_str;

		// if randomize text content, use random Unicode characters
		// otherwise use the font name
		if (!!config_instance.get_number(L"/gdipp/demo/random_text", static_cast<int>(demo_config::RANDOM_TEXT)))
		{
			const int max_text_len = 10;
			paint_str.resize(rand() % max_text_len + 1);

			for (size_t i = 0; i < paint_str.size(); ++i)
			{
				int chr;
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

		_painted_cycles += 1;

		// show the rendered text count in the window title
		wchar_t new_title[GDIPP_DEMO_MAX_STR_LEN];
		wsprintf(new_title, TEXT("Paint - %u"), _painted_cycles);
		SetWindowText(dc.m_hWnd, new_title);

		// force redraw the client rect
		InvalidateRect(dc.m_hWnd, NULL, FALSE);
	}
	else
	{
		if (!_result_prepared)
		{
			const DWORD elapse_time = GetTickCount() - _start_time;
			swprintf(_result_str, GDIPP_DEMO_MAX_STR_LEN, L"%u milliseconds render time, %.2f ms per text run", elapse_time, static_cast<float>(elapse_time) / _painted_cycles);

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

void demo_painter::stop_painting()
{
	_total_cycles = -1;
}

}
