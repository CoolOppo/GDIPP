#pragma once

#include "font_man.h"
#include "glyph_cache.h"
#include "helper_def.h"

template <typename font_trait_type>
class gdimm_renderer
{
	virtual bool render(LPCWSTR lpString, UINT c, bool is_glyph_index, glyph_run &a_glyph_run) = 0;

protected:
	const dc_context *_context;
	font_trait_type _font_trait;
	gdimm_glyph_cache<font_trait_type> _glyph_cache;
	int _char_extra;
	gdimm_font_man _font_man;

public:
	virtual ~gdimm_renderer()
	{
	}

	virtual bool begin(const dc_context *context)
	{
		if (!_font_trait.extract(context->hdc))
			return false;

		_context = context;

		_char_extra = GetTextCharacterExtra(_context->hdc);
		assert(_char_extra != 0x8000000);

		return true;
	}

	int fetch_glyph_run(LPCWSTR lpString, int c, bool is_glyph_index, const glyph_run *&glyph_run_ptr)
	{
		bool b_ret;

		font_trait_type erased_trait;
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

		glyph_run_ptr = _glyph_cache.lookup_glyph_run(_font_trait, str_hash);
		if (glyph_run_ptr != NULL)
			return 0;

		glyph_run new_glyph_run;
		b_ret = render(lpString, c, is_glyph_index, new_glyph_run);
		assert(b_ret);

		glyph_run_ptr = _glyph_cache.store_glyph_run(_font_trait, str_hash, new_glyph_run);
		assert(glyph_run_ptr != NULL);

		return 0;
	}
	
	virtual void end()
	{
	}
};