#include "stdafx.h"
#include "config.h"
#include "gdipp_lib/helper.h"

namespace gdipp
{

config::config(const config_file &config_file)
{
	if (config_file.empty())
		_root_node = NULL;
	else
		_root_node = config_file.get_config_xml();
}

config::config(const void *root_node)
	: _root_node(root_node)
{
}

const wchar_t *config::get_string(const wchar_t *config_path, const wchar_t *default_value) const
{
	if (_root_node == NULL)
		return NULL;

	const pugi::xml_node config_node = reinterpret_cast<const pugi::xml_document *>(_root_node)->select_single_node(config_path).node();
	if (config_node.empty())
		return default_value;

	return config_node.value();
}

const bool config::get_string_list(const wchar_t *config_path, std::vector<const wchar_t *> &list_values) const
{
	if (_root_node == NULL)
		return false;

	const pugi::xpath_node_set config_nodes = reinterpret_cast<const pugi::xml_document *>(_root_node)->select_nodes(config_path);
	for (pugi::xpath_node_set::const_iterator iter = config_nodes.begin(); iter != config_nodes.end(); ++iter)
		list_values.push_back(iter->node().value());

	return true;
}

template<typename T>
T config::get_number(const wchar_t *config_path, T default_value) const
{
	if (_root_node == NULL)
		return default_value;

	const pugi::xml_node config_node = reinterpret_cast<const pugi::xml_document *>(_root_node)->select_single_node(config_path).node();
	if (config_node.empty())
		return default_value;

	T config_value;
	wcs_convert(config_node.value(), &config_value);
	return config_value;
}

template<typename T>
const bool config::get_number_list(const wchar_t *config_path, std::vector<T> &list_values) const
{
	if (_root_node == NULL)
		return false;

	const pugi::xpath_node_set config_nodes = reinterpret_cast<const pugi::xml_document *>(_root_node)->select_nodes(config_path);
	for (pugi::xpath_node_set::const_iterator iter = config_nodes.begin(); iter != config_nodes.end(); ++iter)
	{
		T curr_value;
		wcs_convert(iter->node().value(), &curr_value);
		list_values.push_back(curr_value);
	}

	return true;
}

template GDIPP_API int config::get_number(const wchar_t *, int) const;
template GDIPP_API unsigned int config::get_number(const wchar_t *, unsigned int) const;
template long config::get_number(const wchar_t *, long) const;

}
