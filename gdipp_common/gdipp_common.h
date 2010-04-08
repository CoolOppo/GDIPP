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

/*
GDIPP_SERVICE: gdimm.dll monitors the injector process, and unload itself once the injector process is terminated
GDIPP_LOADER: gdimm.dll does not care about the injector
*/
enum INJECTOR_TYPE
{
	GDIPP_SERVICE,
	GDIPP_LOADER
};

struct inject_payload
{
	INJECTOR_TYPE inject_type;
	WCHAR svc_event_name[MAX_PATH]; 
};

struct string_ci_less
{
	bool operator()(const string &string1, const string &string2) const
	{ return _stricmp(string1.c_str(), string2.c_str()) < 0; }
};

struct wstring_ci_less
{
	bool operator()(const wstring &string1, const wstring &string2) const
	{ return _wcsicmp(string1.c_str(), string2.c_str()) < 0; }
};

EXTERN_C GDIPP_API inline void register_module(HMODULE h_module);
EXTERN_C GDIPP_API inline void get_dir_file_path(HMODULE h_module, const WCHAR *file_name, WCHAR source_path[MAX_PATH]);

// setting APIs
EXTERN_C GDIPP_API inline const WCHAR *get_gdimm_setting(const char *setting_name, const WCHAR *font_name);
EXTERN_C GDIPP_API inline const WCHAR *get_demo_setting(const char *setting_name);
EXTERN_C GDIPP_API inline const vector<const wstring> &get_demo_font();
EXTERN_C GDIPP_API inline const WCHAR *get_service_setting(const char *setting_name);
EXTERN_C GDIPP_API inline bool is_process_excluded(const WCHAR *proc_name);
EXTERN_C GDIPP_API inline bool is_font_excluded(const WCHAR *font_name);

// convert a string to template value if possible
// helper function to convert raw setting strings to values
template <typename T>
inline void wcs_convert(const WCHAR *str, T &converted)
{
	if (str != NULL)
		wistringstream(str) >> converted;
}

// debug APIs
EXTERN_C GDIPP_API inline LONG CALLBACK create_minidump(__in struct _EXCEPTION_POINTERS *ExceptionInfo);
GDIPP_API inline void debug_output(const WCHAR *str = L"");
GDIPP_API inline void debug_output(const WCHAR *str, unsigned int c);
GDIPP_API inline void debug_output(const void *ptr, unsigned int size);
GDIPP_API inline void debug_output(long num);
GDIPP_API inline void debug_output(DWORD num);