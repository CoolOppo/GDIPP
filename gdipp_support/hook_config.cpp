#include "stdafx.h"
#include "hook_config.h"
#include "gdipp_support/helper.h"

namespace gdipp
{
hook_config::hook_config()
	: proc_32_bit(true),
	proc_64_bit(true)
{
}

void hook_config::load(const pugi::xml_node &root)
{
	if (root.empty())
		return;

	pugi::xml_node node;

	node = root.select_single_node(L"include/proc_32_bit/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &proc_32_bit);
	
	node = root.select_single_node(L"include/proc_64_bit/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &proc_64_bit);
	
	const pugi::xpath_node_set exclude_nodes = root.select_nodes(L"exclude/process/text()");
	for (pugi::xpath_node_set::const_iterator node_iter = exclude_nodes.begin(); node_iter != exclude_nodes.end(); ++node_iter)
	{
		const pugi::xml_node curr_node = node_iter->node();
		if (!curr_node.empty())
			excludes.push_back(curr_node.value());
	}
}

}
