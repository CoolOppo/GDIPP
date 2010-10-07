#include "stdafx.h"
#include "renderer.h"
#include "MurmurHash2_64.h"
#include "helper_func.h"
#include "gdimm.h"
#include "freetype.h"

gdimm_renderer::~gdimm_renderer()
{
}

bool gdimm_renderer::render(bool is_glyph_index, bool is_pdy, LPCWSTR lpString, UINT c, CONST INT *lpDx, glyph_run &new_glyph_run)
{
	return true;
}

bool gdimm_renderer::begin(const dc_context *context, FT_Render_Mode render_mode)
{
	_context = context;
	_render_mode = render_mode;

	_char_extra = GetTextCharacterExtra(_context->hdc);
	assert(_char_extra != 0x8000000);

	_font_trait = generate_font_trait(_context->log_font, render_mode);

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

	//static int total = 0;
	//static int cached = 0;

	b_ret = glyph_cache_instance.lookup_glyph_run(_font_trait, string_id, a_glyph_run);
	if (!b_ret)
	{
		// double-check lock
		gdimm_lock lock(LOCK_GLYPH_RUN_CACHE);

		b_ret = glyph_cache_instance.lookup_glyph_run(_font_trait, string_id, a_glyph_run);
		if (!b_ret)
		{
			const int glyph_run_height = render(is_glyph_index, is_pdy, lpString, c, lpDx, a_glyph_run);
			if (glyph_run_height == 0)
				return false;

			glyph_cache_instance.store_glyph_run(_font_trait, string_id, a_glyph_run);
		}
	}
	/*else
		cached += 1;
	total += 1;

	gdipp_debug_decimal(static_cast<double>(cached) / total);*/

	return true;
}