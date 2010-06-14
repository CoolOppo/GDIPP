#pragma once

#include "text.h"

using namespace std;

class gdimm_hook
{
	vector<TRACED_HOOK_HANDLE> _hooks;

public:
	static DWORD text_tls_index;
	static gdimm_text **create_tls_text();
	static void delete_tls_text();

	gdimm_hook();
	~gdimm_hook();

	bool install_hook(LPCTSTR lib_name, LPCSTR proc_name, void *hook_proc);
	bool hook();
	void unhook();
};