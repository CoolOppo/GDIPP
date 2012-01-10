#include "stdafx.h"
#include "render_config_delta_cache.h"
#include "gdipp_lib/helper.h"
#include "gdipp_lib/scoped_rw_lock.h"

namespace gdipp
{

render_config_delta_cache::render_config_delta_cache(const config_file &file)
{
	if (file.empty())
		return;

	const pugi::xml_document *config_xml_doc = reinterpret_cast<const pugi::xml_document *>(file.get_config_xml());
	const pugi::xpath_node_set font_render_font_nodes = config_xml_doc->select_nodes(L"/gdipp/server/render/font");
	for (pugi::xpath_node_set::const_iterator node_iter = font_render_font_nodes.begin(); node_iter != font_render_font_nodes.end(); ++node_iter)
	{
		const pugi::xml_node curr_node = node_iter->node();
		font_config_criteria curr_criteria(&curr_node);
		render_config_delta curr_config_delta;
		curr_config_delta.parse(&curr_node);
		_font_config_deltas.push_front(std::pair<font_config_criteria, render_config_delta>(curr_criteria, curr_config_delta));			
	}

	const pugi::xpath_node_set proc_render_font_nodes = config_xml_doc->select_nodes(L"/gdipp/server/render/process");
	for (pugi::xpath_node_set::const_iterator node_iter = proc_render_font_nodes.begin(); node_iter != proc_render_font_nodes.end(); ++node_iter)
	{
		const pugi::xml_node curr_node = node_iter->node();
		proc_config_criteria curr_criteria(&curr_node);
		render_config_delta curr_config_delta;
		curr_config_delta.parse(&curr_node);
		_proc_config_deltas.push_front(std::pair<proc_config_criteria, render_config_delta>(curr_criteria, curr_config_delta));			
	}
}

render_config_delta render_config_delta_cache::get_font_render_config_delta(bool bold, bool italic, LONG height, const wchar_t *font_name)
{
	const render_config_delta *rcd;

	const uint32_t trait = get_render_config_trait(bold, italic, height, font_name);
	std::map<uint32_t, const render_config_delta *>::const_iterator config_iter = _cache.find(trait);
	if (config_iter == _cache.end())
	{
		const scoped_rw_lock lock_w(scoped_rw_lock::CONFIG_RENDER_CONFIG_DELTA_CACHE, false);
		config_iter = _cache.find(trait);
		if (config_iter == _cache.end())
		{
			rcd = find_font_render_config_delta(bold, italic, height, font_name);
			_cache.insert(std::pair<uint32_t, const render_config_delta *>(trait, rcd));
		}
		else
		{
			rcd = config_iter->second;
		}
	}
	else
	{
		rcd = config_iter->second;
	}

	if (rcd == NULL)
		return render_config_delta();
	else
		return *rcd;
}

render_config_delta render_config_delta_cache::get_proc_render_config_delta(const wchar_t *proc_name)
{
	for (std::list<std::pair<proc_config_criteria, render_config_delta>>::const_iterator config_iter = _proc_config_deltas.begin();
		config_iter != _proc_config_deltas.end();
		++config_iter)
	{
		if (config_iter->first.is_satisfied(proc_name))
			return config_iter->second;
	}

	return render_config_delta();
}

const render_config_delta *render_config_delta_cache::find_font_render_config_delta(bool bold, bool italic, LONG height, const wchar_t *font_name) const
{
	for (std::list<std::pair<font_config_criteria, render_config_delta>>::const_iterator config_iter = _font_config_deltas.begin();
		config_iter != _font_config_deltas.end();
		++config_iter)
	{
		if (config_iter->first.is_satisfied(bold, italic, height, font_name))
			return &config_iter->second;
	}

	return NULL;
}

}
