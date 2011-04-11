#include "stdafx.h"
#include "renderer.h"

gdimm_renderer::gdimm_renderer(gdipp_rpc_session *render_session)
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