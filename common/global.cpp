#include "stdafx.h"
#include "global.h"

#define debug_file_name "C:\\gdimm_debug.txt"

void debug_output(const TCHAR *str)
{
	FILE *f;
	fopen_s(&f, debug_file_name, "a+");
	fwprintf(f, TEXT("%s\n"), str);
	fclose(f);
}

void debug_output(const TCHAR *str, unsigned int c)
{
	FILE *f;
	fopen_s(&f, debug_file_name, "a+");
	fwrite(str, sizeof(TCHAR), c, f);
	fwprintf(f, TEXT("\n"));
	fclose(f);
}

void debug_output(const void *ptr, unsigned int size)
{
	FILE *f;
	fopen_s(&f, debug_file_name, "ab+");
	fwrite(ptr, 1, size, f);
	fclose(f);
}

void debug_output(DWORD num)
{
	FILE *f;
	fopen_s(&f, debug_file_name, "a+");
	fwprintf(f, TEXT("%u\n"), num);
	fclose(f);
}