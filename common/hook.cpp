#include "stdafx.h"
#include "hook.h"
#include "global.h"
#include "text.h"
#include "font_link.h"
#include <tlhelp32.h>

BOOL WINAPI ExtTextOutW_hook(HDC hdc, int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
	//if ((options & ETO_GLYPH_INDEX) == ETO_GLYPH_INDEX || c < 10)
	//	return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

	// no text to render
	if (lpString == NULL || c == 0)
		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

	if ((options & ETO_CLIPPED) == ETO_CLIPPED)
	{
		assert(lprect != NULL);
		
		// completely clipped
		if ((lprect->right == lprect->left) || (lprect->bottom == lprect->top))
			return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);
	}

	if (!gdimm_text::instance().init(hdc, x, y))
		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

	if ((options & ETO_OPAQUE) == ETO_OPAQUE)
	{
		BOOL b_ret = ExtTextOutW(hdc, x, y, ETO_OPAQUE, lprect, NULL, 0, lpDx);
		assert(b_ret);
	}

	gdimm_text::instance().text_out(lpString, c, lprect, lpDx, options & ETO_GLYPH_INDEX, options & ETO_PDY);
	//return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);
	return TRUE;
}

// enumerate all the threads in the current process, except the excluded one
BOOL _gdimm_hook::enum_threads(DWORD *thread_ids, DWORD *count, DWORD exclude) const
{
	THREADENTRY32 te32 = {0};
	te32.dwSize = sizeof(THREADENTRY32);

	const HANDLE h_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (h_snapshot == INVALID_HANDLE_VALUE)
		return FALSE;

	BOOL b_ret = Thread32First(h_snapshot, &te32);
	if (b_ret == FALSE)
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
	
	const HMODULE h_lib = GetModuleHandle(lib_name);
	if (h_lib == NULL)
		return false;

	// install hook with EasyHook
	const TRACED_HOOK_HANDLE h_hook = new HOOK_TRACE_INFO();
	NTSTATUS eh_error = LhInstallHook(GetProcAddress(h_lib, proc_name), hook_proc, NULL, h_hook);
	assert(eh_error == 0);

	eh_error = LhSetInclusiveACL(_thread_ids, _thread_count, h_hook);
	assert(eh_error == 0);

	_hook_handles.push_back(h_hook);

	return true;
}

void _gdimm_hook::hook()
{
	// get all threads in this process
	const int exclude_thr = 0; //GetCurrentThreadId();
	BOOL b_ret = enum_threads(NULL, &_thread_count, exclude_thr);
	assert(b_ret);

	_thread_ids = new DWORD[_thread_count];
	b_ret = enum_threads(_thread_ids, &_thread_count, exclude_thr);
	assert(b_ret);

	install_hook(TEXT("gdi32"), "ExtTextOutW", ExtTextOutW_hook);
}

// EasyHook unhook procedure
void _gdimm_hook::unhook() const
{
	delete[] _thread_ids;

	NTSTATUS eh_error = LhUninstallAllHooks();
	assert(eh_error == 0);
	eh_error = LhWaitForPendingRemovals();
	assert(eh_error == 0);

	for (vector<TRACED_HOOK_HANDLE>::const_iterator iter = _hook_handles.begin(); iter != _hook_handles.end(); iter++)
		delete *iter;
}