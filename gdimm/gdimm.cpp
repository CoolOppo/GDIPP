// gdimm.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "gdimm.h"
#include "ft.h"

HMODULE h_self = NULL;
PVOID h_ex_handler = NULL;

gdimm_font_link font_link_instance;
gdimm_font_man font_man_instance;
gdimm_gamma gamma_instance;
gdimm_hook hook_instance;
gdimm_setting_cache setting_cache_instance;
gdipp_setting setting_instance;

CRITICAL_SECTION critical_section::_cs[_CS_TYPE_COUNT_];

LONG WINAPI create_minidump(__in struct _EXCEPTION_POINTERS *ExceptionInfo)
{
	BOOL b_ret;

	MODULEINFO mod_info;
	b_ret = GetModuleInformation(GetCurrentProcess(), h_self, &mod_info, sizeof(MODULEINFO));
	if (!b_ret)
		return EXCEPTION_CONTINUE_SEARCH;

	// exception not from this module
	if (ExceptionInfo->ExceptionRecord->ExceptionAddress < mod_info.lpBaseOfDll ||
		(DWORD) ExceptionInfo->ExceptionRecord->ExceptionAddress > (DWORD) mod_info.lpBaseOfDll + mod_info.SizeOfImage)
		return EXCEPTION_CONTINUE_SEARCH;

	// use process name and local time as the minidump file name

	const time_t curr_time = time(NULL);
	struct tm curr_tm;
	errno_t er_ret = localtime_s(&curr_tm, &curr_time);
	if (er_ret != 0)
		return EXCEPTION_CONTINUE_SEARCH;

	WCHAR dmp_name[MAX_PATH];
	DWORD exe_name_len = GetModuleBaseNameW(GetCurrentProcess(), NULL, dmp_name, MAX_PATH);
	if (exe_name_len == 0)
		return EXCEPTION_CONTINUE_SEARCH;

	wcsftime(dmp_name + exe_name_len, MAX_PATH - exe_name_len, L" %Y-%m-%d_%H-%M-%S.dmp", &curr_tm);
	WCHAR dmp_path[MAX_PATH];
	get_dir_file_path(h_self, dmp_name, dmp_path);

	// exception information is necessary for stack trace in the minidump
	MINIDUMP_EXCEPTION_INFORMATION ex_info = {GetCurrentThreadId(), ExceptionInfo, FALSE};

	const HANDLE dmp_file = CreateFileW(dmp_path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
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
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		h_ex_handler = AddVectoredExceptionHandler(0, create_minidump);
		assert(h_ex_handler != NULL);

		setting_instance.init(hModule);
		if (setting_instance.is_process_excluded(NULL))
			return FALSE;

		h_self = hModule;

		critical_section::initialize();
		initialize_freetype();

		font_man_instance.tls_index = TlsAlloc();
		assert(font_man_instance.tls_index != TLS_OUT_OF_INDEXES);
		font_man_instance.create_linked_font_holder();

		return hook_instance.hook();
	case DLL_THREAD_ATTACH:
		font_man_instance.create_linked_font_holder();
		break;
	case DLL_THREAD_DETACH:
		font_man_instance.delete_linked_font_holder();
		break;
	case DLL_PROCESS_DETACH:
		hook_instance.unhook();

		font_man_instance.delete_linked_font_holder();
		TlsFree(font_man_instance.tls_index);

		destroy_freetype();
		critical_section::release();

		ULONG ul_ret = RemoveVectoredExceptionHandler(h_ex_handler);
		assert(ul_ret != 0);

		break;
	}

	return TRUE;
}
