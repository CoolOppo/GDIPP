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
	
	critical_section lock;

	if (!gdimm_text::instance().init(hdc, x, y, options))
		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

#ifdef _DEBUG
	const WCHAR *debug_text = NULL;//L"Clear";
	const int start_index = 0;

	if (debug_text)
	{
		bool break_now = false;
		const size_t debug_len = wcslen(debug_text);

		if (options & ETO_GLYPH_INDEX)
		{
			WORD *gi = new WORD[debug_len];
			GetGlyphIndices(hdc, debug_text, debug_len, gi, 0);

			if (memcmp((WORD*) lpString + start_index, gi, sizeof(WORD) * debug_len) == 0)
				break_now = true;
			delete[] gi;
		}
		else if (wcsncmp(lpString + start_index, debug_text, debug_len) == 0)
			break_now = true;

		if (break_now)
			gdimm_text::instance().test = true;
	}
#endif

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

void _gdimm_hook::hook()
{
	critical_section::initialize();

	install_hook(TEXT("gdi32"), "ExtTextOutW", ExtTextOutW_hook);
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

	critical_section::release();
}