#include "stdafx.h"
#include "global.h"
#include <shlwapi.h>

CRITICAL_SECTION critical_section::_cs;

void get_dir_file_path(TCHAR source_path[MAX_PATH], const TCHAR *file_name, HMODULE h_module)
{
	DWORD dw_ret;
	BOOL b_ret;

	dw_ret = GetModuleFileName(h_module, source_path, MAX_PATH);
	assert(dw_ret != 0);

	b_ret = PathRemoveFileSpec(source_path);
	assert(b_ret);

	b_ret = PathAppend(source_path, file_name);
	assert(b_ret);
}

#define debug_file_name "C:\\gdimm_debug.txt"

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