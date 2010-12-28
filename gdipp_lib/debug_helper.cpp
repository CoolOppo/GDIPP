#include "stdafx.h"
#include "debug_helper.h"

const wchar_t *debug_file_name = L"C:\\gdipp_debug.log";

void gdipp_debug_buffer(const void *ptr, unsigned int size)
{
	FILE *f;
	_wfopen_s(&f, debug_file_name, L"a+");

	if (f != NULL)
	{
		fwrite(ptr, 1, size, f);
		fclose(f);
	}
}

void gdipp_debug_decimal(double num, bool new_line)
{
	FILE *f;
	_wfopen_s(&f, debug_file_name, L"a+");

	if (f != NULL)
	{
		if (new_line)
			fwprintf(f, L"%f\n", num);
		else
			fwprintf(f, L"%f, ", num);
		fclose(f);
	}
}

void gdipp_debug_integer(size_t num, bool new_line)
{
	FILE *f;
	_wfopen_s(&f, debug_file_name, L"a+");

	if (f != NULL)
	{
		if (new_line)
			fwprintf(f, L"%u\n", num);
		else
			fwprintf(f, L"%u, ", num);
		fclose(f);
	}
}

void gdipp_debug_string(const wchar_t *str)
{
	FILE *f;
	_wfopen_s(&f, debug_file_name, L"a+");

	if (f != NULL)
	{
		fwprintf(f, L"%s\n", str);
		fclose(f);
	}
}