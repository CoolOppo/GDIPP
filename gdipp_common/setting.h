#pragma once

#include "gdipp_common.h"
using namespace std;
using namespace pugi;

class gdipp_setting
{
	// setting names are case-insensitive
	typedef map<const string, wstring, string_ci_less> setting_map;
	typedef list<pair<const wstring, setting_map>> gdimm_list;

	setting_map _process_setting;
	gdimm_list _gdimm_font;
	setting_map _demo_setting;
	setting_map _service_setting;
	vector<const wstring> _demo_font;
	list<const wstring> _exclude_process;
	list<const wstring> _exclude_font;

	WCHAR _process_name[MAX_PATH];

	void parse_gdimm_setting_node(const xml_node setting_node, setting_map &setting_store);
	void load_gdimm_process(const xpath_node_set &process_nodes);
	void load_gdimm_font(const xpath_node_set &font_nodes);
	void load_demo(const xml_node &root_node);
	void load_service(const xml_node &root_node);
	void load_exclude(const xml_node &root_node);

public:
	gdipp_setting();
	bool init(HMODULE h_module);

	const WCHAR *get_gdimm_setting(const char *setting_name, const WCHAR *font_name) const;
	const WCHAR *get_demo_setting(const char *setting_name) const;
	const vector<const wstring> &get_demo_font() const;
	const WCHAR *get_service_setting(const char *setting_name) const;
	bool is_process_excluded(const WCHAR *proc_name) const;
};