#pragma once

#include "text.h"
using namespace std;

class gdimm_dw_text : public gdimm_text, private IDWriteTextRenderer
{
	CComPtr<IDWriteFactory> _dw_factory;
	CComPtr<IDWriteGdiInterop> _dw_gdi_interop;
	CComPtr<IDWriteRenderingParams> _dw_render_param;
	IDWriteBitmapRenderTarget *_dw_render_target;

	DWRITE_MEASURING_MODE _measuring_mode;
	bool _use_gdi_natural;
	vector<FLOAT> _advances;
	FLOAT _pixels_per_dip;

	UINT32 _cell_width;
	LONG _cell_height;
	LONG _extra_height;
	LONG _em_height;

	bool prepare_glyph(LPCWSTR lpString, UINT c, IDWriteFontFace **dw_font_face);
	bool prepare_text(LPCWSTR lpString, UINT c, IDWriteTextFormat **dw_text_format, IDWriteTextLayout **dw_text_layout);

	//////////////////////////////////////////////////////////////////////////

	IFACEMETHOD(QueryInterface)( 
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject);
	IFACEMETHOD_(ULONG, AddRef)( void);
	IFACEMETHOD_(ULONG, Release)( void);

	//////////////////////////////////////////////////////////////////////////

	IFACEMETHOD(IsPixelSnappingDisabled)(
		__maybenull void* clientDrawingContext,
		__out BOOL* isDisabled
		);
	IFACEMETHOD(GetCurrentTransform)(
		__maybenull void* clientDrawingContext,
		__out DWRITE_MATRIX* transform
		);
	IFACEMETHOD(GetPixelsPerDip)(
		__maybenull void* clientDrawingContext,
		__out FLOAT* pixelsPerDip
		);

	//////////////////////////////////////////////////////////////////////////

	IFACEMETHOD(DrawGlyphRun)(
		__maybenull void* clientDrawingContext,
		FLOAT baselineOriginX,
		FLOAT baselineOriginY,
		DWRITE_MEASURING_MODE measuringMode,
		__in DWRITE_GLYPH_RUN const* glyphRun,
		__in DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
		__maybenull IUnknown* clientDrawingEffect
		);
	IFACEMETHOD(DrawUnderline)(
		__maybenull void* clientDrawingContext,
		FLOAT baselineOriginX,
		FLOAT baselineOriginY,
		__in DWRITE_UNDERLINE const* underline,
		__maybenull IUnknown* clientDrawingEffect
		);
	IFACEMETHOD(DrawStrikethrough)(
		__maybenull void* clientDrawingContext,
		FLOAT baselineOriginX,
		FLOAT baselineOriginY,
		__in DWRITE_STRIKETHROUGH const* strikethrough,
		__maybenull IUnknown* clientDrawingEffect
		);
	IFACEMETHOD(DrawInlineObject)(
		__maybenull void* clientDrawingContext,
		FLOAT originX,
		FLOAT originY,
		IDWriteInlineObject* inlineObject,
		BOOL isSideways,
		BOOL isRightToLeft,
		__maybenull IUnknown* clientDrawingEffect
		);

public:
	gdimm_dw_text();
	bool begin(const gdimm_text_context *context);
	bool text_out(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx);
};