#include "stdafx.h"
#include <gdipp_common.h>

__declspec(dllimport) void gdimm_empty_proc();

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	const HANDLE h_svc_event = OpenEventW(SYNCHRONIZE, FALSE, GDIPP_SVC_EVENT_NAME);
	if (h_svc_event == NULL)
		return EXIT_FAILURE;

#ifdef _M_X64
	const wchar_t *gdimm_name = L"gdimm_64.dll";
#else
	const wchar_t *gdimm_name = L"gdimm_32.dll";
#endif

	gdimm_empty_proc();

	const HMODULE h_gdimm = GetModuleHandleW(gdimm_name);
	if (h_gdimm == NULL)
		return EXIT_FAILURE;

#ifdef _M_X64
	const HOOKPROC h_proc = (HOOKPROC) GetProcAddress(h_gdimm, "gdimm_hook_proc");
#else
	// 32-bit exported function name is mangled
	const HOOKPROC h_proc = (HOOKPROC) GetProcAddress(h_gdimm, "_gdimm_hook_proc@12");
#endif

	if (h_proc != NULL)
	{
		const HHOOK h_hook = SetWindowsHookExW(WH_SHELL, h_proc, h_gdimm, 0);

		if (h_proc != NULL)
			WaitForSingleObject(h_svc_event, INFINITE);

		UnhookWindowsHookEx(h_hook);
	}

	FreeLibrary(h_gdimm);

	return EXIT_SUCCESS;
}