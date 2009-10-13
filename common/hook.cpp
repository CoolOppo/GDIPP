#include "stdafx.h"
#include <tlhelp32.h>
#include "text.h"

// declare the engine instance
gdimm_Text text_engine;

void DrawBackground(HDC hdc, CONST RECT * lprect)
{
	// get background rect geometry
	const LONG rect_width = lprect->right - lprect->left;
	const LONG rect_height = lprect->bottom - lprect->top;

	// create brush with background color
	COLORREF bg_color = GetBkColor(hdc);
	assert(bg_color != CLR_INVALID);
	HBRUSH bg_brush = CreateSolidBrush(bg_color);
	assert(bg_brush != NULL);

	// select new brush, and store old brush
	HBRUSH old_brush = (HBRUSH) SelectObject(hdc, bg_brush);

	// paint rect with brush
	BOOL ret = PatBlt(hdc, lprect->left, lprect->top, rect_width, rect_height, PATCOPY);
	assert(ret == TRUE);
	DeleteObject(bg_brush);

	// restore old brush
	SelectObject(hdc, old_brush);
}

// hooked ExtTextOutW
BOOL WINAPI ExtTextOutW_Hook(HDC hdc, int x, int y, UINT options, CONST RECT * lprect, LPCWSTR lpString, UINT c, CONST INT * lpDx)
{
	// indicator for "no further language-specific processing is required" (from MSDN)
	// no text is drawn
	if (options & ETO_GLYPH_INDEX)
		return ExtTextOut(hdc, x, y, options, lprect, lpString, c, lpDx);

	// draw non-TrueType fonts with original function
	TEXTMETRIC metrics;
	GetTextMetrics(hdc, &metrics);
	if (!(metrics.tmPitchAndFamily & TMPF_TRUETYPE))
		return ExtTextOut(hdc, x, y, options, lprect, lpString, c, lpDx);

	// cursor position for the text
	text_engine.cursor.x = x;
	text_engine.cursor.y = y;

	// assign clip rect
	if ((options & ETO_CLIPPED) == 0)
		text_engine.clip_rect = NULL;
	else
		text_engine.clip_rect = lprect;

	if ((options & ETO_OPAQUE) != 0)
		DrawBackground(hdc, lprect);

	text_engine.distances = lpDx;
	
	return text_engine.TextOut(hdc, lpString, c);
}

int
WINAPI
DrawTextExW_Hook(
			__in HDC hdc,
			__inout_ecount(cchText) LPWSTR lpchText,
			__in int cchText,
			__inout LPRECT lprc,
			__in UINT format,
			__in_opt LPDRAWTEXTPARAMS lpdtp)
{
	return DrawTextExW(hdc, lpchText, cchText, lprc, format, lpdtp);
}

// enumerate all the threads in the current process, except the excluded one
BOOL EnumThreads(DWORD *threadIds, DWORD *count, DWORD exclude = 0)
{
	// this procedure is routine

	THREADENTRY32 te32;
	te32.dwSize = sizeof(THREADENTRY32);

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
		return FALSE;

	BOOL ret = Thread32First(hSnapshot, &te32);
	if (ret == FALSE)
	{
		CloseHandle(hSnapshot);
		return FALSE;
	}

	const DWORD currProcId = GetCurrentProcessId();
	(*count) = 0;
	do
	{
		if (te32.th32OwnerProcessID == currProcId && te32.th32ThreadID != exclude)
		{
			if (threadIds)
				threadIds[*count] = te32.th32ThreadID;
			(*count)++;
		}
	} while (Thread32Next(hSnapshot, &te32));

	CloseHandle(hSnapshot);
	return TRUE;
}

void Hook()
{
	// gdi32.dll must have been loaded
	HMODULE hgdi32 = GetModuleHandle(TEXT("gdi32.dll"));
	assert(hgdi32 != NULL);
	// install hook with EasyHook
	TRACED_HOOK_HANDLE hHook_ExtTextOutW = new HOOK_TRACE_INFO();
	TRACED_HOOK_HANDLE hHook_TextOutW = new HOOK_TRACE_INFO();
	NTSTATUS ehError = LhInstallHook(GetProcAddress(hgdi32, "ExtTextOutW"), ExtTextOutW_Hook, NULL, hHook_ExtTextOutW);
	assert(ehError == 0);
	hgdi32 = GetModuleHandle(TEXT("user32.dll"));
	ehError = LhInstallHook(GetProcAddress(hgdi32, "DrawTextExW"), DrawTextExW_Hook, NULL, hHook_TextOutW);
	assert(ehError == 0);

	// enable hook in all threads
	DWORD threadCount;
	BOOL ret = EnumThreads(NULL, &threadCount);
	assert(ret == TRUE);
	DWORD *threads = new DWORD[threadCount];
	ret = EnumThreads(threads, &threadCount);
	assert(ret == TRUE);
	ehError = LhSetInclusiveACL(threads, threadCount, hHook_ExtTextOutW);
	ehError = LhSetInclusiveACL(threads, threadCount, hHook_TextOutW);
	assert(ehError == 0);
	delete[] threads;
}

// this procedure is used for EasyHook RhInjectLibrary()
// injection is created in a separate thread
// the injection thread does not need hooking
/*void Hook_Inject()
{
	HMODULE hgdi32 = GetModuleHandle(TEXT("gdi32.dll"));
	assert(hgdi32 != NULL);
	TRACED_HOOK_HANDLE hHook_ExtTextOutW = new HOOK_TRACE_INFO();
	NTSTATUS ehError = LhInstallHook(GetProcAddress(hgdi32, "ExtTextOutW"), ExtTextOutW_Hook, NULL, hHook_ExtTextOutW);
	assert(ehError == 0);

	const DWORD currThreadId = GetCurrentThreadId();
	DWORD threadCount;
	BOOL ret = EnumThreads(NULL, &threadCount, currThreadId);
	assert(ret == TRUE);
	DWORD *threads = new DWORD[threadCount];
	ret = EnumThreads(threads, &threadCount, currThreadId);
	assert(ret == TRUE);
	ehError = LhSetInclusiveACL(threads, threadCount, hHook_ExtTextOutW);
	assert(ehError == 0);
	delete[] threads;
}*/

// EasyHook unhook procedure
void Unhook()
{
	NTSTATUS ehError = LhUninstallAllHooks();
	assert(ehError == 0);
	ehError = LhWaitForPendingRemovals();
	assert(ehError == 0);
}