#pragma once

__gdi_entry BOOL WINAPI ExtTextOutW_hook(HDC hdc, int x, int y, UINT options, CONST RECT * lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx);

#if defined GDIPP_INJECT_SANDBOX && !defined _M_X64
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
	LPPROCESS_INFORMATION lpProcessInformation);
#endif // GDIPP_INJECT_SANDBOX && !_M_X64

BOOL
APIENTRY
GetTextExtentPoint32A_hook(
	HDC hdc,
	LPCSTR lpString,
	int c,
	LPSIZE psize
	);

BOOL
APIENTRY
GetTextExtentPoint32W_hook(
	HDC hdc,
	LPCWSTR lpString,
	int c,
	LPSIZE psize
	);

BOOL WINAPI GetTextExtentPointI_hook(HDC hdc, LPWORD pgiIn, int cgi, LPSIZE psize);

BOOL WINAPI AbortPath_hook(HDC hdc);
BOOL WINAPI BeginPath_hook(HDC hdc);
BOOL WINAPI EndPath_hook(HDC hdc);