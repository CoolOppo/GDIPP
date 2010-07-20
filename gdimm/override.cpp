#include "stdafx.h"
#include "override.h"
#include "gdi_painter.h"
#include "gdimm.h"
#include "ggo_renderer.h"
#include "helper_def.h"
#include "helper_func.h"

using namespace std;

set<HDC> hdc_with_path;
gdimm_ggo_renderer _ggo;

bool is_valid_dc(HDC hdc)
{
	// probably a printer
	if (GetDeviceCaps(hdc, TECHNOLOGY) != DT_RASDISPLAY)
		return false;

	// the DC use another map mode, which transform the GDI coordination space
	// we tried to implement MM_ANISOTROPIC, and found that the text looks worse than the native API
	if (GetMapMode(hdc) != MM_TEXT)
		return false;

	/*
	if ExtTextOut is called within an open path bracket, different draw function is required
	because GDI renders the path outline pretty good, and path is rarely used (one example is Google Earth)
	gdipp does not render HDC with path
	*/
	if (hdc_with_path.find(hdc) != hdc_with_path.end())
		return false;

	return true;
}

__gdi_entry BOOL WINAPI ExtTextOutW_hook( __in HDC hdc, __in int x, __in int y, __in UINT options, __in_opt CONST RECT * lprect, __in_ecount_opt(c) LPCWSTR lpString, __in UINT c, __in_ecount_opt(c) CONST INT * lpDx)
{
	bool b_ret;

//	if (options & ETO_GLYPH_INDEX)
//	if ((options & ETO_GLYPH_INDEX) == 0)
//		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);
//	if (c != 3)
//		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

	// no text to output
	if (lpString == NULL || c == 0)
		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

	// rectangle is required but not specified
	// invalid call
	if (((options & ETO_OPAQUE) || (options & ETO_CLIPPED)) && (lprect == NULL))
		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);
	
	// completely clipped
	if ((options & ETO_CLIPPED) && IsRectEmpty(lprect))
		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

	if (!is_valid_dc(hdc))
		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

#ifdef _DEBUG
	const wchar_t *debug_text = NULL;
	//debug_text = L"";
	const int start_index = 0;

	if (debug_text != NULL)
	{
		bool is_target = false;
		const int debug_len = (int) wcslen(debug_text);

		if (options & ETO_GLYPH_INDEX)
		{
			WORD *gi = new WORD[debug_len];
			GetGlyphIndicesW(hdc, debug_text, debug_len, gi, 0);

			if (memcmp((WORD *)lpString + start_index, gi, sizeof(WORD) * debug_len) == 0)
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

	// uncomment this lock to make rendering single-threaded
	//gdimm_lock lock(LOCK_DEBUG);

	dc_context context;
	if (!context.init(hdc))
		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

	const glyph_run *glyph_run_ptr;
	int fetch_ret;
	switch (context.setting_cache->renderer)
	{
	case RENDERER_CLEARTYPE:
		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);
	case RENDERER_GETGLYPHOUTLINE:
		_ggo.begin(&context);
		fetch_ret = _ggo.fetch_glyph_run(lpString, c, !!(options & ETO_GLYPH_INDEX), glyph_run_ptr);
		_ggo.end();
		break;
	case RENDERER_DIRECTWRITE:
		break;
	case RENDERER_WIC:
		break;
	default:
		break;
	}

	if (fetch_ret != 0)
		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

	assert(glyph_run_ptr != NULL);

	gdimm_gdi_painter painter;
	b_ret = painter.begin(&context);
	
	if (b_ret)
	{
		b_ret = painter.paint(x, y, options, lprect, lpDx, glyph_run_ptr);
		painter.end();
	}

	if (!b_ret)
		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

	return TRUE;
}

#if defined GDIPP_INJECT_SANDBOX && !defined _M_X64
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
	dw_ret = GetModuleFileNameW(h_self, (wchar_t *)(inject_buffer + path_offset), MAX_PATH);
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
#endif // GDIPP_INJECT_SANDBOX && !_M_X64

