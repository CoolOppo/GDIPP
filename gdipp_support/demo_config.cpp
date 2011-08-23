#include "stdafx.h"
#include "demo_config.h"
#include "gdipp_support/helper.h"

namespace gdipp
{

demo_config::demo_config()
	: cycles(5000),
	threads(1),
	random_text(false)
{
}

void demo_config::load(const pugi::xml_node &root)
{
	if (root.empty())
		return;

	pugi::xml_node node;
	
	node = root.select_single_node(L"cycles/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &cycles);

	node = root.select_single_node(L"threads/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), reinterpret_cast<unsigned short *>(&threads));

	node = root.select_single_node(L"random_text/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &random_text);

	const pugi::xpath_node_set font_nodes = root.select_nodes(L"fonts/font/text()");
	for (pugi::xpath_node_set::const_iterator node_iter = font_nodes.begin(); node_iter != font_nodes.end(); ++node_iter)
	{
		const pugi::xml_node curr_node = node_iter->node();
		if (!curr_node.empty())
			fonts.push_back(curr_node.value());
	}
}

}
