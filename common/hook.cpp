#include "stdafx.h"
#include "hook.h"
#include "global.h"
#include "text.h"
#include "fontlink.h"
#include <uxtheme.h>
#include <tlhelp32.h>

BOOL WINAPI ExtTextOutW_hook(HDC hdc, int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
	// draw non-TrueType fonts with original function
	if (!is_font_true_type(hdc))
		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

	// no text to render
	if (lpString == NULL || c == 0)
		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

	gdimm_text text(hdc);

	// assign clip rect
	if (((options & ETO_CLIPPED) == 0))
		text.clip_rect = NULL;
	else
	{
		if (lprect && ((lprect->right - lprect->left == 0) || (lprect->bottom - lprect->top == 0)))
			return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

		text.clip_rect = lprect;
	}

	// cursor position for the text
	text.cursor.x = x;
	text.cursor.y = y;

	if ((options & ETO_OPAQUE) != 0)
		text.draw_background(lprect);

	text.distances = lpDx;
	text.is_glyph_index = ((options & ETO_GLYPH_INDEX) != 0);
	
	int text_height = text.text_out(lpString, c);
	if (text_height != 0)
		return TRUE;
	else
		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);
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