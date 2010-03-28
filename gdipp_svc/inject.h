#pragma once

#include <global.h>

class svc_injector
{
	WCHAR _gdimm_path_64[MAX_PATH];
	WCHAR _gdimm_path_32[MAX_PATH];
	inject_payload _payload;

public:
	svc_injector();
	void init_payload(const WCHAR *svc_event_name);
	NTSTATUS inject_proc(LONG proc_id);
	void initial_inject();
};