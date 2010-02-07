#pragma once

#include "global.h"
#include <easyhook.h>
#include <vector>
#include <map>
using namespace std;

struct hook_info
{
	LPCTSTR lib_name;
	LPCSTR proc_name;
	void *hook_proc;

	hook_info(LPCTSTR lib, LPCSTR proc, void *hook): lib_name(lib), proc_name(proc), hook_proc(hook) {}
};

class _gdimm_hook
{
	map<const hook_info, TRACED_HOOK_HANDLE> _hooks;

	bool enum_threads(DWORD exclude, vector<DWORD> &thread_ids) const;
	void install_hook(vector<DWORD> &thread_ids);

public:
	_gdimm_hook();
	void hook();
	void unhook();
};

typedef singleton<_gdimm_hook> gdimm_hook;