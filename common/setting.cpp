#include "stdafx.h"
#include "setting.h"
#include <shlwapi.h>
#include <fstream>
#include <sstream>
using namespace std;

_gdimm_setting::shadow_setting::shadow_setting()
{
	// default setting
	offset_x = 1;
	offset_y = 1;
	alpha = 32;
}

_gdimm_setting::setting_items::setting_items()
{
	// default setting
	auto_hinting = true;
	bold_strength = 0.0;
	embedded_bitmap = false;
	freetype_loader = true;
	hinting = true;
	lcd_filter = 1;
	light_mode = true;
	max_height = 72;
	render_mono = false;
	shadow = shadow_setting();
	subpixel_render = true;
}

_gdimm_setting::_gdimm_setting()
{
	DWORD dw_ret;

	_setting_branchs[COMMON_BRANCH_NAME] = setting_items();

	dw_ret = GetModuleFileNameW(NULL, _process_name, MAX_PATH);
	assert(dw_ret != 0);

	PathStripPathW(_process_name);
}

void _gdimm_setting::load_settings(const xml_node &context_node, const setting_items &default_items, setting_items &settings)
{
	evaluate_to_number(context_node, "auto_hinting", default_items.auto_hinting, settings.auto_hinting);
	evaluate_to_number(context_node, "bold_strength", default_items.bold_strength, settings.bold_strength);
	evaluate_to_number(context_node, "freetype_loader", default_items.freetype_loader, settings.freetype_loader);
	evaluate_to_number(context_node, "hinting", default_items.hinting, settings.hinting);
	evaluate_to_number(context_node, "lcd_filter", default_items.lcd_filter, settings.lcd_filter);
	evaluate_to_number(context_node, "light_mode", default_items.light_mode, settings.light_mode);
	evaluate_to_number(context_node, "max_height", default_items.max_height, settings.max_height);
	evaluate_to_number(context_node, "render_mono", default_items.render_mono, settings.render_mono);
	evaluate_to_number(context_node, "subpixel_render", default_items.subpixel_render, settings.subpixel_render);

	evaluate_to_number(context_node, "shadow/offset_x", default_items.shadow.offset_x, settings.shadow.offset_x);
	evaluate_to_number(context_node, "shadow/offset_y", default_items.shadow.offset_y, settings.shadow.offset_y);
	evaluate_to_number(context_node, "shadow/alpha", default_items.shadow.alpha, settings.shadow.alpha);
}

void _gdimm_setting::load_common(const xml_node &context_node)
{
	const xml_node common_node = context_node.select_single_node("common").node();

	// the default items should never be used
	load_settings(common_node, _setting_branchs[COMMON_BRANCH_NAME], _setting_branchs[COMMON_BRANCH_NAME]);
}

void _gdimm_setting::load_branchs(const xml_node &context_node, const setting_items &default_items, const char *xpath)
{
	xpath_node_set target_node_set = context_node.select_nodes(xpath);
	for (xpath_node_set::const_iterator set_iter = target_node_set.begin(); set_iter != target_node_set.end(); set_iter++)
	{
		xml_attribute node_attr = set_iter->node().attribute("name");
		const char *mb_name;
		if (node_attr.empty())
			mb_name = set_iter->node().name();
		else
			mb_name = node_attr.value();

		int name_len = MultiByteToWideChar(CP_UTF8, 0, mb_name, -1, NULL, 0);
		WCHAR *branch_name = new WCHAR[name_len];
		name_len = MultiByteToWideChar(CP_UTF8, 0, mb_name, -1, branch_name, name_len);
		assert(name_len != 0);

		// create new setting branch if necessary, otherwise use the existing
		setting_map::const_iterator branch_iter = _setting_branchs.find(branch_name);

		if (branch_iter == _setting_branchs.end())
			_setting_branchs[branch_name] = setting_items();

		load_settings(set_iter->node(), default_items, _setting_branchs[branch_name]);

		delete[] branch_name;
	}
}

void _gdimm_setting::load_exclude(const xml_node &context_node)
{
	int name_len;

	xpath_node_set target_node_set = context_node.select_nodes("/gdipp/exclude/*");
	for (xpath_node_set::const_iterator iter = target_node_set.begin(); iter != target_node_set.end(); iter++)
	{
		name_len = MultiByteToWideChar(CP_UTF8, 0, iter->node().first_child().value(), -1, NULL, 0);
		WCHAR *name = new WCHAR[name_len];
		name_len = MultiByteToWideChar(CP_UTF8, 0, iter->node().first_child().value(), -1, name, name_len);
		assert(name_len != 0);

		_exclude_names.insert(name);

		delete[] name;
	}
}

bool _gdimm_setting::init(HMODULE h_module)
{
	// get setting file path
	WCHAR setting_path[MAX_PATH];
	get_dir_file_path(h_module, L"setting.xml", setting_path);

	ifstream f(setting_path);
	if (f.bad())
		return false;

	xml_document xml_doc;
	xml_parse_result xml_ret = xml_doc.load(f);
	assert(xml_ret);

	// if the setting is needed by gdimm, the module handle is not NULL
	if (h_module != NULL)
	{
		const xml_node gdimm_node = xml_doc.select_single_node("/gdipp/gdimm").node();
		load_common(gdimm_node);

		const setting_items &common_items = _setting_branchs[COMMON_BRANCH_NAME];
		load_branchs(gdimm_node, common_items, "process");

		if (_setting_branchs.find(_process_name) == _setting_branchs.end())
			load_branchs(gdimm_node, common_items, "font");
		else
			load_branchs(gdimm_node, _setting_branchs[_process_name], "font");
	}

	load_exclude(xml_doc);

	f.close();
	return true;
}

const _gdimm_setting::setting_items &_gdimm_setting::get_setting_items(const WCHAR *branch_name) const
{
	setting_map::const_iterator branch_iter = _setting_branchs.find(branch_name);

	// if not exist the specified branch, fallback to current process name branch and finally the common branch
	if (branch_iter == _setting_branchs.end())
	{
		branch_iter = _setting_branchs.find(_process_name);
		if (branch_iter == _setting_branchs.end())
			branch_iter = _setting_branchs.find(COMMON_BRANCH_NAME);
	}

	return branch_iter->second;
}