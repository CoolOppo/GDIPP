#include "stdafx.h"
#include "gdipp_common.h"
#include "setting.h"

vector<HMODULE> h_modules;

HMODULE h_self = NULL;
gdipp_setting setting_instance;

GDIPP_API void gdipp_register_module(HMODULE h_module)
{
	h_modules.push_back(h_module);
}

GDIPP_API BOOL gdipp_get_dir_file_path(HMODULE h_module, const wchar_t *file_name, wchar_t *out_path)
{
	// append the file name after the module's resident directory name
	// if the module handle is NULL, use the current exe as the module

	DWORD dw_ret;
	BOOL b_ret;

	dw_ret = GetModuleFileNameW(h_module, out_path, MAX_PATH);
	if (dw_ret == 0)
		return FALSE;

	b_ret = PathRemoveFileSpecW(out_path);
	if (!b_ret)
		return FALSE;

	return PathAppendW(out_path, file_name);
}

GDIPP_API void gdipp_init_setting()
{
	return setting_instance.init_setting();
}

GDIPP_API void gdipp_uninit_setting()
{
	setting_instance.uninit_setting();
}

GDIPP_API BOOL gdipp_load_setting(const wchar_t *setting_path)
{
	return setting_instance.load_setting(setting_path);
}

GDIPP_API BOOL gdipp_save_setting(const wchar_t *setting_path)
{
	return setting_instance.save_setting(setting_path);
}

GDIPP_API void *gdipp_insert_setting(const wchar_t *node_name, const wchar_t *node_value, const wchar_t *parent_xpath, const wchar_t *ref_node_xpath)
{
	return setting_instance.insert_setting(node_name, node_value, parent_xpath, ref_node_xpath);
}

GDIPP_API BOOL gdipp_set_setting_attr(const void *node_ptr, const wchar_t *attr_name, const wchar_t *attr_value)
{
	return setting_instance.set_setting_attr(node_ptr, attr_name, attr_value);
}

GDIPP_API BOOL gdipp_remove_setting_item(const wchar_t *node_xpath)
{
	return setting_instance.remove_setting_item(node_xpath);
}

GDIPP_API const wchar_t *gdipp_get_gdimm_setting(const wchar_t *setting_name, const wchar_t *font_name)
{
	return setting_instance.get_gdimm_setting(setting_name, font_name);
}

GDIPP_API const wchar_t *gdipp_get_demo_setting(const wchar_t *setting_name)
{
	return setting_instance.get_demo_setting(setting_name);
}

GDIPP_API const vector<const wstring> &gdipp_get_demo_fonts()
{
	return setting_instance.get_demo_fonts();
}

GDIPP_API const wchar_t *gdipp_get_service_setting(const wchar_t *setting_name)
{
	return setting_instance.get_service_setting(setting_name);
}

GDIPP_API bool gdipp_is_process_excluded(const wchar_t *proc_name)
{
	return setting_instance.is_process_excluded(proc_name);
}

#define debug_file_name "C:\\gdipp_debug.log"

GDIPP_API void gdipp_debug_output(const wchar_t *str)
{
	FILE *f;
	fopen_s(&f, debug_file_name, "a+");
	fwprintf(f, L"%s\n", str);
	fclose(f);
}

GDIPP_API void gdipp_debug_output(const wchar_t *str, unsigned int c)
{
	FILE *f;
	fopen_s(&f, debug_file_name, "a+");
	fwrite(str, sizeof(wchar_t), c, f);
	fwprintf(f, L"\n");
	fclose(f);
}

GDIPP_API void gdipp_debug_output(const void *ptr, unsigned int size)
{
	FILE *f;
	fopen_s(&f, debug_file_name, "ab+");
	fwrite(ptr, 1, size, f);
	fclose(f);
}

GDIPP_API void gdipp_debug_output(long num)
{
	FILE *f;
	fopen_s(&f, debug_file_name, "a+");
	fwprintf(f, L"%d\n", num);
	fclose(f);
}

