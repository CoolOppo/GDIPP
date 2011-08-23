#include "stdafx.h"
#include "freetype_config.h"
#include "gdipp_support/helper.h"

namespace gdipp
{

freetype_config::freetype_config()
	: cache_max_faces(8),
	cache_max_sizes(16),
	cache_max_bytes(1048576),
	lcd_filter(FT_LCD_FILTER_DEFAULT)
{
}

void freetype_config::load(const pugi::xml_node &root)
{
	if (root.empty())
		return;

	pugi::xml_node node;
	
	node = root.select_single_node(L"cache_max_faces/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &cache_max_faces);

	node = root.select_single_node(L"cache_max_sizes/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &cache_max_sizes);

	node = root.select_single_node(L"cache_max_bytes/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &cache_max_bytes);

	node = root.select_single_node(L"lcd_filter/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), reinterpret_cast<short *>(&lcd_filter));
}

}
