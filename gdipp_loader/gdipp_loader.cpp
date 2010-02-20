#include "stdafx.h"
#include "global.h"
#include <easyhook.h>

int APIENTRY _tWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	BOOL b_ret;
	NTSTATUS eh_error;

	if (lstrlen(lpCmdLine) == 0)
	{
		MessageBox(NULL, TEXT("Drag an exe file to me and I will load it with gdimm.dll."), TEXT("loader"), MB_OK | MB_ICONINFORMATION);
		return EXIT_SUCCESS;
	}

	STARTUPINFO si = {0};
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi = {0};

	b_ret = CreateProcess(NULL, lpCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	if (!b_ret)
	{
		MessageBox(NULL, TEXT("Fail to load the program. Please verify that it is an exe file and try again."), TEXT("Error"), MB_OK | MB_ICONERROR);
		return EXIT_FAILURE;
	}

	BOOL is_wow64;
	b_ret = IsWow64Process(pi.hProcess, &is_wow64);
	assert(b_ret);

	TCHAR gdimm_path[MAX_PATH];
	const INJECTOR_TYPE injector_type = GDIPP_LOADER;
	if (is_wow64)
	{
		get_dir_file_path(gdimm_path, TEXT("gdimm_64.dll"));
		eh_error = RhInjectLibrary(pi.dwProcessId, 0, EASYHOOK_INJECT_DEFAULT, NULL, gdimm_path, (PVOID) &injector_type, sizeof(INJECTOR_TYPE));
		assert(eh_error == 0);
	}
	else
	{
		get_dir_file_path(gdimm_path, TEXT("gdimm_32.dll"));
		eh_error = RhInjectLibrary(pi.dwProcessId, 0, EASYHOOK_INJECT_DEFAULT, gdimm_path, NULL, (PVOID) &injector_type, sizeof(INJECTOR_TYPE));
		assert(eh_error == 0);
	}

	return EXIT_SUCCESS;
}
