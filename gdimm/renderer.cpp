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

bool gdimm_renderer::begin(const dc_context *context)
{
	_context = context;

	_char_extra = GetTextCharacterExtra(_context->hdc);
	assert(_char_extra != 0x8000000);

	// exclude the bytes after the face name, which may contain junk data
	const int lf_metric_size = sizeof(_context->log_font) - sizeof(_context->log_font.lfFaceName);
	const int lf_facename_size = static_cast<const int>((wcslen(_context->log_font.lfFaceName) * sizeof(wchar_t)));
	const int lf_total_size = lf_metric_size + lf_facename_size;

#ifdef _M_X64
	_font_trait = MurmurHash64A(&_context->log_font, lf_total_size, 0);
#else
	_font_trait = MurmurHash64B(&_context->log_font, lf_total_size, 0);
#endif // _M_X64

	return true;
}

void gdimm_renderer::end()
{
}

void gdimm_renderer::fetch_glyph_run(bool is_glyph_index, bool is_pdy, LPCWSTR lpString, int c, CONST INT *lpDx, glyph_run &a_glyph_run)
{
	bool b_ret;

	uint64_t erased_trait;
	const bool overflow = _glyph_cache._glyph_run_lru.access(_font_trait, erased_trait);
	if (overflow)
	{
		b_ret = _glyph_cache.erase_font_trait(erased_trait);
		assert(b_ret);
	}

#ifdef _M_X64
	const uint64_t str_hash = MurmurHash64A(lpString, c * sizeof(WCHAR), is_glyph_index);
#else
	const uint64_t str_hash = MurmurHash64B(lpString, c * sizeof(WCHAR), is_glyph_index);
#endif // _M_X64

	b_ret = _glyph_cache.lookup_glyph_run(_font_trait, str_hash, a_glyph_run);
	if (!b_ret)
	{
		b_ret = render(is_glyph_index, is_pdy, lpString, c, lpDx, a_glyph_run);
		assert(b_ret);

		_glyph_cache.store_glyph_run(_font_trait, str_hash, a_glyph_run);
	}
}