#include "stdafx.h"
#include "client_config.h"
#include "gdipp_support/helper.h"

namespace gdipp
{

client_config::client_config()
	: painter(PAINTER_GDI)
{
}

void client_config::load(const pugi::xml_node &root)
{
	if (root.empty())
		return;

	pugi::xml_node node;
	
	node = root.select_single_node(L"painter/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), reinterpret_cast<unsigned short *>(&painter));
}

}
