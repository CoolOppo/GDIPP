#include "stdafx.h"
#include "hook.h"
#include "gdipp_client/api_override.h"
#include "gdipp_client/com_override.h"
#include "gdipp_client/global.h"
#include "gdipp_lib/helper.h"

namespace gdipp
{

// exported function for SetWindowsHookEx
__declspec(dllexport) LRESULT CALLBACK gdimm_hook_proc(int nCode, WPARAM wParam, LPARAM lParam)
{
	init_minidump();
	register_minidump_module(h_self);

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// exported function for EasyHook remote hooking
EXTERN_C __declspec(dllexport) void __stdcall NativeInjectionEntryPoint(REMOTE_ENTRY_INFO* remote_info)
{
	// the process is created suspended, wake it up
	RhWakeUpProcess();
}

bool char_str_ci_less::operator()(const char *string1, const char *string2) const
{
	// filename is insensitive in Windows
	return _stricmp(string1, string2) < 0;
}

bool wchar_str_ci_less::operator()(const wchar_t *string1, const wchar_t *string2) const
{
	return _wcsicmp(string1, string2) < 0;
}

bool hook::install_hook(LPCSTR lib_name, LPCSTR proc_name, void *hook_proc)
{
	// hook a procedure in the specified library that has been loaded before gdimm
	// ANSI version

	const HMODULE h_lib = GetModuleHandleA(lib_name);
	if (h_lib == NULL)
		return false;

	return install_hook(h_lib, proc_name, hook_proc);
}

bool hook::install_hook(LPCWSTR lib_name, LPCSTR proc_name, void *hook_proc)
{
	// hook a procedure in the specified library that has been loaded before gdimm
	// Unicode version

	const HMODULE h_lib = GetModuleHandleW(lib_name);
	if (h_lib == NULL)
		return false;

	return install_hook(h_lib, proc_name, hook_proc);
}

bool hook::install_delayed_hook(LPCSTR lib_name, HMODULE h_lib)
{
	// hook a procedure in the specified library that is dynamically loaded (e.g. by LoadLibrary())
	// the procedure is registered a priori via register_delayed_hook()
	// ANSI version
	
	bool b_ret = true;

	lib_hook_map_a::const_iterator lib_iter = _delayed_hooks_a.find(lib_name);
	if (lib_iter != _delayed_hooks_a.end())
	{
		for (hook_proc_map::const_iterator proc_iter = lib_iter->second->begin(); proc_iter != lib_iter->second->end(); ++proc_iter)
		{
			b_ret &= install_hook(h_lib, proc_iter->first, proc_iter->second);
			if (!b_ret)
				break;
		}
	}

	return b_ret;
}

bool hook::install_delayed_hook(LPCWSTR lib_name, HMODULE h_lib)
{
	// hook a procedure in the specified library that is dynamically loaded (e.g. by LoadLibrary())
	// the procedure is registered a priori via register_delayed_hook()
	// Unicode version

	bool b_ret = true;

	lib_hook_map_w::const_iterator lib_iter = _delayed_hooks_w.find(lib_name);
	if (lib_iter != _delayed_hooks_w.end())
	{
		for (hook_proc_map::const_iterator proc_iter = lib_iter->second->begin(); proc_iter != lib_iter->second->end(); ++proc_iter)
		{
			b_ret &= install_hook(h_lib, proc_iter->first, proc_iter->second);
			if (!b_ret)
				break;
		}
	}

	return b_ret;
}

bool hook::start()
{
	bool b_ret;

	b_ret = install_hook(L"gdi32.dll", "ExtTextOutW", ExtTextOutW_hook);
	if (b_ret)
	{
		// hook other GDI APIs only if ExtTextOut is successfully hooked

		// reserve for future use
// 		b_ret &= install_hook(L"user32.dll", "DrawTextExA", DrawTextExA_hook);
// 		b_ret &= install_hook(L"user32.dll", "DrawTextExW", DrawTextExW_hook);

// 		b_ret &= install_hook(L"gdi32.dll", "GetTextExtentPoint32A", GetTextExtentPoint32A_hook);
// 		b_ret &= install_hook(L"gdi32.dll", "GetTextExtentPoint32W", GetTextExtentPoint32W_hook);
// 		b_ret &= install_hook(L"gdi32.dll", "GetTextExtentPointI", GetTextExtentPointI_hook);
//
// 		b_ret &= install_hook(L"gdi32.dll", "GetTextExtentExPointA", GetTextExtentExPointA_hook);
// 		b_ret &= install_hook(L"gdi32.dll", "GetTextExtentExPointW", GetTextExtentExPointW_hook);
// 		b_ret &= install_hook(L"gdi32.dll", "GetTextExtentExPointI", GetTextExtentExPointI_hook);
//
// 		b_ret &= install_hook(L"gdi32.dll", "GetGlyphOutlineA", GetGlyphOutlineA_hook);
// 		b_ret &= install_hook(L"gdi32.dll", "GetGlyphOutlineW", GetGlyphOutlineW_hook);
//
// 		b_ret &= install_hook(L"gdi32.dll", "AbortPath", AbortPath_hook);
// 		b_ret &= install_hook(L"gdi32.dll", "BeginPath", BeginPath_hook);
// 		b_ret &= install_hook(L"gdi32.dll", "EndPath", EndPath_hook);
//
// 		install_hook(L"usp10.dll", "ScriptPlace", ScriptPlace_hook);

		// register hooks whose libraries are dynamically loaded by LoadLibrary
		//register_delayed_hook("d2d1.dll", L"d2d1.dll", "D2D1CreateFactory", D2D1CreateFactory_hook);

		if (!_delayed_hook_registry.empty())
		{
			install_hook(L"kernel32.dll", "LoadLibraryA", LoadLibraryA_hook);
			install_hook(L"kernel32.dll", "LoadLibraryExA", LoadLibraryExA_hook);
			install_hook(L"kernel32.dll", "LoadLibraryW", LoadLibraryW_hook);
			install_hook(L"kernel32.dll", "LoadLibraryExW", LoadLibraryExW_hook);
		}
	}

#if defined GDIPP_INJECT_SANDBOX && !defined _M_X64
	// currently not support inject at EIP for 64-bit processes
	b_ret &= install_hook(L"advapi32.dll", "CreateProcessAsUserW", CreateProcessAsUserW_hook);
#endif // GDIPP_INJECT_SANDBOX && !_M_X64

	return b_ret;
}

void hook::stop()
{
	NTSTATUS eh_ret;

	eh_ret = LhUninstallAllHooks();
	assert(eh_ret == 0);

	eh_ret = LhWaitForPendingRemovals();
	assert(eh_ret == 0);

	for (std::list<TRACED_HOOK_HANDLE>::const_iterator iter = _hooks.begin(); iter != _hooks.end(); ++iter)
		delete *iter;
}

bool hook::install_hook(HMODULE h_lib, LPCSTR proc_name, void *hook_proc)
{
	// hook a procedure in the specified library that has been loaded before gdimm
	// use EasyHook
	
	NTSTATUS eh_ret;

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

void hook::register_delayed_hook(LPCSTR lib_name_a, LPCWSTR lib_name_w, LPCSTR proc_name, void *hook_proc)
{
	// register a procedure for delayed hook
	
	lib_hook_map_a::const_iterator lib_iter = _delayed_hooks_a.find(lib_name_a);
	if (lib_iter == _delayed_hooks_a.end())
	{
		_delayed_hook_registry.push_back(hook_proc_map());
		hook_proc_map *curr_hook_map = &_delayed_hook_registry.back();

		(*curr_hook_map)[proc_name] = hook_proc;
		_delayed_hooks_a[lib_name_a] = curr_hook_map;
		_delayed_hooks_w[lib_name_w] = curr_hook_map;
	}
	else
		(*lib_iter->second)[proc_name] = hook_proc;
}

}