GDIPP_API void gdipp_debug_output(DWORD num)
{
	FILE *f;
	fopen_s(&f, debug_file_name, "a+");
	fwprintf(f, L"%u\n", num);
	fclose(f);
}

LONG CALLBACK create_minidump(__in struct _EXCEPTION_POINTERS *ExceptionInfo)
{
	BOOL b_ret;

	bool ex_in_module = false;
	for (vector<HMODULE>::const_iterator iter = h_modules.begin(); iter != h_modules.end(); iter++)
	{
		MODULEINFO mod_info;
		b_ret = GetModuleInformation(GetCurrentProcess(), *iter, &mod_info, sizeof(MODULEINFO));
		if (!b_ret)
			return EXCEPTION_CONTINUE_SEARCH;

		// exception not from this module
		if (ExceptionInfo->ExceptionRecord->ExceptionAddress >= mod_info.lpBaseOfDll &&
			(DWORD) ExceptionInfo->ExceptionRecord->ExceptionAddress <= (DWORD) mod_info.lpBaseOfDll + mod_info.SizeOfImage)
		{
			ex_in_module = true;
			break;
		}
	}

	if (!ex_in_module)
		return EXCEPTION_CONTINUE_SEARCH;

	// path of the crash dump is
	// [gdipp_directory]\[dmp_dir_name]\[crash_process_name]\[crash_time].dmp

	const time_t curr_time = time(NULL);
	struct tm curr_tm;
	errno_t er_ret = localtime_s(&curr_tm, &curr_time);
	if (er_ret != 0)
		return EXCEPTION_CONTINUE_SEARCH;

	const wchar_t *dmp_dir_name = L"crash_dump\\";
	wchar_t dmp_file_path[MAX_PATH];
	b_ret = gdipp_get_dir_file_path(h_self, dmp_dir_name, dmp_file_path);
	assert(b_ret);
	const size_t dmp_dir_len = wcslen(dmp_file_path);
	assert(dmp_dir_len < MAX_PATH);

	b_ret = CreateDirectoryW(dmp_file_path, NULL);
	assert(b_ret || GetLastError() == ERROR_ALREADY_EXISTS);

	const DWORD exe_name_len = GetModuleBaseNameW(GetCurrentProcess(), NULL, dmp_file_path + dmp_dir_len, MAX_PATH);
	if (exe_name_len == 0)
		return EXCEPTION_CONTINUE_SEARCH;

	b_ret = CreateDirectoryW(dmp_file_path, NULL);
	assert(b_ret || GetLastError() == ERROR_ALREADY_EXISTS);

	wcsftime(dmp_file_path + dmp_dir_len + exe_name_len, MAX_PATH - dmp_dir_len - exe_name_len, L"\\%Y-%m-%d_%H-%M-%S.dmp", &curr_tm);

	// exception information is necessary for stack trace in the minidump
	MINIDUMP_EXCEPTION_INFORMATION ex_info = {GetCurrentThreadId(), ExceptionInfo, FALSE};

	const HANDLE dmp_file = CreateFileW(dmp_file_path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (dmp_file != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_TYPE dump_type = (MINIDUMP_TYPE)
			(
			MiniDumpWithDataSegs |
			MiniDumpWithHandleData |
			MiniDumpScanMemory |
			MiniDumpWithIndirectlyReferencedMemory |
			MiniDumpWithFullMemoryInfo |
			MiniDumpWithThreadInfo
			);
		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), dmp_file, dump_type, &ex_info, NULL, NULL);
		CloseHandle(dmp_file);
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

BOOL APIENTRY DllMain(
	HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved)
{
	static PVOID h_ex_handler = NULL;

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		h_self = hModule;
		gdipp_register_module(hModule);

		// add process-wide vectored exception handler to create minidump
		h_ex_handler = AddVectoredExceptionHandler(0, create_minidump);
		assert(h_ex_handler != NULL);

		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		ULONG ul_ret = RemoveVectoredExceptionHandler(h_ex_handler);
		assert(ul_ret != 0);

		break;
	}

	return TRUE;
}