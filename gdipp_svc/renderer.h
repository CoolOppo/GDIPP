#pragma once

#include "rpc_impl.h"
#include "gdipp_support/gs_rpc.h"

class gdimm_renderer
{
public:
	explicit gdimm_renderer(gdipp_rpc_session *render_session);
	virtual ~gdimm_renderer();

protected:
	gdipp_rpc_session *_render_session;

private:
	virtual bool render(bool is_glyph_index, LPCWSTR lpString, UINT c, glyph_run *new_glyph_run);
};
