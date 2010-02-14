#include "stdafx.h"
#include "inject.h"
#include <easyhook.h>

_svc_injector::_svc_injector()
{
#ifdef _M_X64
	get_dir_file_path(_gdimm_path, TEXT("gdimm_64.dll"));
#else
	get_dir_file_path(_gdimm_path, TEXT("gdimm_32.dll"));
#endif
}

void _svc_injector::inject(LONG proc_id)
{
	NTSTATUS eh_error;

#ifdef _M_X64
	eh_error = RhInjectLibrary(proc_id, 0, EASYHOOK_INJECT_DEFAULT, NULL, _gdimm_path, NULL, 0);
#else
	eh_error = RhInjectLibrary(proc_id, 0, EASYHOOK_INJECT_DEFAULT, _gdimm_path, NULL, NULL, 0);
#endif
	assert(eh_error == 0);
}