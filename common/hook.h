#pragma once

#include "global.h"
#include <vector>
using namespace std;

class _gdimm_hook
{
	vector<TRACED_HOOK_HANDLE> hook_handles;
	DWORD *threads;
	DWORD thread_count;

	BOOL enum_threads(DWORD *thread_ids, DWORD *count, DWORD exclude = 0) const;
	void install_hook(TCHAR *lib_name, LPCSTR proc_name, void *hook_proc);

public:
	void hook();
	void unhook() const;
};

typedef singleton<_gdimm_hook> gdimm_hook;