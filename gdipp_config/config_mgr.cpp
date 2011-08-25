#include "stdafx.h"
#include "config_mgr.h"
#include "gdipp_lib/helper.h"

namespace gdipp
{

config_mgr::config_mgr(const wchar_t *config_filename)
	: _config_xml(NULL),
	_cache(NULL),
	_ft_conf(NULL),
	_hook_conf(NULL)
{
	// get config file path
	wchar_t config_path[MAX_PATH];
	if (!get_dir_file_path(NULL, config_filename, config_path))
		return;

	pugi::xml_document *config_xml_doc = new pugi::xml_document();
	_config_xml = config_xml_doc;

	if (!config_xml_doc->load_file(config_path))
		return;
}

config_mgr::~config_mgr()
{
	if (_config_xml != NULL)
		delete _config_xml;

	if (_cache != NULL)
		delete _cache;

	if (_ft_conf != NULL)
		delete _ft_conf;

	if (_hook_conf != NULL)
		delete _hook_conf;
}

freetype_config config_mgr::get_freetype_config()
{
	if (_ft_conf == NULL)
	{
		_ft_conf = new freetype_config;
		if (_config_xml != NULL)
		{
			pugi::xml_document *config_xml_doc = reinterpret_cast<pugi::xml_document *>(_config_xml);
			if (!config_xml_doc->empty())
			{
				const pugi::xml_node freetype_node = config_xml_doc->select_single_node(L"/gdipp/freetype").node();
				_ft_conf->load(&freetype_node);
			}
		}
	}

	return *_ft_conf;
}

hook_config config_mgr::get_hook_config()
{
	if (_hook_conf == NULL)
	{
		_hook_conf = new hook_config;
		if (_config_xml != NULL)
		{
			pugi::xml_document *config_xml_doc = reinterpret_cast<pugi::xml_document *>(_config_xml);
			if (!config_xml_doc->empty())
			{
				const pugi::xml_node hook_node = config_xml_doc->select_single_node(L"/gdipp/hook").node();
				_hook_conf->load(&hook_node);
			}
		}
	}

	return *_hook_conf;
}

render_config config_mgr::get_font_render_config(bool bold, bool italic, LONG height, const wchar_t *font_name)
{
	if (_cache == NULL)
	{
		render_config render_conf;

		if (_config_xml == NULL)
			return render_conf;

		pugi::xml_document *config_xml_doc = reinterpret_cast<pugi::xml_document *>(_config_xml);

		const pugi::xpath_node_set render_font_nodes = config_xml_doc->select_nodes(L"/gdipp/render/font");
		_cache = new font_render_config_cache(&render_font_nodes);	
	}

	return _cache->get_render_config(bold, italic, height, font_name);
}

}
