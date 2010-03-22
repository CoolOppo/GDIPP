#include "stdafx.h"
#include "inject.h"
#include <global.h>

svc_injector::svc_injector()
{
	get_dir_file_path(NULL, L"gdimm_64.dll", _gdimm_path_64);
	get_dir_file_path(NULL, L"gdimm_32.dll", _gdimm_path_32);
}

void svc_injector::init_payload(const WCHAR *svc_event_name)
{
	_payload.inject_type = GDIPP_SERVICE;
	wcscpy_s(_payload.svc_event_name, svc_event_name);
}

bool svc_injector::inject_proc(LONG proc_id)
{
	NTSTATUS eh_error;

	// if the target process has loaded gdimm, do not inject

	HANDLE h_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, proc_id);
	assert(h_snapshot != INVALID_HANDLE_VALUE);

	MODULEENTRY32 me32 = {0};
	me32.dwSize = sizeof(MODULEENTRY32);

	if (Module32First(h_snapshot, &me32))
	{
		do
		{
			// full path without the trailing "32.dll" and "64.dll"
			if (_wcsnicmp(me32.szExePath, _gdimm_path_32, wcslen(_gdimm_path_32) - 6) == 0)
			{
				// gdimm is loaded
				CloseHandle(h_snapshot);
				return false;
			}
		} while (Module32Next(h_snapshot, &me32));
	}

	CloseHandle(h_snapshot);

#ifdef _M_X64
	eh_error = RhInjectLibrary(proc_id, 0, EASYHOOK_INJECT_DEFAULT, NULL, _gdimm_path_64, (PVOID) &_payload, sizeof(inject_payload));
#else
	eh_error = RhInjectLibrary(proc_id, 0, EASYHOOK_INJECT_DEFAULT, _gdimm_path_32, NULL, (PVOID) &_payload, sizeof(inject_payload));
#endif

	return (eh_error == 0);
}

void svc_injector::initial_inject()
{
	list<const WCHAR*> init_proc;
	init_proc.push_back(L"explorer.exe");

	HANDLE h_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	assert(h_snapshot != INVALID_HANDLE_VALUE);

	PROCESSENTRY32W pe32 = {0};
	pe32.dwSize = sizeof(PROCESSENTRY32W);

	if (Process32FirstW(h_snapshot, &pe32))
	{
		do
		{
			for (list<const WCHAR*>::const_iterator iter = init_proc.begin(); iter != init_proc.end(); iter++)
			{
				if (_wcsicmp(pe32.szExeFile, *iter) == 0)
					inject_proc(pe32.th32ProcessID);
			}

		} while (Process32NextW(h_snapshot, &pe32));
	}

	CloseHandle(h_snapshot);
}