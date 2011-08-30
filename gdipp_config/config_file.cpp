#include "stdafx.h"
#include "config_file.h"
#include "gdipp_lib/helper.h"

namespace gdipp
{

config_file::config_file(const wchar_t *filename)
	: _config_xml(NULL)
{
	// get config file path
	wchar_t config_path[MAX_PATH];
	if (!get_dir_file_path(NULL, filename, config_path))
		return;

	pugi::xml_document *config_xml_doc = new pugi::xml_document();
	_config_xml = config_xml_doc;

	config_xml_doc->load_file(config_path);
}

config_file::~config_file()
{
	if (_config_xml != NULL)
		delete _config_xml;
}

const void *config_file::get_config_xml() const
{
	return _config_xml;
}

bool config_file::empty() const
{
	if (_config_xml == NULL)
		return true;

	pugi::xml_document *config_xml_doc = reinterpret_cast<pugi::xml_document *>(_config_xml);
	return config_xml_doc->empty();
}
/*
render_config config_file::get_font_render_config(bool bold, bool italic, LONG height, const wchar_t *font_name)
{
	if (_cache == NULL)
	{
		render_config render_conf;

		if (_config_xml == NULL)
			return render_conf;

		pugi::xml_document *config_xml_doc = reinterpret_cast<pugi::xml_document *>(_config_xml);

		const pugi::xpath_node_set render_font_nodes = config_xml_doc->select_nodes(L"/gdipp/render/font");
		_cache = new render_config_cache(&render_font_nodes);	
	}

	return _cache->get_render_config(bold, italic, height, font_name);
}
*/
}
