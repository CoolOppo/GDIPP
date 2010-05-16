#include "stdafx.h"
#include "gdipp_enum.h"
#include <gdipp_common.h>

using namespace std;

int APIENTRY wWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	return 0;

	int argc;
	LPWSTR *argv = CommandLineToArgvW(lpCmdLine, &argc);
	assert(argv != NULL);

	const wchar_t *param_prefix = L"--svc_name=";
	const size_t param_length = wcslen(param_prefix);
	
	const size_t MAX_SVC_NAME_LEN = 64;
	wchar_t svc_name[MAX_SVC_NAME_LEN] = {};

	for (int i = 0; i < argc; i++)
	{
		if (_wcsnicmp(argv[i], param_prefix, param_length) == 0)
		{
			wcsncpy_s(svc_name, argv[i] + param_length, MAX_SVC_NAME_LEN);
			break;
		}
	}

	gdipp_init_payload(GDIPP_SERVICE, svc_name);

	list<const wchar_t*> init_proc;
	init_proc.push_back(L"explorer.exe");

	HANDLE h_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	assert(h_snapshot != INVALID_HANDLE_VALUE);

	PROCESSENTRY32W pe32 = {};
	pe32.dwSize = sizeof(PROCESSENTRY32W);

	if (Process32FirstW(h_snapshot, &pe32))
	{
		do
		{
			for (list<const wchar_t*>::const_iterator iter = init_proc.begin(); iter != init_proc.end(); iter++)
			{
				if (_wcsicmp(pe32.szExeFile, *iter) == 0)
					gdipp_inject_process(pe32.th32ProcessID);
			}

		} while (Process32NextW(h_snapshot, &pe32));
	}

	CloseHandle(h_snapshot);

	return 0;
}