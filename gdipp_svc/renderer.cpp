#include "stdafx.h"
#include "renderer.h"
#include <MurmurHash2_64.h>

gdimm_renderer::gdimm_renderer(gdipp_rpc_session *render_session)
	: _render_session(render_session)
{
}

gdimm_renderer::~gdimm_renderer()
{
}

bool gdimm_renderer::render(bool is_glyph_index, LPCWSTR lpString, UINT c, glyph_run &new_glyph_run)
{
	return true;
}

bool gdimm_renderer::fetch_glyph_run(bool is_glyph_index, LPCWSTR lpString, int c, glyph_run &a_glyph_run)
{
	bool b_ret;

#ifdef _M_X64
	const unsigned __int64 string_id = MurmurHash64A(lpString, c * sizeof(WCHAR), is_glyph_index);
#else
	const unsigned __int64 string_id = MurmurHash64B(lpString, c * sizeof(WCHAR), is_glyph_index);
#endif // _M_X64

	b_ret = glyph_cache_instance.lookup_glyph_run(_render_session->render_trait, string_id, a_glyph_run);
	b_ret = false;
	if (!b_ret)
	{
		const int glyph_run_height = render(is_glyph_index, lpString, c, a_glyph_run);
		if (glyph_run_height == 0)
			return false;

		glyph_cache_instance.store_glyph_run(_render_session->render_trait, string_id, a_glyph_run);
	}

	return true;
}