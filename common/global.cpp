#include "stdafx.h"
#include "global.h"

void get_dir_file_path(HMODULE h_module, const WCHAR *file_name, WCHAR source_path[MAX_PATH])
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

const WCHAR *metric_family_name(const BYTE *metric_buf)
{
	return (const WCHAR*)(metric_buf + (UINT)((OUTLINETEXTMETRICW*) metric_buf)->otmpFamilyName);
}

const WCHAR *metric_family_name(const OUTLINETEXTMETRICW *outline_metric)
{
	return (const WCHAR*)((BYTE*) outline_metric + (UINT) outline_metric->otmpFamilyName);
}

const WCHAR *metric_face_name(const BYTE *metric_buf)
{
	return (const WCHAR*)(metric_buf + (UINT)((OUTLINETEXTMETRICW*) metric_buf)->otmpFaceName);
}

const WCHAR *metric_face_name(const OUTLINETEXTMETRICW *outline_metric)
{
	return (const WCHAR*)((BYTE*) outline_metric + (UINT) outline_metric->otmpFaceName);
}

#define debug_file_name "C:\\gdipp_debug.log"

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

void debug_output(DWORD num)
{
	FILE *f;
	fopen_s(&f, debug_file_name, "a+");
	fwprintf(f, L"%u\n", num);
	fclose(f);
}