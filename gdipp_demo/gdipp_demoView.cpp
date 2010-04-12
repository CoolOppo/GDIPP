// gdipp_demoView.cpp : implementation of the Cgdipp_demoView class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "gdipp_demoView.h"
#include "gdipp_demo.h"

DWORD start_time;

void Cgdipp_demoView::prepare_result(CPaintDC &dc)
{
	RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ERASENOW);

	dc.SetTextColor(RGB(0, 0, 0));
	dc.SetBkMode(TRANSPARENT);
	dc.SetTextAlign(TA_LEFT | TA_TOP);

	const DWORD elapse_time = GetTickCount() - start_time;
	swprintf(_result_str, 100, L"%u milliseconds render time, %.2f ms per render", elapse_time, (double) elapse_time / total_count);

	_curr_font = CreateFontW(-20, 0, 0, 0, FW_REGULAR, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Tahoma");
}

BOOL Cgdipp_demoView::PreTranslateMessage(MSG* pMsg)
{
	pMsg;
	return FALSE;
}

LRESULT Cgdipp_demoView::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	if (_curr_font != NULL)
	{
		DeleteObject(_curr_font);
		_curr_font = NULL;
	}

	bHandled = FALSE;
	return 1;
}

LRESULT Cgdipp_demoView::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
#ifdef test
	static int rendered = 0;
	CPaintDC dc(m_hWnd);

	if (window_title[0] == L'\0')
		GetParent().GetWindowText(window_title, 100);

	if (rendered == 0)
	{
		srand((unsigned) time(NULL));
		start_time = GetTickCount();
	}

	if (rendered <= total_count)
	{
		// randomize text color
		dc.SetTextColor(RGB(rand() % 256, rand() % 256, rand() % 256));

		// randomize text background
		int bk_mode = rand() % 2 + 1;
		dc.SetBkMode(bk_mode);
		if (bk_mode == OPAQUE)
			dc.SetBkColor(RGB(rand() % 256, rand() % 256, rand() % 256));

		// randomize text position
		const int x = rand() % (dc.m_ps.rcPaint.right - dc.m_ps.rcPaint.left);
		const int y = rand() % (dc.m_ps.rcPaint.bottom - dc.m_ps.rcPaint.top);

		// randomize text metrics
		const LONG height = (rand() % 10) + 10;
		const LONG weight = (rand() % 8 + 1) * 100;
		const BYTE italic = rand() % 2;
		const wstring &font_name = candidate_font[rand() % candidate_font.size()];

		_curr_font = CreateFontW(-height, 0, 0, 0, weight, italic, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, font_name.c_str());
		dc.SelectFont(_curr_font);

		// if randomize text content, use random Unicode characters
		// otherwise use the font name
		if (random_text)
		{
			const int max_text_len = 10;
			WCHAR render_str[max_text_len];
			const int render_len = rand() % max_text_len + 1;

			for (int i = 0; i < render_len; i++)
			{
				unsigned short chr;
				do 
				{
					chr = rand();
				} while (iswcntrl(chr));
				render_str[i] = chr;
			}

			dc.ExtTextOut(x, y, 0, NULL, render_str, render_len, NULL);
		}
		else
			dc.ExtTextOut(x, y, 0, NULL, font_name.c_str(), font_name.size(), NULL);

		DeleteObject(_curr_font);
		_curr_font = NULL;

		// show the rendered text count in the window title
		WCHAR new_title[100];
		wsprintf(new_title, TEXT("%s - %u"), window_title, rendered);
		GetParent().SetWindowText(new_title);

		// force redraw the client rect
		RedrawWindow(NULL, NULL, RDW_INVALIDATE);
	}
	else
	{
		if (rendered == total_count + 1)
			prepare_result(dc);

		dc.SelectFont(_curr_font);
		dc.ExtTextOut(10, 10, 0, NULL, _result_str, wcslen(_result_str), NULL);
	}

	rendered += 1;
#endif // test

	return 0;
}