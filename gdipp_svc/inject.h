#pragma once

#include "global.h"

class _svc_injector
{
	TCHAR _gdimm_path_32[MAX_PATH];
	TCHAR _gdimm_path_64[MAX_PATH];

public:
	_svc_injector();
	void inject(LONG proc_id);
};

typedef singleton<_svc_injector> svc_injector;