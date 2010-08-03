#pragma once

#include "glyph_cache.h"

class gdimm_renderer
{
	virtual bool render(LPCWSTR lpString, UINT c, bool is_glyph_index, CONST INT *lpDx, bool is_pdy, glyph_run &a_glyph_run) = 0;

protected:
	static gdimm_glyph_cache _glyph_cache;

	int _char_extra;
	const dc_context *_context;

public:
	virtual ~gdimm_renderer();

	virtual bool begin(const dc_context *context);
	virtual void end();
	void fetch_glyph_run(LPCWSTR lpString, int c, bool is_glyph_index, CONST INT *lpDx, bool is_pdy, glyph_run &a_glyph_run);
};