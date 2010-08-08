#include "stdafx.h"
#include "setting.h"

gdipp_setting::gdipp_setting()
	: _xml_doc(NULL)
{
	DWORD dw_ret = GetModuleBaseNameW(GetCurrentProcess(), NULL, _process_name, MAX_PATH);
	assert(dw_ret != 0);
}

gdipp_setting::~gdipp_setting()
{
	if (_xml_doc != NULL)
		delete _xml_doc;
}

void gdipp_setting::parse_gdimm_setting_node(const xml_node &setting_node, setting_map &setting_store)
{
	const string_t name = setting_node.name();

	if (name == L"freetype" || name == L"gamma" || name == L"render_mode" || name == L"shadow")
	{
		// these settings have nested items
		for (xml_node::iterator iter = setting_node.begin(); iter != setting_node.end(); iter++)
			setting_store[name + L"/" + iter->name()] = iter->first_child().value();
	}
	else
		setting_store[name] = setting_node.first_child().value();
}

void gdipp_setting::load_gdimm_process(const xpath_node_set &process_nodes)
{
	// backward iterate so that first-coming process settings overwrites last-coming ones

	xpath_node_set::const_iterator node_iter = process_nodes.end();
	node_iter--;

	for (size_t i = 0; i < process_nodes.size(); i++, node_iter--)
	{
		// only store the setting items which match the current process name

		const xml_node curr_proc = node_iter->node();
		const xml_attribute name_attr = curr_proc.attribute(L"name");

		const wregex name_ex(name_attr.value(),
			std::tr1::regex_constants::icase | std::tr1::regex_constants::nosubs | std::tr1::regex_constants::optimize);
		if (regex_match(_process_name, name_ex))
		{
			for (xml_node::iterator set_iter = node_iter->node().begin(); set_iter != node_iter->node().end(); set_iter++)
				parse_gdimm_setting_node(*set_iter, _process_setting);
		}
	}
}

void gdipp_setting::load_gdimm_font(const xpath_node_set &font_node)
{
	for (xpath_node_set::const_iterator node_iter = font_node.begin(); node_iter != font_node.end(); node_iter++)
	{
		setting_map curr_settings;

		for (xml_node::iterator set_iter = node_iter->node().begin(); set_iter != node_iter->node().end(); set_iter++)
			parse_gdimm_setting_node(*set_iter, curr_settings);

		const xml_node curr_font = node_iter->node();
		const xml_attribute name_attr = curr_font.attribute(L"name");
		const xml_attribute bold_attr = curr_font.attribute(L"bold");
		const xml_attribute italic_attr = curr_font.attribute(L"italic");

		// negative indicates such optional attribute is not specified
		const gdimm_font_node new_font = {name_attr.value(),
			(bold_attr.empty() ? -1 : bold_attr.as_uint()),
			(italic_attr.empty() ? -1 : italic_attr.as_uint()),
			curr_settings};
		_gdimm_font.push_back(new_font);
	}
}

void gdipp_setting::load_demo(const xml_node &root_node)
{
	for (xml_node::iterator iter = root_node.begin(); iter != root_node.end(); iter++)
	{
		const wstring node_name = iter->name();
		const wstring curr_value = iter->first_child().value();

		if (node_name == L"font")
			_demo_fonts.push_back(curr_value);
		else
			_demo_setting[node_name] = curr_value;
	}
}

void gdipp_setting::load_exclude(const xml_node &root_node)
{
	for (xml_node::iterator iter = root_node.begin(); iter != root_node.end(); iter++)
		_exclude_process.push_back(iter->first_child().value());
}

const wchar_t *gdipp_setting::get_gdimm_setting(const wchar_t *setting_name, const wchar_t *font_name, unsigned char weight, bool italic) const
{
	// check setting for the current process
	setting_map::const_iterator setting_iter = _process_setting.find(setting_name);
	if (setting_iter != _process_setting.end())
		return setting_iter->second.c_str();

	// check setting for the specified font
	for (list<gdimm_font_node>::const_iterator list_iter = _gdimm_font.begin(); list_iter != _gdimm_font.end(); list_iter++)
	{
		// check next font if optional attributes match
		// easy checks come first

		if ((list_iter->bold >= 0) && (!list_iter->bold == (weight > 0)))
			continue;

		if ((list_iter->italic >= 0) && (!list_iter->italic == italic))
			continue;

		const wregex name_ex(list_iter->name.data(),
			std::tr1::regex_constants::icase | std::tr1::regex_constants::nosubs | std::tr1::regex_constants::optimize);
		// check next font if font name match
		if (!regex_match(font_name, name_ex))
			continue;

		setting_iter = list_iter->settings.find(setting_name);
		if (setting_iter != list_iter->settings.end())
			return setting_iter->second.c_str();
	}

	return NULL;
}

