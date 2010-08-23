#pragma once

#include "glyph_cache.h"

class gdimm_renderer
{
	virtual bool render(bool is_glyph_index, bool is_pdy, LPCWSTR lpString, UINT c, CONST INT *lpDx, glyph_run &new_glyph_run);

protected:
	static gdimm_glyph_cache _glyph_cache;

	int _char_extra;
	const dc_context *_context;
	uint64_t _font_trait;
	FT_Render_Mode _render_mode;

public:
	gdimm_renderer();
	virtual ~gdimm_renderer();

	virtual bool begin(const dc_context *context, FT_Render_Mode render_mode);
	virtual void end();
	bool fetch_glyph_run(bool is_glyph_index, bool is_pdy, LPCWSTR lpString, int c, CONST INT *lpDx, glyph_run &a_glyph_run);
};