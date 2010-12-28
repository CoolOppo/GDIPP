#pragma once

#include <setting_trait.h>

using namespace std;

#ifdef GDIPP_EXPORTS
#define GDIPP_API __declspec(dllexport)
#else
#define GDIPP_API __declspec(dllimport)
#endif // GDIPP_EXPORTS

GDIPP_API BOOL gdipp_get_dir_file_path(HMODULE h_module, const wchar_t *file_name, wchar_t *out_path);

GDIPP_API void gdipp_init_setting();
GDIPP_API BOOL gdipp_load_setting(const wchar_t *setting_path);
GDIPP_API BOOL gdipp_save_setting(const wchar_t *setting_path);
GDIPP_API BOOL gdipp_insert_setting(const wchar_t *node_name, const wchar_t *node_text, const wchar_t *parent_xpath, const wchar_t *ref_node_xpath, wstring &new_node_xpath);
GDIPP_API BOOL gdipp_set_setting_attr(const wchar_t *node_xpath, const wchar_t *attr_name, const wchar_t *attr_value);
GDIPP_API BOOL gdipp_remove_setting(const wchar_t *node_xpath);

GDIPP_API const wchar_t *gdipp_get_gdimm_setting(const wchar_t *setting_name, const gdimm_setting_trait *setting_trait);
GDIPP_API const wchar_t *gdipp_get_demo_setting(const wchar_t *setting_name);
GDIPP_API const vector<const wstring> &gdipp_get_demo_fonts();
GDIPP_API const wchar_t *gdipp_get_service_setting(const wchar_t *setting_name);
GDIPP_API bool gdipp_is_process_excluded(const wchar_t *proc_name);