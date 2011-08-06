#include "stdafx.h"
#include "gdipp_lib.h"
#include "gdipp_lib/config.h"
#include "gdipp_lib/config_cache.h"
#include "gdipp_lib/minidump.h"

namespace gdipp
{

BOOL get_dir_file_path(HMODULE h_module, const wchar_t *file_name, wchar_t *out_path)
{
	// append the file name after the module's resident directory name
	// if the module handle is NULL, use the current exe as the module

	DWORD dw_ret;
	BOOL b_ret;

	dw_ret = GetModuleFileNameW(h_module, out_path, MAX_PATH);
	if (dw_ret == 0)
		return FALSE;

	b_ret = PathRemoveFileSpecW(out_path);
	if (!b_ret)
		return FALSE;

	return PathAppendW(out_path, file_name);
}

config config_instance;
config_cache config_cache_instance;

void init_config()
{
	return config_instance.init_config();
}

BOOL load_config(const wchar_t *config_path)
{
	return config_instance.load_config(config_path);
}

BOOL save_config(const wchar_t *config_path)
{
	return config_instance.save_config(config_path);
}

BOOL insert_setting(const wchar_t *node_name, const wchar_t *node_text, const wchar_t *parent_xpath, const wchar_t *ref_node_xpath, std::wstring &new_node_xpath)
{
	return config_instance.insert_setting(node_name, node_text, parent_xpath, ref_node_xpath, new_node_xpath);
}

BOOL set_setting_attr(const wchar_t *node_xpath, const wchar_t *attr_name, const wchar_t *attr_value)
{
	return config_instance.set_setting_attr(node_xpath, attr_name, attr_value);
}

BOOL remove_setting(const wchar_t *node_xpath)
{
	return config_instance.remove_setting(node_xpath);
}

const wchar_t *get_gdimm_setting(const wchar_t *setting_name, const config_trait *setting_trait)
{
	return config_instance.get_gdimm_setting(setting_name, setting_trait);
}

const font_setting_cache *get_gdimm_setting_cache(const config_trait *setting_trait)
{
	return config_cache_instance.lookup(setting_trait);
}

const wchar_t *get_demo_setting(const wchar_t *setting_name)
{
	return config_instance.get_demo_setting(setting_name);
}

const std::vector<const std::wstring> &get_demo_fonts()
{
	return config_instance.get_demo_fonts();
}

const wchar_t *get_service_setting(const wchar_t *setting_name)
{
	return config_instance.get_service_setting(setting_name);
}

bool is_process_excluded(const wchar_t *proc_name)
{
	return config_instance.is_process_excluded(proc_name);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		register_minidump_module(hModule);
		break;
	}

	return TRUE;
}

}
