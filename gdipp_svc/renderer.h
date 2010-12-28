#pragma once

#include <rpc_api.h>

class gdimm_renderer
{
	virtual bool render(bool is_glyph_index, bool is_pdy, LPCWSTR lpString, UINT c, CONST INT *lpDx, glyph_run &new_glyph_run);

protected:
	unsigned int _font_trait;
	FT_Render_Mode _render_mode;

	unsigned int generate_font_trait(const LOGFONTW *log_font, FT_Render_Mode render_mode);

public:
	virtual ~gdimm_renderer();

	virtual bool begin(const LOGFONT *log_font, FT_Render_Mode render_mode);
	virtual void end();
	bool fetch_glyph_run(bool is_glyph_index, bool is_pdy, LPCWSTR lpString, int c, CONST INT *lpDx, glyph_run &a_glyph_run);
};