#pragma once

extern const wchar_t *debug_text;
bool is_target_text(HDC hdc, bool is_glyph_index, LPCWSTR lpString, const wchar_t *target_text, int start_index = 0);
bool get_text_extent(HDC hdc, LPCWSTR lpString, int count, LPSIZE lpSize, bool is_glyph_index, int nMaxExtent = 0, LPINT lpnFit = NULL, LPINT lpnDx = NULL);

BOOL WINAPI ExtTextOutW_hook(HDC hdc, int x, int y, UINT options, CONST RECT * lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx);

int WINAPI DrawTextExA_hook(HDC hdc, LPSTR lpchText, int cchText, LPRECT lprc, UINT format, LPDRAWTEXTPARAMS lpdtp);
int WINAPI DrawTextExW_hook(HDC hdc, LPWSTR lpchText, int cchText, LPRECT lprc, UINT format, LPDRAWTEXTPARAMS lpdtp);

BOOL APIENTRY GetTextExtentPoint32A_hook(HDC hdc, LPCSTR lpString, int c, LPSIZE lpSize);
BOOL APIENTRY GetTextExtentPoint32W_hook(HDC hdc, LPCWSTR lpString, int c, LPSIZE lpSize);
BOOL WINAPI GetTextExtentPointI_hook(HDC hdc, LPWORD pgiIn, int cgi, LPSIZE lpSize);

BOOL APIENTRY GetTextExtentExPointA_hook(HDC hdc, LPCSTR lpszString, int cchString, int nMaxExtent, LPINT lpnFit, LPINT lpnDx, LPSIZE lpSize);
BOOL APIENTRY GetTextExtentExPointW_hook(HDC hdc, LPCWSTR lpszString, int cchString, int nMaxExtent, LPINT lpnFit, LPINT lpnDx, LPSIZE lpSize);
BOOL WINAPI GetTextExtentExPointI_hook(HDC hdc, LPWORD lpwszString, int cwchString, int nMaxExtent, LPINT lpnFit, LPINT lpnDx, LPSIZE lpSize);

DWORD WINAPI GetGlyphOutlineA_hook(HDC hdc, UINT uChar, UINT fuFormat, LPGLYPHMETRICS lpgm, DWORD cjBuffer, LPVOID pvBuffer, CONST MAT2 *lpmat2);
DWORD WINAPI GetGlyphOutlineW_hook(HDC hdc, UINT uChar, UINT fuFormat, LPGLYPHMETRICS lpgm, DWORD cjBuffer, LPVOID pvBuffer, CONST MAT2 *lpmat2);

BOOL WINAPI AbortPath_hook(HDC hdc);
BOOL WINAPI BeginPath_hook(HDC hdc);
BOOL WINAPI EndPath_hook(HDC hdc);

HRESULT WINAPI ScriptPlace_hook(
	HDC                  hdc,        // In    Optional (see under caching)
	SCRIPT_CACHE         *psc,       // InOut Cache handle
	const WORD           *pwGlyphs,  // In    Glyph buffer from prior ScriptShape call
	int                  cGlyphs,    // In    Number of glyphs
	const SCRIPT_VISATTR *psva,      // In    Visual glyph attributes
	SCRIPT_ANALYSIS      *psa,       // InOut Result of ScriptItemize (may have fNoGlyphIndex set)
	int                  *piAdvance, // Out   Advance wdiths
	GOFFSET              *pGoffset,  // Out   x,y offset for combining glyph
	ABC                  *pABC);     // Out   Composite ABC for the whole run (Optional)

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