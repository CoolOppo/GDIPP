#pragma once

#include "api.h"

namespace gdipp
{

class GDIPP_API lock
{
public:
	enum LOCK_TYPE
	{
		CLIENT_COM_HOOK,
		CLIENT_GAMMA,
		CONFIG_RENDER_CACHE,
		CONFIG_RENDER_CONFIG_DELTA_CACHE,
		GLOBAL_DEBUG,
		SERVER_DC_POOL,
		SERVER_FONT_MGR,
		SERVER_FREETYPE,
		SERVER_GLYPH_CACHE,
		SERVER_GLYPH_RUN_CACHE,
		SERVER_LRU,

		LAST_LOCK_TYPE
	};

public:
	static void initialize_locks();
	static void destory_locks();

	explicit lock(LOCK_TYPE lock_type);
	~lock();

private:
	static std::vector<CRITICAL_SECTION> _locks;

	CRITICAL_SECTION *_cs;
};

}
