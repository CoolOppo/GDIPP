#pragma once

#include <list>
#include <map>
#include <vector>
#include "gdipp_support/helper.h"
#include "gdipp_lib/gdipp_lib.h"

namespace gdipp
{

class config
{
public:
	config();
	~config();

	const wchar_t *get_server_config(const wchar_t *setting_name, const config_trait *trait) const;
	const wchar_t *get_demo_config(const wchar_t *setting_name) const;
	const std::vector<const std::wstring> &get_demo_fonts() const;
	const wchar_t *get_server_hook_config(const wchar_t *setting_name) const;
	bool is_process_excluded(const wchar_t *proc_name) const;

	void init_config();
	void uninit_config();
	BOOL load_config(const wchar_t *config_path);
	BOOL save_config(const wchar_t *config_path);
	BOOL insert_setting(const wchar_t *node_name, const wchar_t *node_text, const wchar_t *parent_xpath, const wchar_t *ref_node_xpath, std::wstring &new_node_xpath);
	BOOL set_setting_attr(const wchar_t *node_xpath, const wchar_t *attr_name, const wchar_t *attr_value);
	BOOL remove_setting(const wchar_t *node_xpath);

private:
	// setting names are case-insensitive
	typedef std::map<const std::wstring, std::wstring, wstring_ci_less> config_map;

	struct conf_server_font_node
	{
		std::wstring name;
		char bold;
		char italic;
		LONG max_height;
		config_map configs;
	};

	void parse_server_config_node(const pugi::xml_node &setting_node, config_map &setting_store);
	void load_server_process(const pugi::xpath_node_set &process_nodes);
	void load_server_font(const pugi::xpath_node_set &font_nodes);
	void load_demo(const pugi::xml_node &root_node);
	void load_server_hook(const pugi::xml_node &root_node);
	void load_client_exclude(const pugi::xml_node &root_node);

	pugi::xml_document *_xml_doc;

	config_map _config_server_process;
	std::list<conf_server_font_node> _config_server_fonts;
	config_map _config_server_hook;
	std::list<const std::wstring> _config_client_excludes;
	config_map _config_demo;
	std::vector<const std::wstring> _demo_fonts;

	wchar_t _process_name[MAX_PATH];
};

}
