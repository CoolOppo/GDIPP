// notepad.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "global.h"
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

void inject_gdimm(DWORD proc_id, LPTSTR gdimm_path)
{
	NTSTATUS eh_error;

#ifdef _M_X64
	eh_error = RhInjectLibrary(proc_id, 0, EASYHOOK_INJECT_DEFAULT, NULL, gdimm_path, NULL, 0);
#else
	eh_error = RhInjectLibrary(proc_id, 0, EASYHOOK_INJECT_DEFAULT, gdimm_path, NULL, NULL, 0);
#endif

	assert(eh_error == 0);
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	BOOL b_ret;

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
	
	b_ret = PathAddExtension(proc_name, TEXT(".exe"));
	assert(b_ret);

	b_ret = PathRemoveFileSpec(curr_dir);
	assert(b_ret);

	DWORD proc_id = get_proc_id(proc_name);
	if (proc_id == -1)
		proc_id = load_process(lpCmdLine, curr_dir);

	if (proc_id != 0)
	{
		TCHAR gdimm_path[MAX_PATH];
#ifdef _M_X64
		get_dir_file_path(gdimm_path, TEXT("gdimm_64.dll"));
#else
		get_dir_file_path(gdimm_path, TEXT("gdimm_32.dll"));
#endif

		inject_gdimm(proc_id, gdimm_path);
		return EXIT_SUCCESS;
	}
	else
		return EXIT_FAILURE;
}
