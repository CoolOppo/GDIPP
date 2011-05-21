#pragma once

#include <vector>
#include "gdipp_svc/renderer.h"

using std::vector;

class gdimm_dw_renderer : public gdimm_renderer, public IDWriteTextRenderer
{
public:
	gdimm_dw_renderer();

	bool begin(const dc_context *context, FT_Render_Mode render_mode);

private:
	bool make_glyph_texture(FLOAT x, FLOAT y, const DWRITE_GLYPH_RUN *dw_glyph_run, glyph_run *a_glyph_run);
	bool render_glyph(LPCWSTR lpString, UINT c, glyph_run &new_glyph_run);
	bool render_text(LPCWSTR lpString, UINT c, glyph_run &new_glyph_run);
	bool render(bool is_glyph_index, bool is_pdy, LPCWSTR lpString, UINT c, CONST INT *lpDx, glyph_run &new_glyph_run);

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

	static IDWriteFactory *_dw_factory;
	static IDWriteGdiInterop *_dw_gdi_interop;

	vector<FLOAT> _advances;
	DWRITE_MEASURING_MODE _dw_measuring_mode;
	FLOAT _em_size;
	FLOAT _pixels_per_dip;
	bool _use_gdi_natural;
};
