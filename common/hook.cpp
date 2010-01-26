#include "stdafx.h"
#include "hook.h"
#include "global.h"
#include "text.h"
#include "font_chg.h"
#include <tlhelp32.h>

BOOL WINAPI ExtTextOutW_hook(HDC hdc, int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
	// draw non-TrueType fonts with original function
	if (!is_font_true_type(hdc))
		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

	// no text to render
	if (lpString == NULL || c == 0)
		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

	if ((options & ETO_CLIPPED) != 0)
	{
		assert(lprect != NULL);
		
		// completely clipped
		if ((lprect->right == lprect->left) || (lprect->bottom == lprect->top))
			return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);
	}

	gdimm_text text(hdc, x, y, lprect, lpDx);

	if ((options & ETO_OPAQUE) != 0)
		text.draw_background();

	const WCHAR *str = lpString;
	WCHAR *glyph_indices = NULL;

	text.prepare();

	if ((options & ETO_GLYPH_INDEX) == 0)
	{
		glyph_indices = new WCHAR[c];

		if (text.to_glyph_indices(lpString, c, glyph_indices))
			str = glyph_indices;
		else
		{
			delete[] glyph_indices;
			return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);
		}
	}

	text.text_out(str, c);

	if (glyph_indices != NULL)
		delete[] glyph_indices;

	return TRUE;
}

// enumerate all the threads in the current process, except the excluded one
BOOL _gdimm_hook::enum_threads(DWORD *thread_ids, DWORD *count, DWORD exclude) const
{
	THREADENTRY32 te32;
	te32.dwSize = sizeof(THREADENTRY32);

	HANDLE h_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (h_snapshot == INVALID_HANDLE_VALUE)
		return FALSE;

	BOOL success = Thread32First(h_snapshot, &te32);
	if (success == FALSE)
	{
		CloseHandle(h_snapshot);
		return FALSE;
	}

	const DWORD curr_proc_id = GetCurrentProcessId();
	(*count) = 0;
	do
	{
		if (te32.th32OwnerProcessID == curr_proc_id && te32.th32ThreadID != exclude)
		{
			if (thread_ids)
				thread_ids[*count] = te32.th32ThreadID;
			(*count)++;
		}
	} while (Thread32Next(h_snapshot, &te32));

	CloseHandle(h_snapshot);
	return TRUE;
}

bool _gdimm_hook::install_hook(LPCTSTR lib_name, LPCSTR proc_name, void *hook_proc)
{
	// pre-condition: the dll file <lib_name> must have been loaded in this process
	
	HMODULE h_lib = GetModuleHandle(lib_name);
	if (h_lib == NULL)
		return false;

	// install hook with EasyHook
	TRACED_HOOK_HANDLE h_hook = new HOOK_TRACE_INFO();
	NTSTATUS eh_error = LhInstallHook(GetProcAddress(h_lib, proc_name), hook_proc, NULL, h_hook);
	assert(eh_error == 0);

	eh_error = LhSetInclusiveACL(threads, thread_count, h_hook);
	assert(eh_error == 0);

	hook_handles.push_back(h_hook);

	return true;
}

void _gdimm_hook::hook()
{
	// get all threads in this process
	int exclude_thr = 0; //GetCurrentThreadId();
	BOOL ret = enum_threads(NULL, &thread_count, exclude_thr);
	assert(ret == TRUE);

	threads = new DWORD[thread_count];
	ret = enum_threads(threads, &thread_count, exclude_thr);
	assert(ret == TRUE);

	install_hook(TEXT("gdi32"), "ExtTextOutW", ExtTextOutW_hook);
}

// EasyHook unhook procedure
void _gdimm_hook::unhook() const
{
	delete[] threads;

	NTSTATUS eh_error = LhUninstallAllHooks();
	assert(eh_error == 0);
	eh_error = LhWaitForPendingRemovals();
	assert(eh_error == 0);

	for (vector<TRACED_HOOK_HANDLE>::const_iterator iter = hook_handles.begin(); iter != hook_handles.end(); iter++)
		delete *iter;
}