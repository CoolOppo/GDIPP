#include "stdafx.h"
#include "font_render_config_cache.h"
#include "gdipp_support/helper.h"
#include "gdipp_support/lock.h"

namespace gdipp
{

font_render_config_cache::font_render_config_cache(const pugi::xpath_node_set &root)
{
	for (pugi::xpath_node_set::const_iterator node_iter = root.begin(); node_iter != root.end(); ++node_iter)
	{
		const pugi::xml_node curr_node = node_iter->node();
		font_config_criteria curr_criteria(curr_node);
		render_config curr_config;
		curr_config.load(curr_node);
		_configs.push_front(std::pair<font_config_criteria, render_config>(curr_criteria, curr_config));			
	}
}

render_config font_render_config_cache::get_render_config(bool bold, bool italic, LONG height, const wchar_t *font_name)
{
	const render_config *rc;

	const uint32_t trait = get_render_config_trait(bold, italic, height, font_name);
	std::map<uint32_t, const render_config *>::const_iterator config_iter = _cache.find(trait);
	if (config_iter == _cache.end())
	{
		lock l("font_render_config_cache");
		config_iter = _cache.find(trait);
		if (config_iter == _cache.end())
		{
			rc = find_render_config(bold, italic, height, font_name);
			_cache.insert(std::pair<uint32_t, const render_config *>(trait, rc));
		}
		else
		{
			rc = config_iter->second;
		}
	}
	else
	{
		rc = config_iter->second;
	}

	if (rc == NULL)
		return render_config();
	else
		return *rc;
}

const render_config *font_render_config_cache::find_render_config(bool bold, bool italic, LONG height, const wchar_t *font_name) const
{
	for (std::list<std::pair<font_config_criteria, render_config>>::const_iterator config_iter = _configs.begin();
		config_iter != _configs.end();
		config_iter++)
	{
		if (config_iter->first.is_satisfied(bold, italic, height, font_name))
			return &config_iter->second;
	}

	return NULL;
}

}
