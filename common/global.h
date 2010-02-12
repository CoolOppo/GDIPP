#pragma once

#include <string>
using namespace std;

typedef basic_string<TCHAR> t_string;

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

void debug_output_process_name();
void debug_output(const TCHAR *str = TEXT(""));
void debug_output(const TCHAR *str, unsigned int c);
void debug_output(const void *ptr, unsigned int size);
void debug_output(DWORD num);