#include "stdafx.h"
#include <gdipp_common.h>

using namespace std;

enum ENUM_OP_MODE
{
	ENUM_OP_INJECT,
	ENUM_OP_EJECT
};

typedef bool (*ENUM_PROCESSES_CALLBACK)(const PROCESSENTRY32W &pe32, void *parameter);
typedef bool (*ENUM_MODULES_CALLBACK)(const MODULEENTRY32W &me32, void *parameter);

struct eject_module_addr
{
	HMODULE h_kernel32;
	HMODULE h_gdimm;
};

bool enum_processes(ENUM_PROCESSES_CALLBACK callback, void *parameter)
{
	bool b_ret = FALSE;

	const HANDLE h_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (h_snapshot == INVALID_HANDLE_VALUE)
		return false;

	PROCESSENTRY32W pe32 = {};
	pe32.dwSize = sizeof(PROCESSENTRY32W);

	if (Process32FirstW(h_snapshot, &pe32))
	{
		do
		{
			// function returns false only if all callback functions return false
			b_ret |= callback(pe32, parameter);
		} while (Process32NextW(h_snapshot, &pe32));
	}

	CloseHandle(h_snapshot);

	return b_ret;
}

bool enum_modules(DWORD process_id, ENUM_MODULES_CALLBACK callback, void *parameter)
{
	bool b_ret = false;

	// only enumerate modules in the same bitness as the gdipp Enumerator
	const HANDLE h_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, process_id);
	if (h_snapshot == INVALID_HANDLE_VALUE)
		return false;

	MODULEENTRY32W me32 = {};
	me32.dwSize = sizeof(MODULEENTRY32W);

	if (Module32FirstW(h_snapshot, &me32))
	{
		do
		{
			b_ret |= callback(me32, parameter);
		} while (Module32NextW(h_snapshot, &me32));
	}

	CloseHandle(h_snapshot);

	return b_ret;
}

bool inject_callback(const PROCESSENTRY32W &pe32, void *parameter)
{
	BOOL b_ret;
	NTSTATUS nt_ret;

	const DWORD *user_session_id = (DWORD *)parameter;

	// processes in session 0 are usually UI-less
	// exclude them from injecting
	DWORD proc_session_id;
	b_ret = ProcessIdToSessionId(pe32.th32ProcessID, &proc_session_id);

	if (!b_ret || proc_session_id != *user_session_id || gdipp_is_process_excluded(pe32.szExeFile))
		return false;

	nt_ret = gdipp_inject_process(pe32.th32ProcessID);
	if (nt_ret != 0)
		return false;

	return true;
}

bool get_module_addr(const MODULEENTRY32W &me32, void *parameter)
{
	eject_module_addr *module_addr = (eject_module_addr *) parameter;

	if (_wcsicmp(me32.szModule, L"kernel32.dll") == 0)
		module_addr->h_kernel32 = me32.hModule;

#ifdef _M_X64
	else if (_wcsicmp(me32.szModule, L"gdimm_64.dll") == 0)
#else
	else if (_wcsicmp(me32.szModule, L"gdimm_32.dll") == 0)
#endif _M_X64
		module_addr->h_gdimm = me32.hModule;

	else
		return false;

	return true;
}

bool eject_callback(const PROCESSENTRY32W &pe32, void *parameter)
{
	BOOL b_ret;

	/*
	create remote thread and execute remote FreeLibrary with the argument of gdimm module address
	to calculate the procedure address of FreeLibrary in the target process,
	it is assumed that the offset of FreeLibrary from the starting of kernel32 is the same in both gdipp Enumerator and the target process
	since 32-bit and 64-bit kernel32 are different modules, gdipp Enumerator only ejects gdimm from processes with the same bitness as itself
	*/

	const DWORD *user_session_id = (DWORD *)parameter;

	DWORD proc_session_id;
	b_ret = ProcessIdToSessionId(pe32.th32ProcessID, &proc_session_id);
	if (proc_session_id != *user_session_id)
		return false;
	
	eject_module_addr remote_mod_addr = {};
	b_ret = enum_modules(pe32.th32ProcessID, get_module_addr, &remote_mod_addr);
	if (!b_ret)
		return false;

	// the process does not have either kernel32 or gdimm
	if (remote_mod_addr.h_kernel32 == NULL || remote_mod_addr.h_gdimm == NULL)
		return false;

	// although kernel32 appears to be loaded at the same address for all processes, we do not assume it to be true
	// instead, calculate the offset of FreeLibrary and apply to the remote kernel32
	const HMODULE h_local_kernel32 = GetModuleHandleW(L"kernel32.dll");
	if (h_local_kernel32 == NULL)
		return false;

	const FARPROC h_local_free = GetProcAddress(h_local_kernel32, "FreeLibrary");
	if (h_local_free == NULL)
		return false;

	const void *h_remote_free = (char *)h_local_free - (char *)h_local_kernel32 + (char *)remote_mod_addr.h_kernel32;

	const HANDLE h_process = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, FALSE, pe32.th32ProcessID);
	if (h_process == NULL)
		return FALSE;

	const HANDLE h_thread = CreateRemoteThread(h_process,
		NULL,
		0,
		(LPTHREAD_START_ROUTINE) h_remote_free,
		remote_mod_addr.h_gdimm,
		0,
		NULL);
	if (h_thread != NULL)
	{
		WaitForSingleObject(h_thread, 5000);
		CloseHandle(h_thread);
	}

	CloseHandle(h_process);

	return (h_thread != NULL);
}

BOOL load_setting()
{
	BOOL b_ret;

	// get setting file path
	wchar_t setting_path[MAX_PATH];
	b_ret = gdipp_get_dir_file_path(NULL, L"gdipp_setting.xml", setting_path);
	if (!b_ret)
		return FALSE;

	gdipp_init_setting();
	return gdipp_load_setting(setting_path);
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	BOOL b_ret;

	int argc;
	LPWSTR *argv = CommandLineToArgvW(lpCmdLine, &argc);
	if (argv == NULL)
		return EXIT_FAILURE;

	const wchar_t *stop_param = L"--eject";
	ENUM_OP_MODE op_mode = ENUM_OP_INJECT;
	
	for (int i = 0; i < argc; i++)
	{
		if (_wcsicmp(argv[i], stop_param) == 0)
		{
			op_mode = ENUM_OP_EJECT;
			break;
		}
	}

	// all functions only affect processes in the same session as gdipp Enumerator 
	DWORD user_session_id = WTSGetActiveConsoleSessionId();
	if (user_session_id == 0xFFFFFFFF)
		return EXIT_FAILURE;

	switch (op_mode)
	{
	case ENUM_OP_EJECT:
		b_ret = enum_processes(eject_callback, &user_session_id);
		break;
	default:
		b_ret = load_setting();
		if (!b_ret)
			return EXIT_FAILURE;

		gdipp_init_payload(GDIPP_INJECTOR_SERVICE);

		b_ret = enum_processes(inject_callback, &user_session_id);
		break;
	}

	if (b_ret)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}