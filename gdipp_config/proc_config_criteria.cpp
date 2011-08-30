#include "stdafx.h"
#include "proc_config_criteria.h"

namespace gdipp
{

proc_config_criteria::proc_config_criteria(const void *node)
{
	if (node == NULL)
		return;

	const pugi::xml_node *node_ptr = reinterpret_cast<const pugi::xml_node *>(node);
	if (node_ptr->empty())
		return;

	pugi::xml_attribute attr;
	
	attr = node_ptr->attribute(L"name");
	if (!attr.empty())
		_proc_name = attr.value();
}

bool proc_config_criteria::is_satisfied(const wchar_t *proc_name) const
{
	if (!_proc_name.empty())
	{
		const std::tr1::wregex font_name_regex(_proc_name,
			std::tr1::regex_constants::icase | std::tr1::regex_constants::nosubs | std::tr1::regex_constants::optimize);
		return regex_match(proc_name, font_name_regex);
	}

	return true;
}

}
