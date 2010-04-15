#pragma once

#include <gdipp_common.h>

class svc_injector
{
	wchar_t _gdimm_path_64[MAX_PATH];
	wchar_t _gdimm_path_32[MAX_PATH];
	gdipp_inject_payload _payload;

public:
	svc_injector();
	void init_payload(const wchar_t *svc_event_name);
	NTSTATUS inject_proc(LONG proc_id);
	void initial_inject();
};