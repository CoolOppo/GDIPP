#pragma once

#include <global.h>

class _gdimm_hook
{
	vector<TRACED_HOOK_HANDLE> _hooks;

public:
	void install_hook(LPCTSTR lib_name, LPCSTR proc_name, void *hook_proc);
	bool hook();
	void unhook();
};

typedef singleton<_gdimm_hook> gdimm_hook;

extern HMODULE h_self;