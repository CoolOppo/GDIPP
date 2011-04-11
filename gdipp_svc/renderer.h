#pragma once

#include "rpc_impl.h"
#include <support_rpc.h>

class gdimm_renderer
{
	virtual bool render(bool is_glyph_index, LPCWSTR lpString, UINT c, glyph_run *new_glyph_run);

protected:
	gdipp_rpc_session *_render_session;

public:
	gdimm_renderer(gdipp_rpc_session *render_session);
	virtual ~gdimm_renderer();
};