#include "stdafx.h"
#include "text.h"
#include "lock.h"

HANDLE h_svc_event = NULL;
HANDLE h_unload = NULL;

unsigned __stdcall unload_self(void *arglist)
{
	BOOL b_ret;

	gdimm_lock lock(LOCK_HOOK);
	FreeLibraryAndExitThread(h_self, 0);

	b_ret = RedrawWindow(NULL, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
	assert(b_ret);

	_endthreadex(0);
	return 0;
}

__gdi_entry BOOL WINAPI ExtTextOutW_hook( __in HDC hdc, __in int x, __in int y, __in UINT options, __in_opt CONST RECT * lprect, __in_ecount_opt(c) LPCWSTR lpString, __in UINT c, __in_ecount_opt(c) CONST INT * lpDx)
{
	// if injected by service, check if the service event is set (service process terminates)
	if (h_svc_event != NULL && WaitForSingleObject(h_svc_event, 0) != WAIT_TIMEOUT)
	{
		// double-check lock
		gdimm_lock lock(LOCK_HOOK);

		if (h_svc_event != NULL)
		{
			CloseHandle(h_svc_event);
			h_svc_event = NULL;

			// use critical section to ensure FreeLibraryAndExitThread is called after this hooked API call
			h_unload = (HANDLE) _beginthreadex(NULL, 0, unload_self, NULL, 0, NULL);
		}

		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);
	}

	//if ((options & ETO_GLYPH_INDEX))
	//if (c < 10)
	//	return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

	// no text to render
	if (lpString == NULL || c == 0)
		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

	// rectangle is required but not specified
	// invalid call
	if (((options & ETO_OPAQUE) || (options & ETO_CLIPPED)) && (lprect == NULL))
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

	note for implementing MM_ANISOTROPIC:
	1. call GetViewportExtEx, GetViewportOrgEx, GetWindowExtEx and GetWindowOrgEx to get the new coordinations
	2. for all metrics come from GDI API, they are transform, while the outline metrics remain the same
	3. apply some multiplier to restore GDI metrics
	4. when drawing the bitmap back to DC, use StretchBlt
	yes, it is slow, ugly, but it works. hope you can find a better way
	*/
	if (GetMapMode(hdc) != MM_TEXT)
		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);
	
#ifdef _DEBUG
	const wchar_t *debug_text = NULL;
	//debug_text = L"Oblique";
	const int start_index = 0;

	if (debug_text != NULL)
	{
		bool is_target = false;
		const int debug_len = (int) wcslen(debug_text);

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
#endif // _DEBUG

	//gdimm_lock lock(LOCK_DEBUG);

	gdimm_text text_instance;

	if (!text_instance.init(hdc))
		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

	if (!text_instance.text_out(x, y, options, lprect, lpString, c, lpDx))
		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

	return TRUE;
}

