// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"
#include "AboutDlg.h"
#include "PaintDlg.h"
#include "gdipp_demo.h"
#include <gdipp_lib.h>
#include <support_helper.h>

DWORD WINAPI paint_thread(LPVOID context)
{
	CPaintDlg dlg;
	
	return (dlg.DoModal(NULL, reinterpret_cast<LPARAM>(context)) != IDOK);
}

BOOL CMainDlg::PreTranslateMessage(MSG* pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

BOOL CMainDlg::OnIdle()
{
	return FALSE;
}

__declspec(dllimport) void reg();

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	bool b_ret;

	// center the dialog on the screen
	CenterWindow();
	SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);

	// set icons
	HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINMENU), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINMENU), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);

	HMENU hMenu = (HMENU)::LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINMENU));
	SetMenu(hMenu);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	UIAddChildWindowContainer(m_hWnd);

	b_ret = load_gdimm();

	b_ret = load_demo_setting();
	assert(b_ret);

	for (int i = 0; i < thread_count; i++)
		CreateThread(NULL, 0, paint_thread, reinterpret_cast<void *>(i), 0, NULL);

	return TRUE;
}

LRESULT CMainDlg::OnClose(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	for (size_t i = 0; i < paint_hwnd.size(); i++)
		::EndDialog(paint_hwnd[i], (int) wParam);

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

	return 0;
}

LRESULT CMainDlg::OnFileExit(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add validation code 
	CloseDialog(wID);
	return 0;
}

LRESULT CMainDlg::OnToolsLoad(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	load_gdimm();

	return 0;
}

LRESULT CMainDlg::OnToolsUnload(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	unload_gdimm();

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

bool CMainDlg::load_demo_setting()
{
	BOOL b_ret;

	if (h_gdimm == NULL)
	{
		// get setting file path
		wchar_t setting_path[MAX_PATH];
		b_ret = gdipp_get_dir_file_path(NULL, L"gdipp_setting.xml", setting_path);
		if (!b_ret)
			return false;

		gdipp_init_setting();

		b_ret = gdipp_load_setting(setting_path);
		if (!b_ret)
			return false;
	}

	wcs_convert(gdipp_get_demo_setting(L"count"), &total_count);
	wcs_convert(gdipp_get_demo_setting(L"threads"), &thread_count);
	wcs_convert(gdipp_get_demo_setting(L"random_text"), &random_text);

	paint_fonts = gdipp_get_demo_fonts();

	// if no font is specified, use Tahoma
	if (paint_fonts.empty())
		paint_fonts.push_back(L"Tahoma");

	return true;
}

void CMainDlg::update_menu_state()
{
	BOOL b_ret;

	const bool gdimm_loaded = (h_gdimm != NULL);
	b_ret = EnableMenuItem(GetMenu(), ID_TOOLS_LOAD, MF_BYCOMMAND | (gdimm_loaded ? MF_GRAYED : MF_ENABLED));
	b_ret = EnableMenuItem(GetMenu(), ID_TOOLS_UNLOAD, MF_BYCOMMAND | (gdimm_loaded ? MF_ENABLED : MF_GRAYED));
}

bool CMainDlg::load_gdimm()
{
	BOOL b_ret;

	if (h_gdimm == NULL)
	{
#ifdef _M_X64
		const wchar_t *gdimm_name = L"gdimm_64.dll";
#else
		const wchar_t *gdimm_name = L"gdimm_32.dll";
#endif // _M_X64

		h_gdimm = GetModuleHandle(gdimm_name);
		if (h_gdimm == NULL)
		{
			b_ret = gdipp_get_dir_file_path(NULL, gdimm_name, gdimm_path);
			if (b_ret)
				h_gdimm = LoadLibraryW(gdimm_path);
		}
	}

	update_menu_state();

	return (h_gdimm != NULL);
}

bool CMainDlg::unload_gdimm()
{
	BOOL b_ret;

	if (h_gdimm != NULL)
	{
		b_ret = FreeLibrary(h_gdimm);
		if (b_ret)
			h_gdimm = NULL;
	}

	update_menu_state();

	return (h_gdimm == NULL);
}