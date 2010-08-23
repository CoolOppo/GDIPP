#include "stdafx.h"
#include "renderer.h"
#include "freetype.h"

gdimm_glyph_cache gdimm_renderer::_glyph_cache;

gdimm_renderer::gdimm_renderer()
{
	_glyph_cache._glyph_run_lru.resize(ft_cache_max_faces * ft_cache_max_sizes);
}

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

	_font_trait = get_font_trait(_context->log_font, render_mode);

	return true;
}

void gdimm_renderer::end()
{
}

bool gdimm_renderer::fetch_glyph_run(bool is_glyph_index, bool is_pdy, LPCWSTR lpString, int c, CONST INT *lpDx, glyph_run &a_glyph_run)
{
	bool b_ret;

	uint64_t erased_trait;
	const bool overflow = _glyph_cache._glyph_run_lru.access(_font_trait, erased_trait);
	if (overflow)
	{
		// erasing font trait may fail, in case that no glyph was successfully rendered
		b_ret = _glyph_cache.erase_font_trait(erased_trait);
	}

#ifdef _M_X64
	const uint64_t str_hash = MurmurHash64A(lpString, c * sizeof(WCHAR), is_glyph_index);
#else
	const uint64_t str_hash = MurmurHash64B(lpString, c * sizeof(WCHAR), is_glyph_index);
#endif // _M_X64

	b_ret = _glyph_cache.lookup_glyph_run(_font_trait, str_hash, a_glyph_run);
	if (!b_ret)
	{
		// double-check lock
		gdimm_lock lock(LOCK_GLYPH_RUN_CACHE);

		b_ret = _glyph_cache.lookup_glyph_run(_font_trait, str_hash, a_glyph_run);
		if (!b_ret)
		{
			const int glyph_run_height = render(is_glyph_index, is_pdy, lpString, c, lpDx, a_glyph_run);
			if (glyph_run_height == 0)
				return false;

			_glyph_cache.store_glyph_run(_font_trait, str_hash, a_glyph_run);
		}
	}

	return true;
}