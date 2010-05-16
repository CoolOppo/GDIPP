#pragma once

#include "text.h"
#include "dw_renderer.h"

using namespace std;

class gdimm_dw_text : public gdimm_text
{
	static CComPtr<IDWriteFactory> _dw_factory;
	static CComPtr<IDWriteGdiInterop> _dw_gdi_interop;
	static CComPtr<IDWriteRenderingParams> _dw_render_param;
	static CComPtr<gdimm_dw_renderer> _dw_renderer;

	DWRITE_MEASURING_MODE _measuring_mode;
	vector<FLOAT> _advances;

	bool draw_glyph(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c);
	bool draw_text(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c);

public:
	bool begin(HDC hdc, const OUTLINETEXTMETRICW *outline_metrics, const wchar_t *font_face, const font_setting_cache *setting_cache);
	bool text_out(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx);
};