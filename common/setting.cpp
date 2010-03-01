#include "stdafx.h"
#include "setting.h"
#include <fstream>
using namespace std;

_gdimm_setting::_gdimm_setting()
{
	setting_map gdimm_default;
	gdimm_default[L"auto_hinting"]		= L"1";
	gdimm_default[L"bold_strength"]		= L"0.0";
	gdimm_default[L"embedded_bitmap"]	= L"0";
	gdimm_default[L"freetype_loader"]	= L"1";
	gdimm_default[L"hinting"]			= L"1";
	gdimm_default[L"lcd_filter"]		= L"1";
	gdimm_default[L"light_mode"]		= L"1";
	gdimm_default[L"max_height"]		= L"72";
	gdimm_default[L"render_mono"]		= L"0";
	gdimm_default[L"subpixel_render"]	= L"1";

	_setting_branchs.push_back(gdimm_default);
	_branch_names[L"common"] = &_setting_branchs.back();
}

void _gdimm_setting::load_branch(const pugi::xml_document &xml_doc, const char *xpath)
{
	int name_len;

	pugi::xpath_node_set target_node_set = xml_doc.select_nodes(xpath);
	for (pugi::xpath_node_set::const_iterator set_iter = target_node_set.begin(); set_iter != target_node_set.end(); set_iter++)
	{
		pugi::xml_attribute node_attr = set_iter->node().attribute("name");
		const char *mb_name;
		if (node_attr.empty())
			mb_name = set_iter->node().name();
		else
			mb_name = node_attr.value();

		name_len = MultiByteToWideChar(CP_UTF8, 0, mb_name, -1, NULL, 0);
		WCHAR *branch_name = new WCHAR[name_len];
		name_len = MultiByteToWideChar(CP_UTF8, 0, mb_name, -1, branch_name, name_len);
		assert(name_len != 0);

		// create new setting branch if necessary, otherwise use the existing
		branch_map::const_iterator branch_iter = _branch_names.find(branch_name);
		setting_map *branch_setting;

		if (branch_iter == _branch_names.end())
		{
			_setting_branchs.push_back(setting_map());
			branch_setting = &_setting_branchs.back();
			_branch_names[branch_name] = branch_setting;
		}
		else
			branch_setting = branch_iter->second;

		delete[] branch_name;
		
		for (pugi::xml_node_iterator setting_iter = set_iter->node().begin(); setting_iter != set_iter->node().end(); setting_iter++)
		{
			name_len = MultiByteToWideChar(CP_UTF8, 0, setting_iter->name(), -1, NULL, 0);
			WCHAR *setting_name = new WCHAR[name_len];
			name_len = MultiByteToWideChar(CP_UTF8, 0, setting_iter->name(), -1, setting_name, name_len);
			assert(name_len != 0);

			name_len = MultiByteToWideChar(CP_UTF8, 0, setting_iter->first_child().value(), -1, NULL, 0);
			WCHAR *setting_value = new WCHAR[name_len];
			name_len = MultiByteToWideChar(CP_UTF8, 0, setting_iter->first_child().value(), -1, setting_value, name_len);
			assert(name_len != 0);

			(*branch_setting)[setting_name] = setting_value;

			delete[] setting_name;
			delete[] setting_value;
		}
	}
}

void _gdimm_setting::load_exclude(const pugi::xml_document &xml_doc)
{
	int name_len;

	pugi::xpath_node_set target_node_set = xml_doc.select_nodes("/gdipp/exclude/*");
	for (pugi::xpath_node_set::const_iterator iter = target_node_set.begin(); iter != target_node_set.end(); iter++)
	{
		name_len = MultiByteToWideChar(CP_UTF8, 0, iter->node().first_child().value(), -1, NULL, 0);
		WCHAR *name = new WCHAR[name_len];
		name_len = MultiByteToWideChar(CP_UTF8, 0, iter->node().first_child().value(), -1, name, name_len);
		assert(name_len != 0);

		_exclude_names.insert(name);

		delete[] name;
	}
}

bool _gdimm_setting::load_settings(HMODULE h_module)
{
	// get setting file path
	WCHAR setting_path[MAX_PATH];
	get_dir_file_path(setting_path, L"setting.xml", h_module);

	ifstream f(setting_path);
	if (f.bad())
		return false;

	pugi::xml_document xml_doc;
	pugi::xml_parse_result xml_ret = xml_doc.load(f);
	assert(xml_ret);

	if (h_module != NULL)
	{
		load_branch(xml_doc, "/gdipp/gdimm/common");
		load_branch(xml_doc, "/gdipp/gdimm/process");
		load_branch(xml_doc, "/gdipp/gdimm/font");
	}

	load_exclude(xml_doc);

	f.close();
	return true;
}