BOOL
APIENTRY
GetTextExtentPoint32A_hook(
	__in HDC hdc,
	__in_ecount(c) LPCSTR lpString,
	__in int c,
	__out LPSIZE psize
	)
{
	BOOL b_ret = FALSE;

	if (lpString == NULL || psize == NULL || c == 0)
		return GetTextExtentPoint32A(hdc, lpString, c, psize);

	if (!is_valid_dc(hdc))
		return GetTextExtentPoint32A(hdc, lpString, c, psize);

	int wc_str_len = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lpString, c, NULL, 0);
	if (wc_str_len == 0)
		return GetTextExtentPoint32A(hdc, lpString, c, psize);

	LPWSTR wc_str = new WCHAR[wc_str_len];
	wc_str_len = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lpString, c, wc_str, wc_str_len);
	if (wc_str_len != 0)
		b_ret = GetTextExtentPoint32W_hook(hdc, wc_str, wc_str_len, psize);

	delete[] wc_str;

	if (b_ret)
		return TRUE;
	else
		return GetTextExtentPoint32A(hdc, lpString, c, psize);
}

BOOL
APIENTRY
GetTextExtentPoint32W_hook(
	__in HDC hdc,
	__in_ecount(c) LPCWSTR lpString,
	__in int c,
	__out LPSIZE psize
	)
{
	if (lpString == NULL || psize == NULL || c == 0)
		return GetTextExtentPoint32W(hdc, lpString, c, psize);

	if (!is_valid_dc(hdc))
		return GetTextExtentPoint32W(hdc, lpString, c, psize);

	dc_context context;
	if (!context.init(hdc))
		return GetTextExtentPoint32W(hdc, lpString, c, psize);

	const glyph_run *glyph_run_ptr;
	int fetch_ret;
	switch (context.setting_cache->renderer)
	{
	case RENDERER_CLEARTYPE:
		return GetTextExtentPoint32W(hdc, lpString, c, psize);
	case RENDERER_GETGLYPHOUTLINE:
		_ggo.begin(&context);
		fetch_ret = _ggo.fetch_glyph_run(lpString, c, false, glyph_run_ptr);
		_ggo.end();
		break;
	case RENDERER_DIRECTWRITE:
		break;
	case RENDERER_WIC:
		break;
	default:
		break;
	}

	const RECT glyph_run_rect = get_glyph_run_rect(glyph_run_ptr);
	psize->cx = glyph_run_rect.right - glyph_run_rect.left;
	psize->cy = max(glyph_run_rect.bottom - glyph_run_rect.top, context.outline_metrics->otmTextMetrics.tmHeight);

	return TRUE;
}

BOOL WINAPI GetTextExtentPointI_hook(__in HDC hdc, __in_ecount(cgi) LPWORD pgiIn, __in int cgi, __out LPSIZE psize)
{
	if (pgiIn == NULL || psize == NULL || cgi == 0)
		return GetTextExtentPointI(hdc, pgiIn, cgi, psize);

	if (!is_valid_dc(hdc))
		return GetTextExtentPointI(hdc, pgiIn, cgi, psize);

	dc_context context;
	if (!context.init(hdc))
		return GetTextExtentPointI(hdc, pgiIn, cgi, psize);

	const glyph_run *glyph_run_ptr;
	int fetch_ret;
	switch (context.setting_cache->renderer)
	{
	case RENDERER_CLEARTYPE:
		return GetTextExtentPointI(hdc, pgiIn, cgi, psize);
	case RENDERER_GETGLYPHOUTLINE:
		_ggo.begin(&context);
		fetch_ret = _ggo.fetch_glyph_run((LPCWSTR) pgiIn, cgi, true, glyph_run_ptr);
		_ggo.end();
		break;
	case RENDERER_DIRECTWRITE:
		break;
	case RENDERER_WIC:
		break;
	default:
		break;
	}

	const RECT glyph_run_rect = get_glyph_run_rect(glyph_run_ptr);
	psize->cx = glyph_run_rect.right - glyph_run_rect.left;
	psize->cy = max(glyph_run_rect.bottom - glyph_run_rect.top, context.outline_metrics->otmTextMetrics.tmHeight);

	return TRUE;
}

BOOL WINAPI AbortPath_hook(__in HDC hdc)
{
	BOOL b_ret = AbortPath(hdc);
	if (b_ret)
		hdc_with_path.erase(hdc);

	return b_ret;
}

BOOL WINAPI BeginPath_hook(__in HDC hdc)
{
	BOOL b_ret = BeginPath(hdc);
	if (b_ret)
		hdc_with_path.insert(hdc);

	return b_ret;
}

BOOL WINAPI EndPath_hook(__in HDC hdc)
{
	BOOL b_ret = EndPath(hdc);
	if (b_ret)
		hdc_with_path.erase(hdc);

	return b_ret;
}