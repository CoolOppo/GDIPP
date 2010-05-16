#pragma once

using namespace std;

class gdimm_hook
{
	vector<TRACED_HOOK_HANDLE> _hooks;

public:
	static DWORD tls_index;
	static void *create_tls_text();
	static void delete_tls_text();

	bool install_hook(LPCTSTR lib_name, LPCSTR proc_name, void *hook_proc);
	bool hook();
	void unhook();
};