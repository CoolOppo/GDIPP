#include "stdafx.h"
#include "hook.h"
#include "text.h"
#include <tlhelp32.h>

__gdi_entry BOOL  WINAPI ExtTextOutW_hook( __in HDC hdc, __in int x, __in int y, __in UINT options, __in_opt CONST RECT * lprect, __in_ecount_opt(c) LPCWSTR lpString, __in UINT c, __in_ecount_opt(c) CONST INT * lpDx)
{
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

	if (options & ETO_OPAQUE)
		ExtTextOutW(hdc, x, y, ETO_OPAQUE, lprect, NULL, 0, NULL);

	if (!gdimm_text::instance().init(hdc, x, y, options))
		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

#ifdef _DEBUG
	const WCHAR *debug_text = NULL;//L"10%";

	if (debug_text)
	{
		bool break_now = false;
		if (options & ETO_GLYPH_INDEX)
		{
			const size_t debug_len = wcslen(debug_text);
			WORD *gi = new WORD[debug_len];
			GetGlyphIndices(hdc, debug_text, debug_len, gi, 0);

			if (memcmp((WORD*) lpString, gi, sizeof(WORD) * debug_len) == 0)
				break_now = true;
			delete[] gi;
		}
		else if (lstrcmp(lpString, debug_text) == 0)
			break_now = true;

		if (break_now)
			gdimm_text::instance().test = true;
	}
#endif

	if (!gdimm_text::instance().text_out(lpString, c, lprect, lpDx))
		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

	return TRUE;
}

__out_opt
HANDLE
WINAPI
CreateThread_hook(
    __in_opt  LPSECURITY_ATTRIBUTES lpThreadAttributes,
    __in      SIZE_T dwStackSize,
    __in      LPTHREAD_START_ROUTINE lpStartAddress,
    __in_opt  LPVOID lpParameter,
    __in      DWORD dwCreationFlags,
    __out_opt LPDWORD lpThreadId
    )
{
	HANDLE h_thread = CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);
	gdimm_hook::instance().hook();
	return h_thread;
}

bool operator<(const hook_info &hook1, const hook_info &hook2)
{
	return (memcmp(&hook1, &hook2, sizeof(hook_info)) < 0);
}

_gdimm_hook::_gdimm_hook()
{
	critical_section::initialize();
	_hooks[hook_info(TEXT("gdi32"), "ExtTextOutW", ExtTextOutW_hook)] = NULL;
	_hooks[hook_info(TEXT("kernel32"), "CreateThread", CreateThread_hook)] = NULL;
}

// enumerate all the threads in the current process, except the excluded one
bool _gdimm_hook::enum_threads(DWORD exclude, vector<DWORD> &thread_ids) const
{
	BOOL b_ret;

	THREADENTRY32 te32 = {0};
	te32.dwSize = sizeof(THREADENTRY32);

	const HANDLE h_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (h_snapshot == INVALID_HANDLE_VALUE)
		return false;

	b_ret = Thread32First(h_snapshot, &te32);
	if (b_ret == FALSE)
	{
		CloseHandle(h_snapshot);
		return false;
	}

	const DWORD curr_proc_id = GetCurrentProcessId();
	do
	{
		if (te32.th32OwnerProcessID == curr_proc_id && te32.th32ThreadID != exclude)
			thread_ids.push_back(te32.th32ThreadID);

	} while (Thread32Next(h_snapshot, &te32));

	CloseHandle(h_snapshot);
	return true;
}

void _gdimm_hook::install_hook(vector<DWORD> &thread_ids)
{
	NTSTATUS eh_error;

	for (map<const hook_info, TRACED_HOOK_HANDLE>::iterator iter = _hooks.begin(); iter != _hooks.end(); iter++)
	{
		// the target library module must have been loaded in this process before hooking
		const HMODULE h_lib = GetModuleHandle(iter->first.lib_name);
		if (h_lib == NULL)
			continue;

		TRACED_HOOK_HANDLE &h_hook = iter->second;
		if (h_hook == NULL)
		{
			h_hook = new HOOK_TRACE_INFO();
			
			eh_error = LhInstallHook(GetProcAddress(h_lib, iter->first.proc_name), iter->first.hook_proc, NULL, h_hook);
			assert(eh_error == 0);
		}

		// activate hook
		eh_error = LhSetInclusiveACL(&thread_ids[0], thread_ids.size(), h_hook);
		assert(eh_error == 0);
	}
}

void _gdimm_hook::hook()
{
	bool b_ret;

	// get target thread IDs in this process
	const int exclude_id = 0; //GetCurrentThreadId();
	vector<DWORD> thread_ids;
	b_ret = enum_threads(exclude_id, thread_ids);
	assert(b_ret);
	assert(!thread_ids.empty());

	install_hook(thread_ids);
}

void _gdimm_hook::unhook()
{
	NTSTATUS eh_error = LhUninstallAllHooks();
	assert(eh_error == 0);

	eh_error = LhWaitForPendingRemovals();
	assert(eh_error == 0);

	for (map<const hook_info, TRACED_HOOK_HANDLE>::const_iterator iter = _hooks.begin(); iter != _hooks.end(); iter++)
		delete iter->second;

	critical_section::release();
}