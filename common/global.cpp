#include "stdafx.h"
#include "global.h"

CRITICAL_SECTION critical_section::_cs;

#define debug_file_name "C:\\gdimm_debug.txt"

void debug_output(const TCHAR *str)
{
#ifdef _DEBUG
	FILE *f;
	fopen_s(&f, debug_file_name, "a+");
	fwprintf(f, TEXT("%s\n"), str);
	fclose(f);
#endif
}

void debug_output(const TCHAR *str, unsigned int c)
{
#ifdef _DEBUG
	FILE *f;
	fopen_s(&f, debug_file_name, "a+");
	fwrite(str, sizeof(TCHAR), c, f);
	fwprintf(f, TEXT("\n"));
	fclose(f);
#endif
}

void debug_output(const void *ptr, unsigned int size)
{
#ifdef _DEBUG
	FILE *f;
	fopen_s(&f, debug_file_name, "ab+");
	fwrite(ptr, 1, size, f);
	fclose(f);
#endif
}

void debug_output(DWORD num)
{
#ifdef _DEBUG
	FILE *f;
	fopen_s(&f, debug_file_name, "a+");
	fwprintf(f, TEXT("%u\n"), num);
	fclose(f);
#endif
}

void debug_output_process_name()
{
#ifdef _DEBUG
	HANDLE h_proc = GetCurrentProcess();
	DWORD name_size = MAX_PATH;
	TCHAR name_str[MAX_PATH];
	QueryFullProcessImageName(h_proc, 0, name_str, &name_size);

	debug_output(name_str, name_size);
#endif
}