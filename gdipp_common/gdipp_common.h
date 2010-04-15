#pragma once

using namespace std;

#ifdef GDIPP_EXPORTS
#define GDIPP_API __declspec(dllexport)
#else
#define GDIPP_API __declspec(dllimport)
#endif // GDIPP_EXPORTS

#define FT_PAD_FLOOR( x, n )  ( (x) & ~((n)-1) )
#define FT_PAD_ROUND( x, n )  FT_PAD_FLOOR( (x) + ((n)/2), n )
#define FT_PAD_CEIL( x, n )   FT_PAD_FLOOR( (x) + ((n)-1), n )

struct wstring_ci_less
{
	bool operator()(const wstring &string1, const wstring &string2) const
	{ return _wcsicmp(string1.c_str(), string2.c_str()) < 0; }
};

/*
GDIPP_SERVICE: gdimm.dll monitors the injector process, and unload itself once the injector process is terminated
GDIPP_LOADER: gdimm.dll does not care about the injector
*/
enum GDIPP_INJECTOR_TYPE
{
	GDIPP_SERVICE,
	GDIPP_LOADER
};

struct gdipp_inject_payload
{
	GDIPP_INJECTOR_TYPE inject_type;
	wchar_t svc_event_name[MAX_PATH]; 
};

GDIPP_API void gdipp_register_module(HMODULE h_module);
GDIPP_API BOOL gdipp_get_dir_file_path(HMODULE h_module, const wchar_t *file_name, wchar_t *source_path);

// setting wrapper APIs
// gdipp_setting uses STL templates, which makes it frustrating to export
GDIPP_API BOOL gdipp_init_setting();
GDIPP_API void gdipp_uninit_setting();
GDIPP_API BOOL gdipp_load_setting(const wchar_t *setting_path);
GDIPP_API BOOL gdipp_save_setting(const wchar_t *setting_path);
GDIPP_API BOOL gdipp_insert_setting(const wchar_t *node_name, const wchar_t *node_value, const wchar_t *ref_node_xpath);
GDIPP_API BOOL gdipp_set_setting_attr(const wchar_t *node_xpath, const wchar_t *attr_name, const wchar_t *attr_value);
GDIPP_API BOOL gdipp_remove_setting_item(const wchar_t *node_xpath);

GDIPP_API const wchar_t *gdipp_get_gdimm_setting(const wchar_t *setting_name, const wchar_t *font_name);
GDIPP_API const wchar_t *gdipp_get_demo_setting(const wchar_t *setting_name);
GDIPP_API const vector<const wstring> &gdipp_get_demo_fonts();
GDIPP_API const wchar_t *gdipp_get_service_setting(const wchar_t *setting_name);
GDIPP_API bool gdipp_is_process_excluded(const wchar_t *proc_name);

// convert a string to template value if possible
// helper function to convert raw setting strings to values
template <typename T>
void wcs_convert(const wchar_t *str, T *converted)
{
	if (str != NULL)
		wistringstream(str) >> *converted;
}

// debug APIs
GDIPP_API void gdipp_debug_output(const wchar_t *str = L"");
GDIPP_API void gdipp_debug_output(const wchar_t *str, unsigned int c);
GDIPP_API void gdipp_debug_output(const void *ptr, unsigned int size);
GDIPP_API void gdipp_debug_output(long num);
GDIPP_API void gdipp_debug_output(DWORD num);