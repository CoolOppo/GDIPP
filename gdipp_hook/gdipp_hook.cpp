#include "stdafx.h"
#include <gdipp_common.h>

__declspec(dllimport) void gdimm_empty_proc();

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	gdipp_debug_output(1);
	const HANDLE h_svc_event = OpenEventW(SYNCHRONIZE, FALSE, GDIPP_SVC_EVENT_NAME);
	if (h_svc_event == NULL)
	{
		gdipp_debug_output(L"Cannot open event");
		return EXIT_FAILURE;
	}
	gdipp_debug_output(2);
#ifdef _M_X64
	const wchar_t *gdimm_name = L"gdimm_64.dll";
#else
	const wchar_t *gdimm_name = L"gdimm_32.dll";
#endif
	gdipp_debug_output(3);
	const HMODULE h_gdimm = GetModuleHandleW(gdimm_name);
	if (h_gdimm == NULL)
	{
		gdipp_debug_output(L"Cannot get module handle");
		return EXIT_FAILURE;
	}
	gdipp_debug_output(4);
#ifdef _M_X64
	const HOOKPROC h_proc = (HOOKPROC) GetProcAddress(h_gdimm, "gdimm_hook_proc");
#else
	// 32-bit exported function name is mangled
	const HOOKPROC h_proc = (HOOKPROC) GetProcAddress(h_gdimm, "_gdimm_hook_proc@12");
#endif
	gdipp_debug_output(5);
	if (h_proc != NULL)
	{
		const HHOOK h_hook = SetWindowsHookExW(WH_CBT, h_proc, h_gdimm, 0);
		gdipp_debug_output(6);

		if (h_proc != NULL)
		{
			gdipp_debug_output(7);
			WaitForSingleObject(h_svc_event, INFINITE);
			gdipp_debug_output(8);
			UnhookWindowsHookEx(h_hook);
			gdipp_debug_output(9);
		}
		gdipp_debug_output(10);
	}
	gdipp_debug_output(11);

	FreeLibrary(h_gdimm);

	gdimm_empty_proc();

	return EXIT_SUCCESS;
}