#include "stdafx.h"
#include <gdipp_common.h>

using namespace std;

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

BOOL inject_processes()
{
	BOOL b_ret;

	DWORD curr_session_id = WTSGetActiveConsoleSessionId();
	if (curr_session_id == 0xFFFFFFFF)
		return FALSE;

	HANDLE h_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	assert(h_snapshot != INVALID_HANDLE_VALUE);

	PROCESSENTRY32W pe32 = {};
	pe32.dwSize = sizeof(PROCESSENTRY32W);

	if (Process32FirstW(h_snapshot, &pe32))
	{
		do
		{
			// processes in session 0 are usually GUI-less
			DWORD session_id;
			b_ret = ProcessIdToSessionId(pe32.th32ProcessID, &session_id);
			if (b_ret && session_id == curr_session_id && !gdipp_is_process_excluded(pe32.szExeFile))
				gdipp_inject_process(pe32.th32ProcessID);

		} while (Process32NextW(h_snapshot, &pe32));
	}

	CloseHandle(h_snapshot);

	return TRUE;
}

int APIENTRY wWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	if (!load_setting())
		return EXIT_FAILURE;

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

	gdipp_init_payload(GDIPP_INJECTOR_SERVICE, svc_name);

	inject_processes();

	return EXIT_SUCCESS;
}