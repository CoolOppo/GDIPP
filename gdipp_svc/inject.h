#pragma once

class _svc_injector
{
	WCHAR _gdimm_path_64[MAX_PATH];
	WCHAR _gdimm_path_32[MAX_PATH];

public:
	_svc_injector();
	bool inject_proc(LONG proc_id);
	void init_inject();
};

typedef singleton<_svc_injector> svc_injector;