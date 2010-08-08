#include "stdafx.h"
#include "hook.h"
#include "override.h"

// empty exported function to help static linking gdimm against other projects
__declspec(dllexport) void gdimm_empty_proc()
{
}

// exported function for SetWindowsHookEx
EXTERN_C __declspec(dllexport) LRESULT CALLBACK gdimm_hook_proc(int nCode, WPARAM wParam, LPARAM lParam)
{
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// exported function for EasyHook remote hooking
EXTERN_C __declspec(dllexport) void __stdcall NativeInjectionEntryPoint(REMOTE_ENTRY_INFO* remote_info)
{
	// the process is created suspended, wake it up
	RhWakeUpProcess();
}

bool gdimm_hook::install_hook(LPCWSTR lib_name, LPCSTR proc_name, void *hook_proc)
{
	NTSTATUS eh_ret;

	// the target library module must have been loaded in this process before hooking
	const HMODULE h_lib = GetModuleHandleW(lib_name);
	if (h_lib == NULL)
		return false;

	const FARPROC proc_addr = GetProcAddress(h_lib, proc_name);
	assert(proc_addr != NULL);

	TRACED_HOOK_HANDLE h_hook = new HOOK_TRACE_INFO();
	eh_ret = LhInstallHook(proc_addr, hook_proc, NULL, h_hook);
	assert(eh_ret == 0);

	ULONG thread_id_list = 0;
	eh_ret = LhSetExclusiveACL(&thread_id_list, 0, h_hook);
	assert(eh_ret == 0);

	_hooks.push_back(h_hook);

	return true;
}

bool gdimm_hook::hook()
{
	install_hook(L"gdi32.dll", "ExtTextOutW", ExtTextOutW_hook);

 	install_hook(L"gdi32.dll", "GetTextExtentPoint32A", GetTextExtentPoint32A_hook);
 	install_hook(L"gdi32.dll", "GetTextExtentPoint32W", GetTextExtentPoint32W_hook);
 	install_hook(L"gdi32.dll", "GetTextExtentPointI", GetTextExtentPointI_hook);

#if defined GDIPP_INJECT_SANDBOX && !defined _M_X64
	// currently not support inject at EIP for 64-bit processes
	install_hook(L"advapi32.dll", "CreateProcessAsUserW", CreateProcessAsUserW_hook);
#endif // GDIPP_INJECT_SANDBOX && !_M_X64

	install_hook(L"gdi32.dll", "AbortPath", AbortPath_hook);
	install_hook(L"gdi32.dll", "BeginPath", BeginPath_hook);
	install_hook(L"gdi32.dll", "EndPath", EndPath_hook);

	return !(_hooks.empty());
}

void gdimm_hook::unhook()
{
	NTSTATUS eh_ret;

	eh_ret = LhUninstallAllHooks();
	assert(eh_ret == 0);

	eh_ret = LhWaitForPendingRemovals();
	assert(eh_ret == 0);

	for (list<TRACED_HOOK_HANDLE>::const_iterator iter = _hooks.begin(); iter != _hooks.end(); iter++)
		delete *iter;
}