#include "stdafx.h"
#include "renderer.h"
#include <MurmurHash2.h>
#include <MurmurHash2_64.h>

gdimm_renderer::~gdimm_renderer()
{
}

bool gdimm_renderer::render(bool is_glyph_index, bool is_pdy, LPCWSTR lpString, UINT c, CONST INT *lpDx, glyph_run &new_glyph_run)
{
	return true;
}

unsigned int gdimm_renderer::generate_font_trait(const LOGFONTW *log_font, FT_Render_Mode render_mode)
{
	// the LOGFONTW structure and render mode are the minimal set that uniquely determine font metrics used by any renderer

	// exclude the bytes after the face name, which may contain junk data
	const int lf_metric_size = sizeof(LOGFONTW) - sizeof(log_font->lfFaceName);
	const int lf_facename_size = static_cast<const int>((wcslen(log_font->lfFaceName) * sizeof(wchar_t)));
	const int lf_total_size = lf_metric_size + lf_facename_size;

	return MurmurHash2(log_font, lf_total_size, render_mode);
}

bool gdimm_renderer::begin(const LOGFONT *log_font, FT_Render_Mode render_mode)
{
	_render_mode = render_mode;

	_font_trait = generate_font_trait(log_font, render_mode);

	return true;
}

void gdimm_renderer::end()
{
}

bool gdimm_renderer::fetch_glyph_run(bool is_glyph_index, bool is_pdy, LPCWSTR lpString, int c, CONST INT *lpDx, glyph_run &a_glyph_run)
{
	bool b_ret;

#ifdef _M_X64
	const unsigned __int64 string_id = MurmurHash64A(lpString, c * sizeof(WCHAR), is_glyph_index);
#else
	const unsigned __int64 string_id = MurmurHash64B(lpString, c * sizeof(WCHAR), is_glyph_index);
#endif // _M_X64

	//b_ret = glyph_cache_instance.lookup_glyph_run(_font_trait, string_id, a_glyph_run);
	b_ret = false;
	if (!b_ret)
	{
		const int glyph_run_height = render(is_glyph_index, is_pdy, lpString, c, lpDx, a_glyph_run);
		if (glyph_run_height == 0)
			return false;

		//glyph_cache_instance.store_glyph_run(_font_trait, string_id, a_glyph_run);
	}

	return true;
}