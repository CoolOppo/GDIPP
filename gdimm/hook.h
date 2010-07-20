#pragma once

using namespace std;

class gdimm_hook
{
	list<TRACED_HOOK_HANDLE> _hooks;

public:
	bool install_hook(LPCTSTR lib_name, LPCSTR proc_name, void *hook_proc);
	bool hook();
	void unhook();
};