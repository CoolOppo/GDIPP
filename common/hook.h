#pragma once

#include "global.h"
#include <easyhook.h>
#include <vector>
using namespace std;

class _gdimm_hook
{
	vector<TRACED_HOOK_HANDLE> _hooks;

	void install_hook(LPCTSTR lib_name, LPCSTR proc_name, void *hook_proc);

public:
	bool hook();
	void unhook();
};

typedef singleton<_gdimm_hook> gdimm_hook;