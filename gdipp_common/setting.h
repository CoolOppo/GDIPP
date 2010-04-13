#pragma once

#include "gdipp_common.h"
using namespace std;

class gdipp_setting
{
	// setting names are case-insensitive
	typedef map<const wstring, wstring, wstring_ci_less> setting_map;
	map<BSTR, int> qqq;
	typedef list<pair<const wstring, setting_map>> gdimm_list;

	setting_map _process_setting;
	gdimm_list _gdimm_font;
	setting_map _demo_setting;
	setting_map _service_setting;
	vector<const wstring> _demo_font;
	list<const wstring> _exclude_process;

	WCHAR _process_name[MAX_PATH];

	void parse_gdimm_setting_node(const CComPtr<IXMLDOMNode> setting_node, setting_map &setting_store);
	void load_gdimm_process(const CComPtr<IXMLDOMNodeList> process_nodes);
	void load_gdimm_font(const CComPtr<IXMLDOMNodeList> font_nodes);
	void load_demo(const CComPtr<IXMLDOMNode> root_node);
	void load_service(const CComPtr<IXMLDOMNode> root_node);
	void load_exclude(const CComPtr<IXMLDOMNode> root_node);

public:
	gdipp_setting();

	bool init(HMODULE h_module);

	const WCHAR *get_gdimm_setting(const WCHAR *setting_name, const WCHAR *font_name) const;
	const WCHAR *get_demo_setting(const WCHAR *setting_name) const;
	const vector<const wstring> &get_demo_font() const;
	const WCHAR *get_service_setting(const WCHAR *setting_name) const;
	bool is_process_excluded(const WCHAR *proc_name) const;
};