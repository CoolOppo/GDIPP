#pragma once

#include "gdipp_config/render_config_delta.h"
#include "gdipp_config/font_config_criteria.h"
#include "gdipp_config/proc_config_criteria.h"

namespace gdipp
{

class GDIPP_API render_config_delta_cache
{
public:
	render_config_delta_cache(const config_file &file);
	render_config_delta get_font_render_config_delta(bool bold, bool italic, LONG height, const wchar_t *font_name);
	render_config_delta get_proc_render_config_delta(const wchar_t *proc_name);

private:
	// if the font does not exist in the cache, this function is called to find a render config that matches the criteria
	const render_config_delta *find_font_render_config_delta(bool bold, bool italic, LONG height, const wchar_t *font_name) const;

	std::map<uint32_t, const render_config_delta *> _cache;
	std::list<std::pair<font_config_criteria, render_config_delta>> _font_config_deltas;
	std::list<std::pair<proc_config_criteria, render_config_delta>> _proc_config_deltas;
};

}