#ifndef _M_X64
void inject_at_eip(LPPROCESS_INFORMATION lpProcessInformation)
{
	BOOL b_ret;
	DWORD dw_ret;

	// alloc buffer for the injection data
	// the minimum allocation unit is page
	SYSTEM_INFO sys_info;
	GetSystemInfo(&sys_info);
	BYTE *inject_buffer = new BYTE[sys_info.dwPageSize];
	memset(inject_buffer, 0xcc, sys_info.dwPageSize);

	// put gdimm path at the end of the buffer, leave space at the beginning for code
	const DWORD path_offset = sys_info.dwPageSize - MAX_PATH * sizeof(wchar_t);
	dw_ret = GetModuleFileNameW(h_self, (wchar_t*)(inject_buffer + path_offset), MAX_PATH);
	assert(dw_ret != 0);

	// get eip of the spawned thread
	CONTEXT ctx = {};
	ctx.ContextFlags = CONTEXT_CONTROL;
	b_ret = GetThreadContext(lpProcessInformation->hThread, &ctx);
	assert(b_ret);

	LPVOID inject_base = VirtualAllocEx(lpProcessInformation->hProcess, NULL, sys_info.dwPageSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	assert(inject_base != NULL);

	register BYTE *p = inject_buffer;

#define emit_(t, x)	*(t* UNALIGNED) p = (t)(x); p += sizeof(t)
#define emit_db(b)	emit_(BYTE, b)
#define emit_dw(w)	emit_(WORD, w)
#define emit_dd(d)	emit_(DWORD, d)

	emit_db(0x50);		// push eax

	emit_db(0x68);		// push gdimm_path
	emit_dd((DWORD) inject_base + path_offset);
	emit_db(0xB8);		// mov eax, LoadLibraryW
	emit_dd(LoadLibraryW);
	emit_dw(0xD0FF);	// call eax

	emit_db(0x58);		// pop eax -> LoadLibraryW has return value

	emit_db(0x68);		// push original_eip
	emit_dd(ctx.Eip);
	emit_db(0xC3);		// retn -> serve as an absolute jmp

	// write injection data to target process space
	b_ret = WriteProcessMemory(lpProcessInformation->hProcess, inject_base, inject_buffer, sys_info.dwPageSize, NULL);
	assert(b_ret);

	delete[] inject_buffer;

	// notify code change
	b_ret = FlushInstructionCache(lpProcessInformation->hProcess, inject_base, sys_info.dwPageSize);
	assert(b_ret);

	// set eip to the entry point of the injection code
	ctx.Eip = (DWORD) inject_base;
	b_ret = SetThreadContext(lpProcessInformation->hThread, &ctx);
	assert(b_ret);
}

BOOL
WINAPI
CreateProcessAsUserW_hook(
	__in_opt    HANDLE hToken,
	__in_opt    LPCWSTR lpApplicationName,
	__inout_opt LPWSTR lpCommandLine,
	__in_opt    LPSECURITY_ATTRIBUTES lpProcessAttributes,
	__in_opt    LPSECURITY_ATTRIBUTES lpThreadAttributes,
	__in        BOOL bInheritHandles,
	__in        DWORD dwCreationFlags,
	__in_opt    LPVOID lpEnvironment,
	__in_opt    LPCWSTR lpCurrentDirectory,
	__in        LPSTARTUPINFOW lpStartupInfo,
	__out       LPPROCESS_INFORMATION lpProcessInformation)
{
	// if the token is not restricted, redirect the call to original API
	// service can inject
	if (!IsTokenRestricted(hToken))
	{
		return CreateProcessAsUserW(
			hToken,
			lpApplicationName,
			lpCommandLine,
			lpProcessAttributes,
			lpThreadAttributes,
			bInheritHandles,
			dwCreationFlags,
			lpEnvironment,
			lpCurrentDirectory,
			lpStartupInfo,
			lpProcessInformation);
	}

	// otherwise, the spawned process is restricted, and service cannot inject

	// injection at EIP requires the process be suspended
	// if CREATE_SUSPENDED is not specified in the creation flag, remember to resume process after injection
	bool is_suspended;
	if (dwCreationFlags & CREATE_SUSPENDED)
		is_suspended = true;
	else
	{
		is_suspended = false;
		dwCreationFlags |= CREATE_SUSPENDED;
	}

	if (!CreateProcessAsUserW(
		hToken,
		lpApplicationName,
		lpCommandLine,
		lpProcessAttributes,
		lpThreadAttributes,
		bInheritHandles,
		dwCreationFlags,
		lpEnvironment,
		lpCurrentDirectory,
		lpStartupInfo,
		lpProcessInformation))
		return FALSE;

	// since the spawned process can be restricted, EasyHook may not work
	// we inject LoadLibrary call at the entry point of the spawned thread
	inject_at_eip(lpProcessInformation);

	if (!is_suspended)
	{
		DWORD dw_ret = ResumeThread(lpProcessInformation->hThread);
		assert(dw_ret != -1);
	}

	return TRUE;
}
#endif // _M_X64

EXTERN_C __declspec(dllexport) void __stdcall NativeInjectionEntryPoint(REMOTE_ENTRY_INFO* remote_info)
{
	BOOL b_ret;

	if (remote_info->UserDataSize != sizeof(gdipp_inject_payload))
		return;

	const gdipp_inject_payload payload = *(gdipp_inject_payload*) remote_info->UserData;
	switch (payload.inject_type)
	{
	case GDIPP_SERVICE:
		h_svc_event = OpenEventW(SYNCHRONIZE, FALSE, payload.svc_event_name);

		// force the foreground window of the injected process to redraw
		b_ret = RedrawWindow(NULL, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
		assert(b_ret);

		break;
	case GDIPP_LOADER:
		// wake up suspended process
		RhWakeUpProcess();
		break;
	}
}

bool gdimm_hook::install_hook(LPCTSTR lib_name, LPCSTR proc_name, void *hook_proc)
{
	NTSTATUS eh_error;

	// the target library module must have been loaded in this process before hooking
	const HMODULE h_lib = GetModuleHandle(lib_name);
	if (h_lib == NULL)
		return false;

	TRACED_HOOK_HANDLE h_hook = new HOOK_TRACE_INFO();
	eh_error = LhInstallHook(GetProcAddress(h_lib, proc_name), hook_proc, NULL, h_hook);
	assert(eh_error == 0);

	ULONG thread_id_list[1] = {};
 	eh_error = LhSetExclusiveACL(thread_id_list, 0, h_hook);
 	assert(eh_error == 0);

	_hooks.push_back(h_hook);

	return true;
}

bool gdimm_hook::hook()
{
	install_hook(TEXT("gdi32.dll"), "ExtTextOutW", ExtTextOutW_hook);

#ifndef _M_X64
	// currently not support inject at EIP for 64-bit processes
	install_hook(TEXT("advapi32.dll"), "CreateProcessAsUserW", CreateProcessAsUserW_hook);
#endif // _M_X64

	return !(_hooks.empty());
}

void gdimm_hook::unhook()
{
	NTSTATUS eh_error;

	if (h_unload != NULL)
		CloseHandle(h_unload);
	
	eh_error = LhUninstallAllHooks();
	assert(eh_error == 0);

	eh_error = LhWaitForPendingRemovals();
	assert(eh_error == 0);

	for (vector<TRACED_HOOK_HANDLE>::const_iterator iter = _hooks.begin(); iter != _hooks.end(); iter++)
		delete *iter;
}