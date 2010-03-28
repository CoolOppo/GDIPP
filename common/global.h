#pragma once

using namespace std;

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

void get_dir_file_path(HMODULE h_module, const WCHAR *file_name, WCHAR source_path[MAX_PATH]);
const WCHAR *metric_family_name(const BYTE *metric_buf);
const WCHAR *metric_family_name(const OUTLINETEXTMETRICW *outline_metric);
const WCHAR *metric_face_name(const BYTE *metric_buf);
const WCHAR *metric_face_name(const OUTLINETEXTMETRICW *outline_metric);

template <typename T>
inline void wcs_convert(const WCHAR *str, T &converted)
{
	wistringstream(str) >> converted;
}

void debug_output(const WCHAR *str = L"");
void debug_output(const WCHAR *str, unsigned int c);
void debug_output(const void *ptr, unsigned int size);
void debug_output(long num);
void debug_output(DWORD num);