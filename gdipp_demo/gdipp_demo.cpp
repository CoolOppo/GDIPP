// gdipp_demo.cpp : main source file for gdipp_demo.exe
//

#include "stdafx.h"

#include "resource.h"

#include "gdipp_demoView.h"
#include "aboutdlg.h"
#include "MainFrm.h"

#include "gdipp_demo.h"
#include <gdipp_common.h>

CAppModule _Module;

int total_count = 5000;
vector<const wstring> candidate_font;
bool random_text = false;

WCHAR window_title[100];

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainFrame wndMain;

	if(wndMain.CreateEx() == NULL)
	{
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}

	wndMain.ShowWindow(nCmdShow);

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	HMODULE h_gdimm = NULL;

#ifdef render

#ifdef _M_X64
	h_gdimm = LoadLibrary(TEXT("gdimm_64.dll"));
#else
	h_gdimm = LoadLibrary(TEXT("gdimm_32.dll"));
#endif // _M_X64

#endif // render

#ifdef test
	wcs_convert(get_demo_setting(L"count"), &total_count);
	candidate_font = get_demo_font();
	if (candidate_font.empty())
		candidate_font.push_back(L"Tahoma");
	wcs_convert(get_demo_setting(L"random_text"), &random_text);

	window_title[0] = L'\0';

	//total_count = 0;
#endif // test

	HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
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
