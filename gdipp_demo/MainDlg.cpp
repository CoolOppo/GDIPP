// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "aboutdlg.h"
#include "MainDlg.h"

DWORD start_time;

void CMainDlg::prepare_result(CPaintDC &dc)
{
	RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE);

	dc.SetTextColor(RGB(0, 0, 0));
	dc.SetBkMode(TRANSPARENT);
	dc.SetTextAlign(TA_LEFT | TA_TOP);

	const DWORD elapse_time = GetTickCount() - start_time;
	swprintf(_result_str, GDIPP_DEMO_MAX_STR_LEN, L"%u milliseconds render time, %.2f ms per text run", elapse_time, (float) elapse_time / total_count);

	_curr_font = CreateFontW(-20, 0, 0, 0, FW_REGULAR, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Tahoma");
}

BOOL CMainDlg::PreTranslateMessage(MSG* pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

BOOL CMainDlg::OnIdle()
{
	return FALSE;
}

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// center the dialog on the screen
	CenterWindow();

	// set icons
	HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);
	HMENU hMenu = (HMENU)::LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME));
	SetMenu(hMenu);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	UIAddChildWindowContainer(m_hWnd);

	return TRUE;
}

LRESULT CMainDlg::OnClose(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CloseDialog((int) wParam);
	return 0;
}

LRESULT CMainDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	if (_curr_font != NULL)
	{
		DeleteObject(_curr_font);
		_curr_font = NULL;
	}

	return 0;
}

LRESULT CMainDlg::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	static int rendered = 0;
	CPaintDC dc(m_hWnd);

	if (window_title[0] == L'\0')
		GetWindowText(window_title, GDIPP_DEMO_MAX_STR_LEN);

	if (rendered == 0)
	{
		srand((unsigned) time(NULL));
		start_time = GetTickCount();
	}

	if (rendered <= total_count)
	{
#ifdef test
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
			wchar_t render_str[max_text_len];
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
			dc.ExtTextOut(x, y, 0, NULL, font_name.c_str(), (UINT) font_name.size(), NULL);

		DeleteObject(_curr_font);
		_curr_font = NULL;

		// show the rendered text count in the window title
		wchar_t new_title[GDIPP_DEMO_MAX_STR_LEN];
		wsprintf(new_title, TEXT("%s - %u"), window_title, rendered);
		SetWindowText(new_title);
#endif // test

		// force redraw the client rect
		RedrawWindow(NULL, NULL, RDW_INVALIDATE);
	}
	else
	{
		if (rendered == total_count + 1)
			prepare_result(dc);

		dc.SelectFont(_curr_font);

		dc.ExtTextOut(10, 10, 0, NULL, _result_str, (UINT) wcslen(_result_str), NULL);
	}

	rendered += 1;

	return 0;
}

LRESULT CMainDlg::OnFileExit(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add validation code 
	CloseDialog(wID);
	return 0;
}

LRESULT CMainDlg::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

void CMainDlg::CloseDialog(int nVal)
{
	DestroyWindow();
	::PostQuitMessage(nVal);
}
