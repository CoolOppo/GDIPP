#include "stdafx.h"
#include "demo_config.h"
#include "gdipp_lib/helper.h"

namespace gdipp
{

demo_config::demo_config()
	: cycles(5000),
	threads(1),
	random_text(false)
{
}

void demo_config::load(const config_file &file)
{
	if (file.empty())
		return;

	const pugi::xml_document *config_xml_doc = reinterpret_cast<const pugi::xml_document *>(file.get_config_xml());
	const pugi::xml_node root = config_xml_doc->select_single_node(L"/gdipp/demo").node();
	if (root.empty())
		return;

	parse(&root);
}

void demo_config::parse(const void *root)
{
	if (root == NULL)
		return;

	const pugi::xml_node *root_node = reinterpret_cast<const pugi::xml_node *>(root);
	if (root_node->empty())
		return;

	pugi::xml_node node;
	
	node = root_node->select_single_node(L"cycles/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &cycles);

	node = root_node->select_single_node(L"threads/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), reinterpret_cast<unsigned short *>(&threads));

	node = root_node->select_single_node(L"random_text/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &random_text);

	const pugi::xpath_node_set font_nodes = root_node->select_nodes(L"fonts/font/text()");
	for (pugi::xpath_node_set::const_iterator node_iter = font_nodes.begin(); node_iter != font_nodes.end(); ++node_iter)
	{
		const pugi::xml_node curr_node = node_iter->node();
		if (!curr_node.empty())
			fonts.push_back(curr_node.value());
	}
}

}
