#pragma once

#include "gdipp_config/render_config.h"
#include "gdipp_config/font_config_criteria.h"

namespace gdipp
{

class GDIPP_API render_config_cache
{
public:
	render_config_cache(const config_file &file);
	render_config get_font_render_config(bool bold, bool italic, LONG height, const wchar_t *font_name);

private:
	// if the font does not exist in the cache, this function is called to find a render config that matches the criteria
	const render_config *find_font_render_config(bool bold, bool italic, LONG height, const wchar_t *font_name) const;

	std::map<uint32_t, const render_config *> _cache;
	std::list<std::pair<font_config_criteria, render_config>> _configs;
};

}
