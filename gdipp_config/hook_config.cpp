#include "stdafx.h"
#include "hook_config.h"
#include "gdipp_lib/helper.h"

namespace gdipp
{

hook_config::hook_config()
	: proc_32_bit(true),
	proc_64_bit(true)
{
}

void hook_config::load(const config_file &file)
{
	if (file.empty())
		return;

	const pugi::xml_document *config_xml_doc = reinterpret_cast<const pugi::xml_document *>(file.get_config_xml());
	const pugi::xml_node root = config_xml_doc->select_single_node(L"/gdipp/hook").node();
	if (root.empty())
		return;

	parse(&root);
}

void hook_config::parse(const void *root)
{
	if (root == NULL)
		return;

	const pugi::xml_node *root_node = reinterpret_cast<const pugi::xml_node *>(root);
	if (root_node->empty())
		return;

	pugi::xml_node node;

	node = root_node->select_single_node(L"include/proc_32_bit/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &proc_32_bit);
	
	node = root_node->select_single_node(L"include/proc_64_bit/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &proc_64_bit);
}

}
