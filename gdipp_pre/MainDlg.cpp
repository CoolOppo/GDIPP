// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "gdipp_pre/resource.h"
#include "MainDlg.h"
#include "gdipp_pre/gdipp_pre.h"
#include "gdipp_pre/aboutdlg.h"

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
	SetDlgItemTextW(IDC_STATIC, preview_text);

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
	CloseDialog(static_cast<int>(wParam));
	return 0;
}

LRESULT CMainDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	return 0;
}

LRESULT CMainDlg::OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	gdipp::init_config();
	return 0;
}

LRESULT CMainDlg::OnFileOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL b_ret;

	OPENFILENAMEW ofn = {};
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFilter = L"XML Files\0*.xml\0\0";
	ofn.lpstrFile = config_path;
	ofn.nMaxFile = MAX_PATH;

	b_ret = GetOpenFileNameW(&ofn);
	if (b_ret)
	{
		gdipp::init_config();
		b_ret = gdipp::load_config(config_path);
	}

	return 0;
}

LRESULT CMainDlg::OnFileSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	//BOOL b_ret = gdipp::save_config(config_path);
	assert(false);
	return 0;
}

LRESULT CMainDlg::OnFileSaveAs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	/*BOOL b_ret;

	OPENFILENAMEW ofn = {};
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFilter = L"XML Files\0*.xml\0\0";
	ofn.lpstrFile = config_path;
	ofn.nMaxFile = MAX_PATH;

	b_ret = GetSaveFileNameW(&ofn);
	if (b_ret)
	{
		b_ret = gdipp::save_config(config_path);
	}*/
	assert(false);
	return 0;
}

LRESULT CMainDlg::OnFileExit(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add validation code
	CloseDialog(wID);
	return 0;
}

LRESULT CMainDlg::OnChangeFont(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL b_ret;

	const HFONT old_font = reinterpret_cast<const HFONT>(SendMessage(GetDlgItem(IDC_STATIC), WM_GETFONT, 0, 0));
	assert(old_font != NULL);

	LOGFONTW lf;
	const int i_ret = GetObject(old_font, sizeof(LOGFONTW), &lf);
	assert(i_ret != 0);

	CHOOSEFONTW cf = {};
	cf.lStructSize = sizeof(CHOOSEFONTW);
	cf.hwndOwner = m_hWnd;
	cf.lpLogFont = &lf;
	cf.Flags = CF_INITTOLOGFONTSTRUCT;

	b_ret = ChooseFontW(&cf);
	if (b_ret)
	{
		HFONT new_font = CreateFontIndirect(&lf);
		assert(new_font != NULL);

		SendMessage(GetDlgItem(IDC_STATIC), WM_SETFONT, reinterpret_cast<WPARAM>(new_font), MAKELPARAM(TRUE, 0));

		DeleteObject(old_font);
	}

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
