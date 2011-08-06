#pragma once

#include "rpc_impl.h"
#include "glyph_run.h"
#include "gdipp_support/rpc.h"

namespace gdipp
{

class gdimm_renderer
{
public:
	explicit gdimm_renderer(rpc_session *render_session);
	virtual ~gdimm_renderer();
	virtual bool render(bool is_glyph_index, LPCWSTR lpString, UINT c, glyph_run *new_glyph_run);

protected:
	rpc_session *_render_session;
};

}
