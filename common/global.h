#pragma once

#include <string>
using namespace std;

#define FT_PAD_FLOOR( x, n )  ( (x) & ~((n)-1) )
#define FT_PAD_ROUND( x, n )  FT_PAD_FLOOR( (x) + ((n)/2), n )
#define FT_PAD_CEIL( x, n )   FT_PAD_FLOOR( (x) + ((n)-1), n )

template <typename T>
class singleton
{
	singleton() {};                              // Private constructor
	singleton(const singleton&);                 // Prevent copy-construction
	singleton& operator=(const singleton&);      // Prevent assignment

public:
	static T &instance()
	{
		static T _instance;
		return _instance;
	}
};

class critical_section
{
	static CRITICAL_SECTION _cs;

public:
	critical_section()
	{
		EnterCriticalSection(&_cs);
	}

	~critical_section()
	{
		LeaveCriticalSection(&_cs);
	}

	static void initialize()
	{
		InitializeCriticalSection(&_cs);
	}

	static void release()
	{
		DeleteCriticalSection(&_cs);
	}
};

struct string_ci_less
{
	bool operator()(const wstring &string1, const wstring &string2) const
	{ return _wcsicmp(string1.c_str(), string2.c_str()) < 0; }
};

/*
GDIPP_SERVICE: gdimm.dll monitors the injector process, and unload itself once the injector process is terminated
GDIPP_LOADER: gdimm.dll does not care about the injector
*/
enum INJECTOR_TYPE
{
	GDIPP_SERVICE,
	GDIPP_LOADER
};

void get_dir_file_path(WCHAR source_path[MAX_PATH], const WCHAR *file_name, HMODULE h_module = NULL);

void debug_output_process_name();
void debug_output(const WCHAR *str = L"");
void debug_output(const WCHAR *str, unsigned int c);
void debug_output(const void *ptr, unsigned int size);
void debug_output(long num);
void debug_output(DWORD num);