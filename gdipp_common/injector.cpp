#include "stdafx.h"
#include "injector.h"

gdipp_injector::gdipp_injector()
{
	BOOL b_ret;

	const wchar_t *gdimm_name;

#ifdef _M_X64
	gdimm_name = L"gdimm_64.dll";
#else
	gdimm_name = L"gdimm_32.dll";
#endif

	b_ret = gdipp_get_dir_file_path(NULL, gdimm_name, _gdimm_path);
	assert(b_ret);
}

NTSTATUS gdipp_injector::inject_process(ULONG process_id, ULONG thread_id)
{
#ifdef _M_X64
	return RhInjectLibrary(process_id, thread_id, EASYHOOK_INJECT_DEFAULT, NULL, _gdimm_path, NULL, 0);
#else
	return RhInjectLibrary(process_id, thread_id, EASYHOOK_INJECT_DEFAULT, _gdimm_path, NULL, NULL, 0);
#endif
}