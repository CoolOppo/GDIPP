#include "stdafx.h"
#include "hook.h"
#include "text.h"
#include <tlhelp32.h>

__gdi_entry BOOL  WINAPI ExtTextOutW_hook( __in HDC hdc, __in int x, __in int y, __in UINT options, __in_opt CONST RECT * lprect, __in_ecount_opt(c) LPCWSTR lpString, __in UINT c, __in_ecount_opt(c) CONST INT * lpDx)
{
	//if ((options & ETO_GLYPH_INDEX) || c <= 2)
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
	//const WCHAR *debug_text = L"Fuck";
	const int start_index = 0;

	if (debug_text)
	{
		bool is_target = false;
		const size_t debug_len = wcslen(debug_text);

		if (options & ETO_GLYPH_INDEX)
		{
			WORD *gi = new WORD[debug_len];
			GetGlyphIndices(hdc, debug_text, debug_len, gi, 0);

			if (memcmp((WORD*) lpString + start_index, gi, sizeof(WORD) * debug_len) == 0)
				is_target = true;

			delete[] gi;
		}
		else
			is_target = (wcsncmp(lpString + start_index, debug_text, debug_len) == 0);

		if (!is_target)
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
	install_hook(TEXT("gdi32"), "ExtTextOutW", ExtTextOutW_hook);
	return !(_hooks.empty());
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