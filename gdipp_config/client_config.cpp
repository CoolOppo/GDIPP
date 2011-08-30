#include "stdafx.h"
#include "client_config.h"
#include "gdipp_lib/helper.h"

namespace gdipp
{

client_config::gamma_config::gamma_config()
	: red(1.0),
	green(1.0),
	blue(1.0)
{
}

client_config::shadow_config::shadow_config()
	: offset_x(0),
	offset_y(0),
	alpha(0)
{
}

client_config::client_config()
	: painter(PAINTER_GDI)
{
}

void client_config::load(const config_file &file)
{
	if (file.empty())
		return;

	const pugi::xml_document *config_xml_doc = reinterpret_cast<const pugi::xml_document *>(file.get_config_xml());
	const pugi::xml_node root = config_xml_doc->select_single_node(L"/gdipp/client").node();
	if (root.empty())
		return;

	parse(&root);
}

void client_config::parse(const void *root)
{
	if (root == NULL)
		return;

	const pugi::xml_node *root_node = reinterpret_cast<const pugi::xml_node *>(root);
	if (root_node->empty())
		return;

	pugi::xml_node node;

	node = root_node->select_single_node(L"gamma/red/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &gamma.red);

	node = root_node->select_single_node(L"gamma/green/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &gamma.green);

	node = root_node->select_single_node(L"gamma/blue/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &gamma.blue);
	
	node = root_node->select_single_node(L"painter/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), reinterpret_cast<unsigned short *>(&painter));

	node = root_node->select_single_node(L"shadow/offset_x/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &shadow.offset_x);

	node = root_node->select_single_node(L"shadow/offset_y/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &shadow.offset_y);

	node = root_node->select_single_node(L"shadow/alpha/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), reinterpret_cast<short *>(&shadow.alpha));
}

}
