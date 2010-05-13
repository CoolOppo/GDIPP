#include "stdafx.h"
#include "injector.h"

gdipp_injector::gdipp_injector()
{
	BOOL b_ret;

	b_ret = gdipp_get_dir_file_path(NULL, L"gdimm_64.dll", _gdimm_path_64);
	assert(b_ret);

	b_ret = gdipp_get_dir_file_path(NULL, L"gdimm_32.dll", _gdimm_path_32);
	assert(b_ret);
}

void gdipp_injector::init_payload(GDIPP_INJECTOR_TYPE injector_type, const wchar_t *svc_event_name)
{
	_payload.inject_type = injector_type;

	if (svc_event_name != NULL)
		wcscpy_s(_payload.svc_event_name, svc_event_name);
}

NTSTATUS gdipp_injector::inject_process(ULONG process_id, ULONG thread_id)
{
#ifdef _M_X64
	return RhInjectLibrary(process_id, thread_id, EASYHOOK_INJECT_DEFAULT, NULL, _gdimm_path_64, (PVOID) &_payload, sizeof(gdipp_inject_payload));
#else
	return RhInjectLibrary(process_id, thread_id, EASYHOOK_INJECT_DEFAULT, _gdimm_path_32, NULL, (PVOID) &_payload, sizeof(gdipp_inject_payload));
#endif
}