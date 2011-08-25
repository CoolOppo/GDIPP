#include "stdafx.h"
#include "client_config.h"
#include "gdipp_lib/helper.h"

namespace gdipp
{

client_config::client_config()
	: painter(PAINTER_GDI)
{
}

void client_config::load(const void *root)
{
	if (root == NULL)
		return;

	const pugi::xml_node *root_node = reinterpret_cast<const pugi::xml_node *>(root);
	if (root_node->empty())
		return;

	pugi::xml_node node;
	
	node = root_node->select_single_node(L"painter/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), reinterpret_cast<unsigned short *>(&painter));
}

}
