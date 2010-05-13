#pragma once

#include "gdipp_common.h"

class gdipp_injector
{
	wchar_t _gdimm_path_64[MAX_PATH];
	wchar_t _gdimm_path_32[MAX_PATH];
	gdipp_inject_payload _payload;

public:
	gdipp_injector();
	void init_payload(GDIPP_INJECTOR_TYPE injector_type, const wchar_t *svc_event_name);
	NTSTATUS inject_process(ULONG process_id, ULONG thread_id);
};