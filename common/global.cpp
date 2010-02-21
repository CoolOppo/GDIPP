#include "stdafx.h"
#include "global.h"
#include <shlwapi.h>
#include <cstdio>

CRITICAL_SECTION critical_section::_cs;

void get_dir_file_path(WCHAR source_path[MAX_PATH], const WCHAR *file_name, HMODULE h_module)
{
	DWORD dw_ret;
	BOOL b_ret;

	dw_ret = GetModuleFileNameW(h_module, source_path, MAX_PATH);
	assert(dw_ret != 0);

	b_ret = PathRemoveFileSpecW(source_path);
	assert(b_ret);

	b_ret = PathAppendW(source_path, file_name);
	assert(b_ret);
}

#define debug_file_name "C:\\gdimm_debug.txt"

void debug_output_process_name()
{
	HANDLE h_proc = GetCurrentProcess();
	DWORD name_size = MAX_PATH;
	WCHAR name_str[MAX_PATH];
	QueryFullProcessImageNameW(h_proc, 0, name_str, &name_size);

	debug_output(name_str, name_size);
}

void debug_output(const WCHAR *str)
{
	FILE *f;
	fopen_s(&f, debug_file_name, "a+");
	fwprintf(f, L"%s\n", str);
	fclose(f);
}

void debug_output(const WCHAR *str, unsigned int c)
{
	FILE *f;
	fopen_s(&f, debug_file_name, "a+");
	fwrite(str, sizeof(WCHAR), c, f);
	fwprintf(f, L"\n");
	fclose(f);
}

void debug_output(const void *ptr, unsigned int size)
{
	FILE *f;
	fopen_s(&f, debug_file_name, "ab+");
	fwrite(ptr, 1, size, f);
	fclose(f);
}

void debug_output(long num)
{
	FILE *f;
	fopen_s(&f, debug_file_name, "a+");
	fwprintf(f, L"%d\n", num);
	fclose(f);
}