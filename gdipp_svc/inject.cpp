#include "stdafx.h"
#include "inject.h"
#include <easyhook.h>

_svc_injector::_svc_injector()
{
	get_dir_file_path(_gdimm_path_32, TEXT("gdimm_32.dll"));
	get_dir_file_path(_gdimm_path_64, TEXT("gdimm_64.dll"));
}

void _svc_injector::inject(LONG proc_id)
{
	BOOL b_ret;
	NTSTATUS eh_error;

	HANDLE h_process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, proc_id);
	assert(h_process != NULL);

	BOOL is_wow64;
	b_ret = IsWow64Process(h_process, &is_wow64);
	assert(b_ret);
	b_ret = CloseHandle(h_process);
	assert(b_ret);
	
	const INJECTOR_TYPE injector_type = GDIPP_SERVICE;
	if (is_wow64)
	{
		eh_error = RhInjectLibrary(proc_id, 0, EASYHOOK_INJECT_DEFAULT, NULL, _gdimm_path_64, (PVOID) &injector_type, sizeof(INJECTOR_TYPE));
		assert(eh_error == 0);
	}
	else
	{
		eh_error = RhInjectLibrary(proc_id, 0, EASYHOOK_INJECT_DEFAULT, _gdimm_path_32, NULL, (PVOID) &injector_type, sizeof(INJECTOR_TYPE));
		assert(eh_error == 0);
	}
}