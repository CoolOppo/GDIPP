#pragma once

#include "global.h"
#include <pugixml.hpp>
#include <list>
#include <map>
#include <set>
using namespace pugi;
using namespace std;

#define COMMON_BRANCH_NAME L"common"

class _gdimm_setting
{
public:
	struct shadow_setting
	{
		LONG offset_x;
		LONG offset_y;
		BYTE alpha;

		shadow_setting();
	};

	struct setting_items
	{
		bool auto_hinting;
		float bold_strength;
		bool embedded_bitmap;
		bool freetype_loader;
		bool hinting;
		BYTE lcd_filter;
		bool light_mode;
		LONG max_height;
		bool render_mono;
		shadow_setting shadow;
		bool subpixel_render;

		setting_items();
	};

private:
	// map keys are case-insensitive
	typedef map<const wstring, setting_items, string_ci_less> setting_map;

	setting_map _setting_branchs;
	set<const wstring, string_ci_less> _exclude_names;
	WCHAR _process_name[MAX_PATH];

	template <typename T>
	void evaluate_to_number(const xml_node &context_node, const char *expression, const T& default_value, T &out_value)
	{
		xpath_query query(expression);
		if (query.evaluate_boolean(context_node))
			out_value = query.evaluate_number(context_node);
		else
			out_value = default_value;
	}

	void load_settings(const xml_node &context_node, const setting_items &default_items, setting_items &settings);
	void load_common(const xml_node &context_node);
	void load_branchs(const xml_node &context_node, const setting_items &default_items, const char *xpath);
	void load_exclude(const xml_node &context_node);

public:
	_gdimm_setting();
	bool init(HMODULE h_module);
	bool is_name_excluded(const WCHAR *name) const
	{ return (_exclude_names.find(name) != _exclude_names.end()); }

	const setting_items &get_setting_items(const WCHAR *branch_name = COMMON_BRANCH_NAME) const;
};

typedef singleton<_gdimm_setting> gdimm_setting;