#pragma once

#include "gdipp_lib/api.h"

namespace gdipp
{

class GDIPP_API scoped_rw_lock
{
public:
	enum MONITOR_LOCATION
	{
		CLIENT_COM_HOOK,
		CLIENT_GAMMA,
		CONFIG_RENDER_CACHE,
		CONFIG_RENDER_CONFIG_DELTA_CACHE,
		GLOBAL_DEBUG,
		LIB_LRU,
		SERVER_DC_POOL,
		SERVER_FONT_MGR,
		SERVER_FREETYPE,
		SERVER_GLYPH_CACHE,
		SERVER_GLYPH_RUN_CACHE,

		LAST_MONITOR_LOCATION
	};

public:
	static void initialize();

	explicit scoped_rw_lock(MONITOR_LOCATION srw_location, bool is_shared);
	~scoped_rw_lock();

private:
	static std::vector<SRWLOCK> _srws;

	SRWLOCK *_curr_srw;
	bool _is_shared;
};

}
