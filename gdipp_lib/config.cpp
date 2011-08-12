#include "stdafx.h"
#include "config.h"
#include "gdipp_support/config_trait.h"

namespace gdipp
{

const std::tr1::regex_constants::syntax_option_type regex_flags = std::tr1::regex_constants::icase | std::tr1::regex_constants::nosubs | std::tr1::regex_constants::optimize;

config::config()
	: _xml_doc(NULL)
{
	DWORD dw_ret = GetModuleBaseNameW(GetCurrentProcess(), NULL, _process_name, MAX_PATH);
	assert(dw_ret != 0);
}

config::~config()
{
	if (_xml_doc != NULL)
		delete _xml_doc;
}

const wchar_t *config::get_server_config(const wchar_t *setting_name, const config_trait *trait) const
{
	// check config for the current process
	config_map::const_iterator config_iter = _config_server_process.find(setting_name);
	if (config_iter != _config_server_process.end())
		return config_iter->second.c_str();

	if (trait != NULL)
	{
		// check config for the specified font
		for (std::list<conf_server_font_node>::const_iterator list_iter = _config_server_fonts.begin(); list_iter != _config_server_fonts.end(); ++list_iter)
		{
			// check next font if optional attributes match
			// easy checks come first

			if ((list_iter->bold >= 0) && (!list_iter->bold == (trait->get_weight_class() > 1)))
				continue;

			if ((list_iter->italic >= 0) && (!list_iter->italic == trait->get_italic()))
				continue;

			if ((list_iter->max_height >= 0) && (list_iter->max_height < trait->get_height()))
				continue;

			if (!list_iter->name.empty())
			{
				const std::tr1::wregex name_ex(list_iter->name.data(), regex_flags);
				// check next font if font name match
				if (!regex_match(trait->get_font_name(), name_ex))
					continue;
			}

			config_iter = list_iter->configs.find(setting_name);
			if (config_iter != list_iter->configs.end())
				return config_iter->second.c_str();
		}
	}

	return NULL;
}

const wchar_t *config::get_demo_config(const wchar_t *setting_name) const
{
	config_map::const_iterator iter = _config_demo.find(setting_name);

	if (iter == _config_demo.end())
		return NULL;
	else
		return iter->second.c_str();
}

const std::vector<const std::wstring> &config::get_demo_fonts() const
{
	return _demo_fonts;
}

const wchar_t *config::get_server_hook_config(const wchar_t *setting_name) const
{
	config_map::const_iterator iter = _config_server_hook.find(setting_name);

	if (iter == _config_server_hook.end())
		return NULL;
	else
		return iter->second.c_str();
}

bool config::is_process_excluded(const wchar_t *proc_name) const
{
	// if no process name is specified, return true if the current process is excluded
	// otherwise, return true if the specified process is excluded

	const wchar_t *final_name;
	if (proc_name == NULL)
		final_name = _process_name;
	else
		final_name = proc_name;

	for (std::list<const std::wstring>::const_iterator iter = _config_client_excludes.begin(); iter != _config_client_excludes.end(); ++iter)
	{
		const std::tr1::wregex name_ex(iter->data(), regex_flags);
		if (regex_match(final_name, name_ex))
			return true;
	}

	return false;
}

void config::init_config()
{
	if (_xml_doc != NULL)
		delete _xml_doc;

	_xml_doc = new pugi::xml_document;

	// clear existing settings
	_config_server_process.clear();
	_config_server_fonts.clear();
	_config_demo.clear();
	_demo_fonts.clear();
	_config_client_excludes.clear();
}

BOOL config::load_config(const wchar_t *config_path)
{
	if (!_xml_doc->load_file(pugi::as_utf8(config_path).c_str()))
		return FALSE;

	const pugi::xpath_node_set proc_list = _xml_doc->select_nodes(L"/gdipp/server/process");
	if (!proc_list.empty())
		load_server_process(proc_list);

	const pugi::xpath_node_set font_list = _xml_doc->select_nodes(L"/gdipp/server/font");
	if (!font_list.empty())
		load_server_font(font_list);

	const pugi::xml_node hook_node = _xml_doc->select_single_node(L"/gdipp/server/hook").node();
	if (!hook_node.empty())
		load_server_hook(hook_node);

	const pugi::xml_node exclude_node = _xml_doc->select_single_node(L"/gdipp/client/exclude").node();
	if (!exclude_node.empty())
		load_client_exclude(exclude_node);

	const pugi::xml_node demo_node = _xml_doc->select_single_node(L"/gdipp/demo").node();
	if (!demo_node.empty())
		load_demo(demo_node);

	return TRUE;
}

BOOL config::save_config(const wchar_t *config_path)
{
	return _xml_doc->save_file(pugi::as_utf8(config_path).c_str());
}

BOOL config::insert_setting(const wchar_t *node_name, const wchar_t *node_text, const wchar_t *parent_xpath, const wchar_t *ref_node_xpath, std::wstring &new_node_xpath)
{
	// insert the new node as a child node before the reference node, and output its XPath

	bool b_ret;

	pugi::xml_node parent_node = _xml_doc->select_single_node(parent_xpath).node();
	if (parent_node.empty())
		return FALSE;

	const pugi::xml_node ref_node = _xml_doc->select_single_node(ref_node_xpath).node();
	if (ref_node.empty())
		return FALSE;

	pugi::xml_node new_node = parent_node.insert_child_before(pugi::node_element, ref_node);
	if (new_node.empty())
		return FALSE;

	pugi::xml_node text_node = new_node.append_child(pugi::node_pcdata);
	if (text_node.empty())
		return FALSE;

	b_ret = new_node.set_name(node_name);
	if (!b_ret)
		return FALSE;

	text_node.set_value(node_text);
	if (!b_ret)
		return FALSE;

	new_node_xpath = new_node.path();
	return TRUE;
}

BOOL config::set_setting_attr(const wchar_t *node_xpath, const wchar_t *attr_name, const wchar_t *attr_value)
{
	pugi::xml_node node = _xml_doc->select_single_node(node_xpath).node();
	if (node.empty())
		return FALSE;

	pugi::xml_attribute node_attr = node.attribute(attr_name);
	if (node_attr.empty())
		node_attr = node.append_attribute(attr_name);

	return node_attr.set_value(attr_value);
}

BOOL config::remove_setting(const wchar_t *node_xpath)
{
	const pugi::xml_node node = _xml_doc->select_single_node(node_xpath).node();
	if (node.empty())
		return FALSE;

	pugi::xml_node parent_node = node.parent();
	if (parent_node.empty())
		return FALSE;

	parent_node.remove_child(node);
	return TRUE;
}

void config::parse_server_config_node(const pugi::xml_node &setting_node, config_map &setting_store)
{
	const pugi::string_t name = setting_node.name();

	if (name == L"freetype" || name == L"gamma" || name == L"render_mode" || name == L"shadow")
	{
		// these settings have nested items
		for (pugi::xml_node::iterator iter = setting_node.begin(); iter != setting_node.end(); ++iter)
			setting_store[name + L"/" + iter->name()] = iter->first_child().value();
	}
	else
		setting_store[name] = setting_node.first_child().value();
}

void config::load_server_process(const pugi::xpath_node_set &process_nodes)
{
	// backward iterate so that first-coming process settings overwrites last-coming ones

	pugi::xpath_node_set::const_iterator node_iter = process_nodes.end();
	node_iter--;

	for (size_t i = 0; i < process_nodes.size(); ++i, node_iter--)
	{
		// only store the setting items which match the current process name

		const pugi::xml_node curr_proc = node_iter->node();
		const pugi::xml_attribute name_attr = curr_proc.attribute(L"name");

		bool process_matched = name_attr.empty();
		if (!process_matched)
		{
			const std::tr1::wregex name_ex(name_attr.value(), regex_flags);
			process_matched = regex_match(_process_name, name_ex);
		}

		if (process_matched)
		{
			for (pugi::xml_node::iterator set_iter = node_iter->node().begin(); set_iter != node_iter->node().end(); ++set_iter)
				parse_server_config_node(*set_iter, _config_server_process);
		}
	}
}

void config::load_server_font(const pugi::xpath_node_set &font_node)
{
	for (pugi::xpath_node_set::const_iterator node_iter = font_node.begin(); node_iter != font_node.end(); ++node_iter)
	{
		config_map curr_settings;

		for (pugi::xml_node::iterator set_iter = node_iter->node().begin(); set_iter != node_iter->node().end(); ++set_iter)
			parse_server_config_node(*set_iter, curr_settings);

		const pugi::xml_node curr_font = node_iter->node();
		const pugi::xml_attribute name_attr = curr_font.attribute(L"name");
		const pugi::xml_attribute bold_attr = curr_font.attribute(L"bold");
		const pugi::xml_attribute italic_attr = curr_font.attribute(L"italic");
		const pugi::xml_attribute max_height_attr = curr_font.attribute(L"max_height");

		// negative indicates such optional attribute is not specified
		const conf_server_font_node new_font = {(name_attr.empty() ? std::wstring() : name_attr.value()),
			(bold_attr.empty() ? -1 : bold_attr.as_uint()),
			(italic_attr.empty() ? -1 : italic_attr.as_uint()),
			(max_height_attr.empty() ? -1 : max_height_attr.as_uint()),
			curr_settings};
		_config_server_fonts.push_back(new_font);
	}
}

void config::load_demo(const pugi::xml_node &root_node)
{
	for (pugi::xml_node::iterator iter = root_node.begin(); iter != root_node.end(); ++iter)
	{
		const std::wstring node_name = iter->name();
		const std::wstring curr_value = iter->first_child().value();

		if (node_name == L"font")
			_demo_fonts.push_back(curr_value);
		else
			_config_demo[node_name] = curr_value;
	}
}

void config::load_server_hook(const pugi::xml_node &root_node)
{
	for (pugi::xml_node::iterator iter = root_node.begin(); iter != root_node.end(); ++iter)
	{
		const std::wstring node_name = iter->name();
		const std::wstring curr_value = iter->first_child().value();

		_config_server_hook[node_name] = curr_value;
	}
}

void config::load_client_exclude(const pugi::xml_node &root_node)
{
	for (pugi::xml_node::iterator iter = root_node.begin(); iter != root_node.end(); ++iter)
		_config_client_excludes.push_back(iter->first_child().value());
}

}
