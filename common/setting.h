#pragma once

#include "global.h"
#include <pugixml.hpp>
#include <map>
#include <set>
using namespace pugi;
using namespace std;

#define GDIMM_COMMON_NAME "common"

struct gdimm_setting_items
{
	struct gamma_setting
	{
		double gray;
		double red;
		double green;
		double blue;

		gamma_setting();
	};

	struct shadow_setting
	{
		LONG offset_x;
		LONG offset_y;
		BYTE alpha;

		shadow_setting();
	};

	bool auto_hinting;
	bool embedded_bitmap;
	float embolden;
	bool freetype_loader;
	gamma_setting gamma;
	bool hinting;
	BYTE lcd_filter;
	bool light_mode;
	LONG max_height;
	bool render_mono;
	bool render_non_aa;
	shadow_setting shadow;
	bool subpixel_render;
	bool zero_alpha;

	gdimm_setting_items();
};

struct service_setting_items
{
	WCHAR *poll_interval;

	service_setting_items();
};

class _gdipp_setting
{
	// map keys are case-insensitive
	typedef map<const wstring, gdimm_setting_items, string_ci_less> gdimm_map;

	gdimm_map _gdimm_branches;
	service_setting_items _service_setting;
	set<const wstring, string_ci_less> _exclude_names;
	WCHAR _process_name[MAX_PATH];

	template <typename T>
	void evaluate_to_number(const xml_node &context_node, const char *expression, const T &default_value, T &out_value)
	{
		xpath_query query(expression);
		if (query.evaluate_boolean(context_node))
			out_value = (T) query.evaluate_number(context_node);
		else
			out_value = default_value;
	}

	void load_settings(const xml_node &context_node, const gdimm_setting_items &default_items, gdimm_setting_items &settings);
	void load_common(const xml_node &context_node);
	void load_branchs(const xml_node &context_node, const gdimm_setting_items &default_items, const char *xpath);
	void load_exclude(const xml_node &context_node);

public:
	_gdipp_setting();
	bool init(HMODULE h_module);
	bool is_name_excluded(const WCHAR *name) const;

	const gdimm_setting_items &get_gdimm_items(const WCHAR *font_family = TEXT(GDIMM_COMMON_NAME)) const;
	const service_setting_items &get_service_items() const
	{ return _service_setting; }
};

typedef singleton<_gdipp_setting> gdipp_setting;