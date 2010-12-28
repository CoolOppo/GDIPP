#include "stdafx.h"
#include <gdipp_lib.h>

__declspec(dllimport) LRESULT CALLBACK gdimm_hook_proc(int nCode, WPARAM wParam, LPARAM lParam);

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	const wchar_t *env_var_name = L"h_gdipp_hook_wait";
	DWORD env_var_len = GetEnvironmentVariableW(env_var_name, NULL, 0);
	if (env_var_len == 0)
		return EXIT_FAILURE;

	wchar_t *wait_handle_str = new wchar_t[env_var_len];
	env_var_len = GetEnvironmentVariableW(env_var_name, wait_handle_str, env_var_len);
	assert(env_var_len > 0);

#ifdef _M_X64
	const wchar_t *gdimm_name = L"gdimm_64.dll";
#else
	const wchar_t *gdimm_name = L"gdimm_32.dll";
#endif

	const HMODULE h_gdimm = GetModuleHandleW(gdimm_name);
	assert(h_gdimm != NULL);

	const HHOOK h_hook = SetWindowsHookExW(WH_CBT, gdimm_hook_proc, h_gdimm, 0);
	if (h_hook == NULL)
		return EXIT_FAILURE;

	HANDLE h_wait;
	wistringstream(wait_handle_str) >> h_wait;
	WaitForSingleObject(h_wait, INFINITE);
	UnhookWindowsHookEx(h_hook);

	return EXIT_SUCCESS;
}