const wchar_t *gdipp_setting::get_demo_setting(const wchar_t *setting_name) const
{
	setting_map::const_iterator iter = _demo_setting.find(setting_name);

	if (iter == _demo_setting.end())
		return NULL;
	else
		return iter->second.c_str();
}

const vector<const wstring> &gdipp_setting::get_demo_fonts() const
{
	return _demo_fonts;
}

bool gdipp_setting::is_process_excluded(const wchar_t *proc_name) const
{
	// if no process name is specified, return true if the current process is excluded
	// otherwise, return true if the specified process is excluded

	const wchar_t *final_name;
	if (proc_name == NULL)
		final_name = _process_name;
	else
		final_name = proc_name;

	for (list<const wstring>::const_iterator iter = _exclude_process.begin(); iter != _exclude_process.end(); iter++)
	{
		const wregex name_ex(iter->data(),
			std::tr1::regex_constants::icase | std::tr1::regex_constants::nosubs | std::tr1::regex_constants::optimize);
		if (regex_match(final_name, name_ex))
			return true;
	}

	return false;
}

void gdipp_setting::init_setting()
{
	if (_xml_doc != NULL)
		delete _xml_doc;

	_xml_doc = new xml_document;

	// clear existing settings
	_process_setting.clear();
	_gdimm_font.clear();
	_demo_setting.clear();
	_demo_fonts.clear();
	_exclude_process.clear();
}

BOOL gdipp_setting::load_setting(const wchar_t *setting_path)
{
	if (!_xml_doc->load_file(as_utf8(setting_path).c_str()))
		return FALSE;

	const xpath_node_set proc_list = _xml_doc->select_nodes(L"/gdipp/gdimm/process");
	if (!proc_list.empty())
		load_gdimm_process(proc_list);

	const xpath_node_set font_list = _xml_doc->select_nodes(L"/gdipp/gdimm/font");
	if (!font_list.empty())
		load_gdimm_font(font_list);

	const xml_node demo_node = _xml_doc->select_single_node(L"/gdipp/demo").node();
	if (!demo_node.empty())
		load_demo(demo_node);

	const xml_node exclude_node = _xml_doc->select_single_node(L"/gdipp/exclude").node();
	if (!exclude_node.empty())
		load_exclude(exclude_node);

	return TRUE;
}

BOOL gdipp_setting::save_setting(const wchar_t *setting_path)
{
	return _xml_doc->save_file(as_utf8(setting_path).c_str());
}

BOOL gdipp_setting::insert_setting(const wchar_t *node_name, const wchar_t *node_text, const wchar_t *parent_xpath, const wchar_t *ref_node_xpath, wstring &new_node_xpath)
{
	// insert the new node as a child node before the reference node, and output its XPath

	bool b_ret;

	xml_node parent_node = _xml_doc->select_single_node(parent_xpath).node();
	if (parent_node.empty())
		return FALSE;

	const xml_node ref_node = _xml_doc->select_single_node(ref_node_xpath).node();
	if (ref_node.empty())
		return FALSE;

	xml_node new_node = parent_node.insert_child_before(node_element, ref_node);
	if (new_node.empty())
		return FALSE;

	xml_node text_node = new_node.append_child(node_pcdata);
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

BOOL gdipp_setting::set_setting_attr(const wchar_t *node_xpath, const wchar_t *attr_name, const wchar_t *attr_value)
{
	xml_node node = _xml_doc->select_single_node(node_xpath).node();
	if (node.empty())
		return FALSE;

	xml_attribute node_attr = node.attribute(attr_name);
	if (node_attr.empty())
		node_attr = node.append_attribute(attr_name);

	return node_attr.set_value(attr_value);
}

BOOL gdipp_setting::remove_setting(const wchar_t *node_xpath)
{
	const xml_node node = _xml_doc->select_single_node(node_xpath).node();
	if (node.empty())
		return FALSE;

	xml_node parent_node = node.parent();
	if (parent_node.empty())
		return FALSE;

	parent_node.remove_child(node);
	return TRUE;
}