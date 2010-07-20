#pragma once

__gdi_entry BOOL WINAPI ExtTextOutW_hook( __in HDC hdc, __in int x, __in int y, __in UINT options, __in_opt CONST RECT * lprect, __in_ecount_opt(c) LPCWSTR lpString, __in UINT c, __in_ecount_opt(c) CONST INT * lpDx);

#if defined GDIPP_INJECT_SANDBOX && !defined _M_X64
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
	__out       LPPROCESS_INFORMATION lpProcessInformation);
#endif // GDIPP_INJECT_SANDBOX && !_M_X64

BOOL
APIENTRY
GetTextExtentPoint32A_hook(
	__in HDC hdc,
	__in_ecount(c) LPCSTR lpString,
	__in int c,
	__out LPSIZE psize
	);

BOOL
APIENTRY
GetTextExtentPoint32W_hook(
	__in HDC hdc,
	__in_ecount(c) LPCWSTR lpString,
	__in int c,
	__out LPSIZE psize
	);

BOOL WINAPI GetTextExtentPointI_hook(__in HDC hdc, __in_ecount(cgi) LPWORD pgiIn, __in int cgi, __out LPSIZE psize);

BOOL WINAPI AbortPath_hook(__in HDC hdc);
BOOL WINAPI BeginPath_hook(__in HDC hdc);
BOOL WINAPI EndPath_hook(__in HDC hdc);