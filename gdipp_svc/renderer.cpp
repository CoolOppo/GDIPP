#include "stdafx.h"
#include "renderer.h"

namespace gdipp
{

gdimm_renderer::gdimm_renderer(rpc_session *render_session)
	: _render_session(render_session)
{
}

gdimm_renderer::~gdimm_renderer()
{
}

bool gdimm_renderer::render(bool is_glyph_index, LPCWSTR lpString, UINT c, glyph_run *new_glyph_run)
{
	return true;
}

}
