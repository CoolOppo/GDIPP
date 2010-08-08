#include "stdafx.h"
#include "override.h"
#include "dw_renderer.h"
#include "ft_renderer.h"
#include "gdi_painter.h"
#include "gdimm.h"
#include "ggo_renderer.h"
#include "helper_def.h"
#include "helper_func.h"
#include "wic_renderer.h"
#include "wic_painter.h"

using namespace std;

set<HDC> hdc_with_path;

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

bool fetch_glyph_run(bool is_glyph_index, bool is_pdy, LPCWSTR lpString, UINT c, CONST INT *lpDx, const dc_context &context, glyph_run &a_glyph_run, GLYPH_CACHE_LEVEL &cache_level)
{
	bool b_ret;

	gdimm_renderer *renderer;
	switch (context.setting_cache->renderer)
	{
	case RENDERER_CLEARTYPE:
		return false;
	case RENDERER_GETGLYPHOUTLINE:
		renderer = new gdimm_ggo_renderer;
		cache_level = SINGLE_GLYPH;
		break;
	case RENDERER_DIRECTWRITE:
		renderer = new gdimm_dw_renderer;
		cache_level = GLYPH_RUN;
		break;
	case RENDERER_WIC:
		renderer = new gdimm_wic_renderer;
		cache_level = NONE;
		break;
	default:
		renderer = new gdimm_ft_renderer;
		cache_level = SINGLE_GLYPH;
		break;
	}

	b_ret = renderer->begin(&context);
	if (b_ret)
	{
		if (cache_level >= GLYPH_RUN)
			renderer->fetch_glyph_run(is_glyph_index, is_pdy, lpString, c, lpDx, a_glyph_run);

		renderer->end();
	}

	delete renderer;

	return b_ret;
}

BOOL WINAPI ExtTextOutW_hook(HDC hdc, int x, int y, UINT options, CONST RECT * lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
	bool b_ret;

//	if (options & ETO_GLYPH_INDEX)
//	if ((options & ETO_GLYPH_INDEX) == 0)
//	if (c == 1)
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
		const int debug_len = static_cast<const int>(wcslen(debug_text));

		if (options & ETO_GLYPH_INDEX)
		{
			WORD *gi = new WORD[debug_len];
			GetGlyphIndicesW(hdc, debug_text, debug_len, gi, 0);

			if (memcmp(reinterpret_cast<const WORD *>(lpString) + start_index, gi, sizeof(WORD) * debug_len) == 0)
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

	glyph_run a_glyph_run;
	GLYPH_CACHE_LEVEL cache_level;

	b_ret = fetch_glyph_run(!!(options & ETO_GLYPH_INDEX), !!(options & ETO_PDY), lpString, c, lpDx, context, a_glyph_run, cache_level);
	if (!b_ret)
		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

	gdimm_painter *painter;
	switch (context.setting_cache->renderer)
	{
	case RENDERER_CLEARTYPE:
		return ExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);
	case RENDERER_GETGLYPHOUTLINE:
		painter = new gdimm_gdi_painter;
		break;
	case RENDERER_DIRECTWRITE:
		painter = new gdimm_gdi_painter;
		break;
	case RENDERER_WIC:
		painter = new gdimm_wic_painter;
		break;
	default:
		painter = new gdimm_gdi_painter;
		break;
	}

	b_ret = painter->begin(&context);
	if (b_ret)
	{
		if (cache_level >= GLYPH_RUN)
			b_ret = painter->paint(x, y, options, lprect, &a_glyph_run, c, ((cache_level >= SINGLE_GLYPH) ? lpDx : NULL));
		else
			b_ret = painter->paint(x, y, options, lprect, lpString, c, lpDx);

		painter->end();
	}

	delete painter;

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
	dw_ret = GetModuleFileNameW(h_self, reinterpret_cast<wchar_t *>(inject_buffer + path_offset), MAX_PATH);
	assert(dw_ret != 0);

	// get eip of the spawned thread
	CONTEXT ctx = {};
	ctx.ContextFlags = CONTEXT_CONTROL;
	b_ret = GetThreadContext(lpProcessInformation->hThread, &ctx);
	assert(b_ret);

	LPVOID inject_base = VirtualAllocEx(lpProcessInformation->hProcess, NULL, sys_info.dwPageSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	assert(inject_base != NULL);

	register BYTE *p = inject_buffer;

#define emit_(t, x)	*reinterpret_cast<t* UNALIGNED>(p) = (t)(x); p += sizeof(t)
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
	ctx.Eip = reinterpret_cast<DWORD>(inject_base);
	b_ret = SetThreadContext(lpProcessInformation->hThread, &ctx);
	assert(b_ret);
}

BOOL
WINAPI
CreateProcessAsUserW_hook(
	HANDLE hToken,
	LPCWSTR lpApplicationName,
	LPWSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCWSTR lpCurrentDirectory,
	LPSTARTUPINFOW lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation)
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

bool GetTextExtentPoint32_hook(HDC hdc, bool is_glyph_index, LPCWSTR lpString, int c, LPSIZE psize)
{
	bool b_ret;

	if (lpString == NULL || psize == NULL || c == 0)
		return false;

	if (!is_valid_dc(hdc))
		return false;

	dc_context context;
	if (!context.init(hdc))
		return false;

	glyph_run a_glyph_run;
	GLYPH_CACHE_LEVEL cache_level;

	b_ret = fetch_glyph_run(is_glyph_index, false, lpString, c, NULL, context, a_glyph_run, cache_level);
	if (!b_ret)
		return false;
	
	if (cache_level < GLYPH_RUN)
		return false;

	psize->cx = get_glyph_run_width(a_glyph_run, false);
	psize->cy = context.outline_metrics->otmTextMetrics.tmHeight;

	for (glyph_run::const_iterator iter = a_glyph_run.begin(); iter != a_glyph_run.end(); iter++)
		psize->cy = max(psize->cy, iter->bbox.bottom - iter->bbox.top);

	return true;
}

BOOL
APIENTRY
GetTextExtentPoint32A_hook(
		HDC hdc,
		LPCSTR lpString,
		int c,
		LPSIZE psize
	)
{
	BOOL b_ret = FALSE;

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
	HDC hdc,
	LPCWSTR lpString,
	int c,
	LPSIZE psize
	)
{
	if (GetTextExtentPoint32_hook(hdc, false, lpString, c, psize))
		return TRUE;
	else
		return GetTextExtentPoint32W(hdc, lpString, c, psize);
}

BOOL WINAPI GetTextExtentPointI_hook(HDC hdc, LPWORD pgiIn, int cgi, LPSIZE psize)
{
	if (GetTextExtentPoint32_hook(hdc, true, (LPCWSTR) pgiIn, cgi, psize))
		return TRUE;
	else
		return GetTextExtentPointI(hdc, pgiIn, cgi, psize);
}

BOOL WINAPI AbortPath_hook(HDC hdc)
{
	BOOL b_ret = AbortPath(hdc);
	if (b_ret)
		hdc_with_path.erase(hdc);

	return b_ret;
}

BOOL WINAPI BeginPath_hook(HDC hdc)
{
	BOOL b_ret = BeginPath(hdc);
	if (b_ret)
		hdc_with_path.insert(hdc);

	return b_ret;
}

BOOL WINAPI EndPath_hook(HDC hdc)
{
	BOOL b_ret = EndPath(hdc);
	if (b_ret)
		hdc_with_path.erase(hdc);

	return b_ret;
}