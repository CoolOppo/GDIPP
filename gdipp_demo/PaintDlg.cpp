// PaintDlg.cpp : implementation of the CPaintDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PaintDlg.h"
#include "gdipp_demo.h"

LRESULT CPaintDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	RECT window_rect;
	GetWindowRect(&window_rect);

	const int dlg_index = static_cast<const int>(lParam);
	SetWindowPos(HWND_TOP, (window_rect.right - window_rect.left) * (dlg_index / 2 % 2), (window_rect.bottom - window_rect.top) * (dlg_index % 2), 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);

	HMENU hMenu = (HMENU)::LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_PAINTMENU));
	SetMenu(hMenu);

	paint_hwnd.push_back(m_hWnd);

	return TRUE;
}

LRESULT CPaintDlg::OnClose(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	EndDialog((int) wParam);
	return 0;
}

LRESULT CPaintDlg::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CPaintDC dc(m_hWnd);

	_painter.paint_demo(dc);

	return 0;
}

LRESULT CPaintDlg::OnToolsStop(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	_painter.stop_painting();

	return 0;
}