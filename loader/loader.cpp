// notepad.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <shlwapi.h>

#define notepad "C:\\Windows\\notepad.exe"

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	DWORD dw_ret;
	BOOL b_ret;

	HKEY key_win;
	dw_ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows"), 0, KEY_ALL_ACCESS, &key_win);
	assert(dw_ret == ERROR_SUCCESS);

	BYTE orig_dll[MAX_PATH];
	DWORD orig_dll_len = MAX_PATH;
	DWORD reg_type;
	dw_ret = RegQueryValueEx (key_win, TEXT("AppInit_DLLs"), NULL, &reg_type, orig_dll, &orig_dll_len);
	assert(dw_ret == ERROR_SUCCESS);

	TCHAR hook_dll[MAX_PATH];
	dw_ret = GetModuleFileName(NULL, hook_dll, MAX_PATH);
	assert(dw_ret != 0);

	b_ret = PathRemoveFileSpec(hook_dll);
	assert(b_ret == TRUE);

	b_ret = PathAppend(hook_dll, TEXT("dll.dll"));
	assert(b_ret == TRUE);

	dw_ret = RegSetValueEx(key_win, TEXT("AppInit_DLLs"), NULL, reg_type, (LPBYTE) hook_dll, lstrlen(hook_dll) * sizeof(TCHAR));
	assert(dw_ret == ERROR_SUCCESS);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	memset(&pi, 0, sizeof(pi));

	b_ret = CreateProcess(TEXT(notepad), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	assert(b_ret == TRUE);

	WaitForSingleObject(pi.hProcess, 500);

	dw_ret = RegSetValueEx(key_win, TEXT("AppInit_DLLs"), NULL, reg_type, orig_dll, orig_dll_len);
	assert(dw_ret == ERROR_SUCCESS);

	dw_ret = RegCloseKey(key_win);
	assert(dw_ret == ERROR_SUCCESS);

	return 0;
}
