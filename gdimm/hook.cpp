#include "stdafx.h"
#include "hook.h"
#include "global.h"
#include "text.h"

ULONG svc_proc_id = 0;
HMODULE h_self = NULL;

extern "C" __declspec(dllexport) void __stdcall NativeInjectionEntryPoint(REMOTE_ENTRY_INFO* remote_info)
{
	BOOL b_ret;

	const INJECTOR_TYPE injector_type = *(INJECTOR_TYPE*) remote_info->UserData;
	switch (injector_type)
	{
	case GDIPP_SERVICE:
		svc_proc_id = remote_info->HostPID;
		
		// force the foreground window of the injected process to redraw
		b_ret = RedrawWindow(GetForegroundWindow(), NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
		assert(b_ret);

		break;
	case GDIPP_LOADER:
		RhWakeUpProcess();
		break;
	}
}

DWORD WINAPI unload_self(LPVOID lpThreadParameter)
{
	critical_section lock;
	FreeLibraryAndExitThread(h_self, 0);
}

__gdi_entry BOOL WINAPI ExtTextOutW_hook( __in HDC hdc, __in int x, __in int y, __in UINT options, __in_opt CONST RECT * lprect, __in_ecount_opt(c) LPCWSTR lpString, __in UINT c, __in_ecount_opt(c) CONST INT * lpDx)
{
	// check if the service process is running
	// GetProcessVersion and OpenProcess are the two APIs that accept process ID as parameter
	if (GetProcessVersion(svc_proc_id) == 0)
	{
		gdimm_hook::instance().disable_hook();

		// use critical section to ensure FreeLibraryAndExitThread is called after this hooked API call
		critical_section lock;
		HANDLE h_thread = CreateThread(NULL, 0, unload_self, NULL, 0, NULL);
		assert(h_thread);

		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);
	}

	//if ((options & ETO_GLYPH_INDEX))
	//if (c != 7)
	//	return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

	// no text to render
	if (lpString == NULL || c == 0)
		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);
	
	// completely clipped
	if ((options & ETO_CLIPPED) && IsRectEmpty(lprect))
		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

	// probably a printer
	if (GetDeviceCaps(hdc, TECHNOLOGY) != DT_RASDISPLAY)
		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

	/*
	the DC use another map mode, which transform the GDI coordination space
	we tried to implement MM_ANISOTROPIC, and found that the text looks worse than the native API

	recommendation for implementing MM_ANISOTROPIC:
	1. call GetViewportExtEx, GetViewportOrgEx, GetWindowExtEx and GetWindowOrgEx to get the new coordinations
	2. for all metrics come from GDI API, they are transform, while the outline metrics remain the same
	3. apply some multiplier to restore GDI metrics
	4. when drawing the bitmap back to DC, use StretchBlt
	yes, it is slow, ugly, but it works. hope you can find a better way
	*/
	if (GetMapMode(hdc) != MM_TEXT)
		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);
	
#ifdef _DEBUG
	const WCHAR *debug_text = NULL;
	//const WCHAR *debug_text = L"Connecting";
	const int start_index = 0;

	if (debug_text != NULL)
	{
		bool is_target = false;
		const size_t debug_len = wcslen(debug_text);

		if (options & ETO_GLYPH_INDEX)
		{
			WORD *gi = new WORD[debug_len];
			GetGlyphIndicesW(hdc, debug_text, debug_len, gi, 0);

			if (memcmp((WORD*) lpString + start_index, gi, sizeof(WORD) * debug_len) == 0)
				is_target = true;

			delete[] gi;
		}
		else
			is_target = (wcsncmp(lpString + start_index, debug_text, debug_len) == 0);

		if (is_target)
			bool break_now = true;
		else
			return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);
	}
#endif

	critical_section lock;

	if (!gdimm_text::instance().init(hdc, x, y, options))
		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

	if (!gdimm_text::instance().text_out(lpString, c, lprect, lpDx))
		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

	return TRUE;
}

void _gdimm_hook::install_hook(LPCTSTR lib_name, LPCSTR proc_name, void *hook_proc)
{
	NTSTATUS eh_error;

	// the target library module must have been loaded in this process before hooking
	const HMODULE h_lib = GetModuleHandle(lib_name);
	if (h_lib == NULL)
		return;

	TRACED_HOOK_HANDLE h_hook = new HOOK_TRACE_INFO();
	eh_error = LhInstallHook(GetProcAddress(h_lib, proc_name), hook_proc, NULL, h_hook);
	assert(eh_error == 0);

	ULONG thread_id_list = 0;
	eh_error = LhSetExclusiveACL(&thread_id_list, 0, h_hook);
	assert(eh_error == 0);

	_hooks.push_back(h_hook);
}

bool _gdimm_hook::hook()
{
	install_hook(TEXT("gdi32.dll"), "ExtTextOutW", ExtTextOutW_hook);
	return !(_hooks.empty());
}

void _gdimm_hook::disable_hook()
{
	NTSTATUS eh_error;

	for (vector<TRACED_HOOK_HANDLE>::const_iterator iter = _hooks.begin(); iter != _hooks.end(); iter++)
	{
		ULONG thread_id_list = 0;
		eh_error = LhSetInclusiveACL(&thread_id_list, 0, *iter);
		assert(eh_error == 0);
	}
}

void _gdimm_hook::unhook()
{
	NTSTATUS eh_error;
	
	eh_error = LhUninstallAllHooks();
	assert(eh_error == 0);

	eh_error = LhWaitForPendingRemovals();
	assert(eh_error == 0);

	for (vector<TRACED_HOOK_HANDLE>::const_iterator iter = _hooks.begin(); iter != _hooks.end(); iter++)
		delete *iter;
}