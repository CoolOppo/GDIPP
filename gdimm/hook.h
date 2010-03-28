#pragma once

#include <global.h>
using namespace std;

class gdimm_hook
{
	vector<TRACED_HOOK_HANDLE> _hooks;

public:
	bool install_hook(LPCTSTR lib_name, LPCSTR proc_name, void *hook_proc);
	bool hook();
	void unhook();
};