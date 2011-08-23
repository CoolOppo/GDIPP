#include "stdafx.h"
#include "config_mgr.h"
#include "gdipp_lib/gdipp_lib.h"

namespace gdipp
{

config_mgr::config_mgr()
{
	// get config file path
	wchar_t config_path[MAX_PATH];
	if (!get_dir_file_path(NULL, L"server.conf", config_path))
		return;

	pugi::xml_document xml_doc;

	if (!xml_doc.load_file(config_path))
		return;
	
	const pugi::xml_node freetype_node = xml_doc.select_single_node(L"/gdipp/freetype").node();
	ft_conf.load(freetype_node);

	const pugi::xml_node hook_node = xml_doc.select_single_node(L"/gdipp/hook").node();
	hook_conf.load(hook_node);

	const pugi::xpath_node_set render_font_nodes = xml_doc.select_nodes(L"/gdipp/render/font");
	init_font_render_config_cache(render_font_nodes);
}

}
