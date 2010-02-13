// notepad.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <easyhook.h>
#include <shlwapi.h>
#include <tlhelp32.h>

DWORD get_proc_id(LPCTSTR proc_name)
{
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	HANDLE h_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	assert(h_snapshot != INVALID_HANDLE_VALUE);
	
	if (Process32First(h_snapshot, &pe32))
	{
		do
		{
			if (lstrcmpi(pe32.szExeFile, proc_name) == 0)
			{
				CloseHandle(h_snapshot);
				return pe32.th32ProcessID;
			}

		} while (Process32Next(h_snapshot, &pe32));
	}

	CloseHandle(h_snapshot);
	return -1;
}

DWORD load_process(LPTSTR proc_name, LPCTSTR curr_dir)
{
	STARTUPINFO si = {0};
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi = {0};

	BOOL b_ret = CreateProcess(NULL, proc_name, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	if (!b_ret)
	{
		MessageBox(NULL, TEXT("Fail to load the program. Please verify that it is an exe file and try again."), TEXT("Error"), MB_OK | MB_ICONERROR);
		return 0;
	}

	return pi.dwProcessId;
}

void inject_dll(DWORD proc_id, LPTSTR dll_path)
{
#ifdef _M_X64
	NTSTATUS eh_error = RhInjectLibrary(proc_id, 0, EASYHOOK_INJECT_DEFAULT, NULL, dll_path, NULL, 0);
#else
	NTSTATUS eh_error = RhInjectLibrary(proc_id, 0, EASYHOOK_INJECT_DEFAULT, dll_path, NULL, NULL, 0);
#endif
	assert(eh_error == 0);
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	DWORD dw_ret;
	BOOL b_ret;

	TCHAR hook_dll[MAX_PATH];
	dw_ret = GetModuleFileName(NULL, hook_dll, MAX_PATH);
	assert(dw_ret != 0);

	b_ret = PathRemoveFileSpec(hook_dll);
	assert(b_ret);

#ifdef _M_X64
	b_ret = PathAppend(hook_dll, TEXT("gdimm_64.dll"));
#else
	b_ret = PathAppend(hook_dll, TEXT("gdimm_32.dll"));
#endif
	assert(b_ret);

	if (lstrlen(lpCmdLine) == 0)
	{
		MessageBox(NULL, TEXT("Drag an exe file to me and I will load it with gdimm."), TEXT("loader"), MB_OK | MB_ICONINFORMATION);
		return EXIT_SUCCESS;
	}

	TCHAR proc_name[MAX_PATH];
	TCHAR curr_dir[MAX_PATH];
	lstrcpyn(proc_name, lpCmdLine, MAX_PATH);
	lstrcpyn(curr_dir, lpCmdLine, MAX_PATH);
	PathStripPath(proc_name);
	PathAddExtension(proc_name, TEXT(".exe"));
	PathRemoveFileSpec(curr_dir);

	DWORD proc_id = get_proc_id(proc_name);
	if (proc_id == -1)
		proc_id = load_process(lpCmdLine, curr_dir);

	if (proc_id != 0)
	{
		inject_dll(proc_id, hook_dll);
		return EXIT_SUCCESS;
	}
	else
		return EXIT_FAILURE;
}
