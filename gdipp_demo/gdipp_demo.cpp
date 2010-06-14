// gdipp_demo.cpp : main source file for gdipp_demo.exe
//

#include "stdafx.h"

#include "resource.h"

#include "aboutdlg.h"
#include "MainDlg.h"

#include "gdipp_demo.h"
#include <gdipp_common.h>

CAppModule _Module;

int total_count = 5000;
vector<const wstring> candidate_font;
bool random_text = false;

wchar_t window_title[GDIPP_DEMO_MAX_STR_LEN];

HMODULE h_gdimm = NULL;
WCHAR gdimm_path[MAX_PATH];

BOOL load_setting()
{
	BOOL b_ret;

	// get setting file path
	wchar_t setting_path[MAX_PATH];
	b_ret = gdipp_get_dir_file_path(NULL, L"gdipp_setting.xml", setting_path);
	if (!b_ret)
		return FALSE;

	gdipp_init_setting();
	return gdipp_load_setting(setting_path);
}

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainDlg dlgMain;

	if(dlgMain.Create(NULL) == NULL)
	{
		ATLTRACE(_T("Main dialog creation failed!\n"));
		return 0;
	}

	dlgMain.ShowWindow(nCmdShow);

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	BOOL b_ret;

#ifdef render
#ifdef _M_X64
	b_ret = gdipp_get_dir_file_path(NULL, L"gdimm_64.dll", gdimm_path);
#else
	b_ret = gdipp_get_dir_file_path(NULL, L"gdimm_32.dll", gdimm_path);
#endif // _M_X64

	h_gdimm = LoadLibraryW(gdimm_path);
#else
	b_ret = load_setting();
	assert(b_ret);
#endif // render

#ifdef test
	wcs_convert(gdipp_get_demo_setting(L"count"), &total_count);
	candidate_font = gdipp_get_demo_fonts();
	if (candidate_font.empty())
		candidate_font.push_back(L"Tahoma");
	wcs_convert(gdipp_get_demo_setting(L"random_text"), &random_text);

	window_title[0] = L'\0';
#endif // test

	// If you are running on NT 4.0 or higher you can use the following call instead to 
	// make the EXE free threaded. This means that calls come in on a random RPC thread.
	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();
	::CoUninitialize();

	if (h_gdimm != NULL)
		FreeLibrary(h_gdimm);

	return nRet;
}
