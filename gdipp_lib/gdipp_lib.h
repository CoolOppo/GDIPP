#pragma once

namespace gdipp
{

#ifdef GDIPP_EXPORTS
#define GDIPP_API __declspec(dllexport)
#else
#define GDIPP_API __declspec(dllimport)
#endif // GDIPP_EXPORTS

struct font_setting_cache;
class config_trait;

GDIPP_API BOOL get_dir_file_path(HMODULE h_module, const wchar_t *file_name, wchar_t *out_path);

GDIPP_API void init_config();
GDIPP_API BOOL load_config(const wchar_t *config_path);
GDIPP_API BOOL save_config(const wchar_t *config_path);
GDIPP_API BOOL insert_setting(const wchar_t *node_name, const wchar_t *node_text, const wchar_t *parent_xpath, const wchar_t *ref_node_xpath, std::wstring &new_node_xpath);
GDIPP_API BOOL set_setting_attr(const wchar_t *node_xpath, const wchar_t *attr_name, const wchar_t *attr_value);
GDIPP_API BOOL remove_setting(const wchar_t *node_xpath);

GDIPP_API const wchar_t *get_gdimm_setting(const wchar_t *setting_name, const config_trait *setting_trait);
GDIPP_API const font_setting_cache *get_gdimm_setting_cache(const config_trait *setting_trait);
GDIPP_API const wchar_t *get_demo_setting(const wchar_t *setting_name);
GDIPP_API const std::vector<const std::wstring> &get_demo_fonts();
GDIPP_API const wchar_t *get_service_setting(const wchar_t *setting_name);
GDIPP_API bool is_process_excluded(const wchar_t *proc_name);

}
