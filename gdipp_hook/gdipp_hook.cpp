#include "stdafx.h"
#include "gdipp_config/constant_hook.h"

__declspec(dllimport) LRESULT CALLBACK gdipp_client_hook_proc(int nCode, WPARAM wParam, LPARAM lParam);

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	const wchar_t *env_var_name = L"gdipp_svc_proc_id";
	DWORD env_var_len = GetEnvironmentVariableW(env_var_name, NULL, 0);
	if (env_var_len == 0)
		return EXIT_FAILURE;

	wchar_t *hook_env_str = new wchar_t[env_var_len];
	env_var_len = GetEnvironmentVariableW(env_var_name, hook_env_str, env_var_len);
	assert(env_var_len > 0);

	DWORD gdipp_svc_proc_id;
	std::wistringstream(hook_env_str) >> gdipp_svc_proc_id;

	HANDLE h_gdipp_svc = OpenProcess(SYNCHRONIZE, FALSE, gdipp_svc_proc_id);
	if (h_gdipp_svc == NULL)
		return EXIT_FAILURE;

#ifdef _M_X64
	const wchar_t *client_name = L"gdipp_client_64.dll";
#else
	const wchar_t *client_name = L"gdipp_client_32.dll";
#endif

	const HMODULE h_client = GetModuleHandleW(client_name);
	assert(h_client != NULL);

	const HHOOK h_hook = SetWindowsHookExW(WH_CBT, gdipp_client_hook_proc, h_client, 0);
	if (h_hook == NULL)
		return EXIT_FAILURE;

	WaitForSingleObject(h_gdipp_svc, INFINITE);
	UnhookWindowsHookEx(h_hook);

	return EXIT_SUCCESS;
}
