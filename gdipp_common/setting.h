#pragma once

#include "gdipp_common.h"
using namespace std;
using namespace pugi;

class gdipp_setting
{
	// setting names are case-insensitive
	typedef map<const wstring, wstring, wstring_ci_less> setting_map;
	typedef list<pair<const wstring, setting_map>> gdimm_list;

	xml_document *_xml_doc;

	setting_map _process_setting;
	gdimm_list _gdimm_font;
	setting_map _demo_setting;
	setting_map _service_setting;
	vector<const wstring> _demo_fonts;
	list<const wstring> _exclude_process;

	wchar_t _process_name[MAX_PATH];

	void parse_gdimm_setting_node(const xml_node &setting_node, setting_map &setting_store);
	void load_gdimm_process(const xpath_node_set &process_nodes);
	void load_gdimm_font(const xpath_node_set &font_nodes);
	void load_demo(const xml_node &root_node);
	void load_service(const xml_node &root_node);
	void load_exclude(const xml_node &root_node);

public:
	gdipp_setting();
	~gdipp_setting();

	const wchar_t *get_gdimm_setting(const wchar_t *setting_name, const wchar_t *font_name) const;
	const wchar_t *get_demo_setting(const wchar_t *setting_name) const;
	const vector<const wstring> &get_demo_fonts() const;
	const wchar_t *get_service_setting(const wchar_t *setting_name) const;
	bool is_process_excluded(const wchar_t *proc_name) const;

	void init_setting();
	void uninit_setting();
	BOOL load_setting(const wchar_t *setting_path);
	BOOL save_setting(const wchar_t *setting_path);
	BOOL insert_setting(const wchar_t *node_name, const wchar_t *node_text, const wchar_t *parent_xpath, const wchar_t *ref_node_xpath, wstring &new_node_xpath);
	BOOL set_setting_attr(const wchar_t *node_xpath, const wchar_t *attr_name, const wchar_t *attr_value);
	BOOL remove_setting(const wchar_t *node_xpath);
};