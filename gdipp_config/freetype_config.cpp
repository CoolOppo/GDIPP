#include "stdafx.h"
#include "freetype_config.h"
#include "gdipp_lib/helper.h"

namespace gdipp
{

freetype_config::freetype_config()
	: cache_max_faces(8),
	cache_max_sizes(16),
	cache_max_bytes(1048576),
	lcd_filter(FT_LCD_FILTER_DEFAULT)
{
}

void freetype_config::load(const config_file &file)
{
	if (file.empty())
		return;

	const pugi::xml_document *config_xml_doc = reinterpret_cast<const pugi::xml_document *>(file.get_config_xml());
	const pugi::xml_node root = config_xml_doc->select_single_node(L"/gdipp/freetype").node();
	if (root.empty())
		return;

	parse(&root);
}

void freetype_config::parse(const void *root)
{
	if (root == NULL)
		return;

	const pugi::xml_node *root_node = reinterpret_cast<const pugi::xml_node *>(root);
	if (root_node->empty())
		return;

	pugi::xml_node node;

	node = root_node->select_single_node(L"cache_max_faces/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &cache_max_faces);

	node = root_node->select_single_node(L"cache_max_sizes/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &cache_max_sizes);

	node = root_node->select_single_node(L"cache_max_bytes/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &cache_max_bytes);

	node = root_node->select_single_node(L"lcd_filter/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), reinterpret_cast<short *>(&lcd_filter));
}

}
