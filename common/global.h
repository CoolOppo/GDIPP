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

void debug_output(const TCHAR *str = TEXT(""));
void debug_output(const TCHAR *str, unsigned int c);
void debug_output(DWORD num);