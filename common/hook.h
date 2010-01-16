#pragma once

#include <vector>
using namespace std;

class gdimm_hook
{
private:
	vector<TRACED_HOOK_HANDLE> hook_handles;
	DWORD *threads;
	DWORD thread_count;

	BOOL enum_threads(DWORD *thread_ids, DWORD *count, DWORD exclude = 0) const;
	void install_hook(TCHAR *lib_name, LPCSTR proc_name, void *hook_proc);

public:
	void hook();
	void unhook() const;
};