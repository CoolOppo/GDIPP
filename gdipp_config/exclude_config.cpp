#include "stdafx.h"
#include "exclude_config.h"

namespace gdipp
{

exclude_config::exclude_config()
{
}

void exclude_config::load(const config_file &file)
{
	if (file.empty())
		return;

	const pugi::xml_document *config_xml_doc = reinterpret_cast<const pugi::xml_document *>(file.get_config_xml());
	const pugi::xml_node root = config_xml_doc->select_single_node(L"/gdipp/hook").node();
	if (root.empty())
		return;

	parse(&root);
}

bool exclude_config::is_process_excluded(const wchar_t *proc_name) const
{
	if (proc_name == NULL)
		return false;

	for (std::vector<std::wstring>::const_iterator iter = processes.begin(); iter != processes.end(); ++iter)
	{
		const std::tr1::wregex proc_name_regex(iter->data(),
			std::tr1::regex_constants::icase | std::tr1::regex_constants::nosubs | std::tr1::regex_constants::optimize);
		if (regex_match(proc_name, proc_name_regex))
			return true;
	}

	return false;
}

void exclude_config::parse(const void *root)
{
	if (root == NULL)
		return;

	const pugi::xml_node *root_node = reinterpret_cast<const pugi::xml_node *>(root);
	if (root_node->empty())
		return;

	pugi::xml_node node;

	const pugi::xpath_node_set exclude_nodes = root_node->select_nodes(L"exclude/process/text()");
	for (pugi::xpath_node_set::const_iterator node_iter = exclude_nodes.begin(); node_iter != exclude_nodes.end(); ++node_iter)
	{
		const pugi::xml_node curr_node = node_iter->node();
		if (!curr_node.empty())
			processes.push_back(curr_node.value());
	}
}

}
