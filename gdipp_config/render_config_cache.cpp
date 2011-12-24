#include "stdafx.h"
#include "render_config_cache.h"
#include "gdipp_lib/helper.h"
#include "gdipp_lib/lock.h"

namespace gdipp
{

render_config_cache::render_config_cache(const config_file &file)
{
	if (file.empty())
		return;

	const pugi::xml_document *config_xml_doc = reinterpret_cast<const pugi::xml_document *>(file.get_config_xml());
	const pugi::xpath_node_set render_font_nodes = config_xml_doc->select_nodes(L"/gdipp/server/render/font");
	for (pugi::xpath_node_set::const_iterator node_iter = render_font_nodes.begin(); node_iter != render_font_nodes.end(); ++node_iter)
	{
		const pugi::xml_node curr_node = node_iter->node();
		const config cfg(&curr_node);
		const font_config_criteria curr_criteria(&curr_node);
		render_config_static *curr_rcs = new render_config_static();
		curr_rcs->parse(cfg);
		_configs.push_back(std::pair<font_config_criteria, const render_config_static *>(curr_criteria, curr_rcs));			
	}

	_default_config = new render_config_static();
}

render_config_cache::~render_config_cache()
{
	delete _default_config;

	for (std::list<std::pair<font_config_criteria, const render_config_static *>>::const_iterator config_iter = _configs.begin();
		config_iter != _configs.end();
		++config_iter)
		delete config_iter->second;
}

const render_config_static *render_config_cache::get_font_render_config(bool bold, bool italic, LONG height, const wchar_t *font_name)
{
	const uint32_t trait = get_render_config_trait(bold, italic, height, font_name);
	std::map<uint32_t, const render_config_static *>::const_iterator config_iter = _cache.find(trait);
	if (config_iter == _cache.end())
	{
		lock l(lock::CONFIG_RENDER_CACHE);
		config_iter = _cache.find(trait);
		if (config_iter == _cache.end())
		{
			const render_config_static *rcs = find_font_render_config(bold, italic, height, font_name);
			if (rcs == NULL)
				rcs = _default_config;
			_cache.insert(std::pair<uint32_t, const render_config_static *>(trait, rcs));
			return rcs;
		}
	}
	
	return config_iter->second;
}

const render_config_static *render_config_cache::find_font_render_config(bool bold, bool italic, LONG height, const wchar_t *font_name) const
{
	for (std::list<std::pair<font_config_criteria, const render_config_static *>>::const_iterator config_iter = _configs.begin();
		config_iter != _configs.end();
		++config_iter)
	{
		if (config_iter->first.is_satisfied(bold, italic, height, font_name))
			return config_iter->second;
	}

	return NULL;
}

}
