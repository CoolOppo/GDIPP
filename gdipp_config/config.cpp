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

size_t config::get_string_list(const wchar_t *config_path, const wchar_t **list_values) const
{
	if (_root_node == NULL)
		return 0;

	const pugi::xpath_node_set config_nodes = reinterpret_cast<const pugi::xml_document *>(_root_node)->select_nodes(config_path);
	const size_t config_node_count = config_nodes.size();
	if (list_values == NULL)
		return config_node_count;

	pugi::xpath_node_set::const_iterator iter;
	size_t i;
	for (iter = config_nodes.begin(), i = 0; iter != config_nodes.end(); ++iter, ++i)
		list_values[i] = iter->node().value();

	return config_node_count;
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
size_t config::get_number_list(const wchar_t *config_path, T *list_values) const
{
	if (_root_node == NULL)
		return 0;

	const pugi::xpath_node_set config_nodes = reinterpret_cast<const pugi::xml_document *>(_root_node)->select_nodes(config_path);
	const size_t config_node_count = config_nodes.size();
	if (list_values == NULL)
		return config_node_count;

	pugi::xpath_node_set::const_iterator iter;
	size_t i;
	for (iter = config_nodes.begin(), i = 0; iter != config_nodes.end(); ++iter, ++i)
		wcs_convert(iter->node().value(), &list_values[i]);

	return config_node_count;
}

template GDIPP_API int config::get_number(const wchar_t *, int) const;
template GDIPP_API unsigned int config::get_number(const wchar_t *, unsigned int) const;
template long config::get_number(const wchar_t *, long) const;
template double config::get_number(const wchar_t *, double) const;

}
