#include "stdafx.h"
#include "setting.h"
using namespace std::tr1;

bool mb_to_wc(const char *mb_str, wstring &wc_str)
{
	// convert multibyte string to widechar string

	// omit the trailing '\0'
	int wcs_len = MultiByteToWideChar(CP_UTF8, 0, mb_str, -1, NULL, 0) - 1;
	if (wcs_len == 0)
		return false;

	wc_str.resize(wcs_len);
	wcs_len = MultiByteToWideChar(CP_UTF8, 0, mb_str, -1, &wc_str[0], wcs_len);
	
	return (wcs_len != 0);
}

gdipp_setting::gdipp_setting()
{
	DWORD dw_ret;

	dw_ret = GetModuleFileNameW(NULL, _process_name, MAX_PATH);
	assert(dw_ret != 0);

	PathStripPathW(_process_name);
}

void gdipp_setting::parse_gdimm_setting_node(const xml_node setting_node, setting_map &setting_store)
{
	const string name = setting_node.name();

	if (name == "freetype" || name == "gamma" || name == "shadow")
	{
		// these settings have nested items

		for (xml_node::iterator iter = setting_node.begin(); iter != setting_node.end(); iter++)
		{
			wstring curr_value;
			mb_to_wc(iter->first_child().value(), curr_value);

			setting_store[name + "/" + iter->name()] = curr_value;
		}
	}
	else
	{
		wstring value;
		mb_to_wc(setting_node.first_child().value(), value);

		setting_store[name] = value;
	}	
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
		const xml_attribute name_attr = curr_proc.attribute("name");
		wstring proc_name;
		mb_to_wc(name_attr.value(), proc_name);

		const wregex name_ex(proc_name.data(), regex_constants::icase | regex_constants::nosubs | regex_constants::optimize);
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

		const xml_node curr_proc = node_iter->node();
		const xml_attribute name_attr = curr_proc.attribute("name");
		wstring proc_name;
		mb_to_wc(name_attr.value(), proc_name);

		_gdimm_font.push_back(pair<const wstring, setting_map>(proc_name, curr_settings));
	}
}

void gdipp_setting::load_demo(const xml_node &root_node)
{
	// it is OK if the root node is empty

	for (xml_node::iterator iter = root_node.begin(); iter != root_node.end(); iter++)
	{
		wstring curr_value;
		mb_to_wc(iter->first_child().value(), curr_value);

		if (_stricmp(iter->name(), "font") == 0)
			_demo_font.push_back(curr_value);
		else
			_demo_setting[iter->name()] = curr_value;
	}
}

void gdipp_setting::load_service(const xml_node &root_node)
{
	for (xml_node::iterator iter = root_node.begin(); iter != root_node.end(); iter++)
	{
		wstring curr_value;
		mb_to_wc(iter->first_child().value(), curr_value);

		_service_setting[iter->name()] = curr_value;
	}
}

void gdipp_setting::load_exclude(const xml_node &root_node)
{
	for (xml_node::iterator iter = root_node.begin(); iter != root_node.end(); iter++)
	{
		wstring curr_value;
		mb_to_wc(iter->first_child().value(), curr_value);

		if (strcmp(iter->name(), "font") == 0)
			_exclude_font.push_back(curr_value);
		else
			_exclude_process.push_back(curr_value);
	}
}

bool gdipp_setting::init(HMODULE h_module)
{
	// get setting file path
	WCHAR setting_path[MAX_PATH];
	get_dir_file_path(h_module, L"setting.xml", setting_path);

	ifstream f(setting_path);
	if (f.fail())
		return false;

	xml_document xml_doc;
	xml_parse_result xml_ret = xml_doc.load(f);
	assert(xml_ret);

	if (h_module == NULL)
	{
		// the setting is not requested by gdimm

		load_demo(xml_doc.select_single_node("/gdipp/demo").node());
		load_service(xml_doc.select_single_node("/gdipp/service").node());
	}
	else
	{
		// the setting is requested by gdimm

		load_gdimm_process(xml_doc.select_nodes("/gdipp/gdimm/process"));
		load_gdimm_font(xml_doc.select_nodes("gdipp/gdimm/font"));
	}

	const xml_node exclude_node = xml_doc.select_single_node("/gdipp/exclude").node();
	load_exclude(exclude_node);

	f.close();
	return true;
}

const WCHAR *gdipp_setting::get_gdimm_setting(const char *setting_name, const WCHAR *font_name) const
{
	// check setting for the current process
	setting_map::const_iterator setting_iter = _process_setting.find(setting_name);
	if (setting_iter != _process_setting.end())
		return setting_iter->second.c_str();

	// check setting for the specified font
	for (gdimm_list::const_iterator list_iter = _gdimm_font.begin(); list_iter != _gdimm_font.end(); list_iter++)
	{
		const wregex name_ex(list_iter->first.data(), regex_constants::icase | regex_constants::nosubs | regex_constants::optimize);
		if (regex_match(font_name, name_ex))
		{
			setting_iter = list_iter->second.find(setting_name);
			if (setting_iter != list_iter->second.end())
				return setting_iter->second.c_str();
		}
	}

	return NULL;
}

const WCHAR *gdipp_setting::get_demo_setting(const char *setting_name) const
{
	setting_map::const_iterator iter = _demo_setting.find(setting_name);

	if (iter == _demo_setting.end())
		return NULL;
	else
		return iter->second.c_str();
}

const vector<const wstring> &gdipp_setting::get_demo_font() const
{
	return _demo_font;
}

const WCHAR *gdipp_setting::get_service_setting(const char *setting_name) const
{
	setting_map::const_iterator iter = _service_setting.find(setting_name);

	if (iter == _service_setting.end())
		return NULL;
	else
		return iter->second.c_str();
}

bool gdipp_setting::is_process_excluded(const WCHAR *proc_name) const
{
	// if no process name is specified, return true if the current process is excluded
	// otherwise, return true if the specified process is excluded

	const WCHAR *final_name;
	if (proc_name == NULL)
		final_name = _process_name;
	else
		final_name = proc_name;

	for (list<const wstring>::const_iterator iter = _exclude_process.begin(); iter != _exclude_process.end(); iter++)
	{
		const wregex name_ex(iter->data(), regex_constants::icase | regex_constants::nosubs | regex_constants::optimize);
		if (regex_match(final_name, name_ex))
			return true;
	}

	return false;
}

bool gdipp_setting::is_font_excluded(const WCHAR *font_name) const
{
	for (list<const wstring>::const_iterator iter = _exclude_font.begin(); iter != _exclude_font.end(); iter++)
	{
		const wregex name_ex(iter->data(), regex_constants::icase | regex_constants::nosubs | regex_constants::optimize);
		if (regex_match(font_name, name_ex))
			return true;
	}

	return false;
}