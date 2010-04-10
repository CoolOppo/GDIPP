#include "stdafx.h"
#include "gdipp_common.h"
#include "setting.h"

vector<HMODULE> h_modules;

HMODULE h_self = NULL;
gdipp_setting setting_instance;

GDIPP_API inline void register_module(HMODULE h_module)
{
	h_modules.push_back(h_module);
}

GDIPP_API void get_dir_file_path(HMODULE h_module, const WCHAR *file_name, WCHAR *source_path)
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

GDIPP_API inline const WCHAR *get_gdimm_setting(const char *setting_name, const WCHAR *font_name)
{
	return setting_instance.get_gdimm_setting(setting_name, font_name);
}

GDIPP_API inline const WCHAR *get_demo_setting(const char *setting_name)
{
	return setting_instance.get_demo_setting(setting_name);
}

GDIPP_API inline const vector<const wstring> &get_demo_font()
{
	return setting_instance.get_demo_font();
}

GDIPP_API inline const WCHAR *get_service_setting(const char *setting_name)
{
	return setting_instance.get_service_setting(setting_name);
}

GDIPP_API inline bool is_process_excluded(const WCHAR *proc_name)
{
	return setting_instance.is_process_excluded(proc_name);
}

#define debug_file_name "C:\\gdipp_debug.log"

GDIPP_API void debug_output(const WCHAR *str)
{
	FILE *f;
	fopen_s(&f, debug_file_name, "a+");
	fwprintf(f, L"%s\n", str);
	fclose(f);
}

GDIPP_API void debug_output(const WCHAR *str, unsigned int c)
{
	FILE *f;
	fopen_s(&f, debug_file_name, "a+");
	fwrite(str, sizeof(WCHAR), c, f);
	fwprintf(f, L"\n");
	fclose(f);
}

GDIPP_API void debug_output(const void *ptr, unsigned int size)
{
	FILE *f;
	fopen_s(&f, debug_file_name, "ab+");
	fwrite(ptr, 1, size, f);
	fclose(f);
}

GDIPP_API void debug_output(long num)
{
	FILE *f;
	fopen_s(&f, debug_file_name, "a+");
	fwprintf(f, L"%d\n", num);
	fclose(f);
}

GDIPP_API void debug_output(DWORD num)
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

	const WCHAR *dmp_dir_name = L"crash_dump\\";
	WCHAR dmp_file_path[MAX_PATH];
	get_dir_file_path(h_self, dmp_dir_name, dmp_file_path);
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
		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), dmp_file, MiniDumpWithIndirectlyReferencedMemory, &ex_info, NULL, NULL);
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
		register_module(hModule);

		// add process-wide vectored exception handler to create minidump
		h_ex_handler = AddVectoredExceptionHandler(0, create_minidump);
		assert(h_ex_handler != NULL);

		setting_instance.init(hModule);

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