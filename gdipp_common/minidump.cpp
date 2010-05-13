#include "stdafx.h"
#include "minidump.h"
#include "gdipp_common.h"

vector<HMODULE> h_minidump_modules;

BOOL WINAPI minidump_callback(IN PVOID CallbackParam,
	IN CONST PMINIDUMP_CALLBACK_INPUT CallbackInput,
	IN OUT PMINIDUMP_CALLBACK_OUTPUT CallbackOutput)
{
	switch (CallbackInput->CallbackType)
	{
	case IncludeModuleCallback:
		{
			for (vector<HMODULE>::const_iterator iter = h_minidump_modules.begin(); iter != h_minidump_modules.end(); iter++)
			{
				if (*iter == (HMODULE) CallbackInput->IncludeModule.BaseOfImage)
					return TRUE;
			}

			return FALSE;
		}
	case IncludeThreadCallback:
		{
			return (CallbackInput->IncludeThread.ThreadId == GetCurrentThreadId());
		}
	default:
		return TRUE;
	}
}

LONG CALLBACK create_minidump(__in struct _EXCEPTION_POINTERS *ExceptionInfo)
{
	if (h_minidump_modules.empty())
		return 0;

	BOOL b_ret;

	bool ex_in_module = false;
	for (vector<HMODULE>::const_iterator iter = h_minidump_modules.begin(); iter != h_minidump_modules.end(); iter++)
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
	b_ret = gdipp_get_dir_file_path(h_minidump_modules[0], dmp_dir_name, dmp_file_path);
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
		MINIDUMP_CALLBACK_INFORMATION ci;
		ci.CallbackRoutine = minidump_callback;
		ci.CallbackParam = NULL;

		MINIDUMP_TYPE dump_type = (MINIDUMP_TYPE)(MiniDumpWithPrivateReadWriteMemory |
			MiniDumpWithDataSegs |
			MiniDumpWithHandleData);
		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), dmp_file, dump_type, &ex_info, NULL, &ci);
		CloseHandle(dmp_file);
	}

	return EXCEPTION_CONTINUE_SEARCH;
}