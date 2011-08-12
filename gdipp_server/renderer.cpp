#include "stdafx.h"
#include "renderer.h"

namespace gdipp
{

renderer::renderer(rpc_session *render_session)
	: _session(render_session)
{
}

renderer::~renderer()
{
}

bool renderer::render(bool is_glyph_index, LPCWSTR lpString, UINT c, glyph_run *new_glyph_run)
{
	return true;
}

}
