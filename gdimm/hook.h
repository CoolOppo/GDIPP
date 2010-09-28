#pragma once

using namespace std;

struct char_str_ci_less
{
	bool operator()(const char *string1, const char *string2) const;
};

struct wchar_str_ci_less
{
	bool operator()(const wchar_t *string1, const wchar_t *string2) const;
};

class gdimm_hook
{
	list<TRACED_HOOK_HANDLE> _hooks;
	
	typedef map<const char *, void *> hook_proc_map;
	typedef map<const char *, hook_proc_map *, char_str_ci_less> lib_hook_map_a;
	typedef map<const wchar_t *, hook_proc_map *, wchar_str_ci_less> lib_hook_map_w;
	
	// procedure name => hook procedure pointer
	list<hook_proc_map> _delayed_hook_registry;
	lib_hook_map_a _delayed_hooks_a;
	lib_hook_map_w _delayed_hooks_w;

	bool install_hook(HMODULE h_lib, LPCSTR proc_name, void *hook_proc);
	void register_delayed_hook(LPCSTR lib_name_a, LPCWSTR lib_name_w, LPCSTR proc_name, void *hook_proc);

public:
	bool install_hook(LPCSTR lib_name, LPCSTR proc_name, void *hook_proc);
	bool install_hook(LPCWSTR lib_name, LPCSTR proc_name, void *hook_proc);
	bool install_delayed_hook(LPCSTR lib_name, HMODULE h_lib);
	bool install_delayed_hook(LPCWSTR lib_name, HMODULE h_lib);
	bool hook();
	void unhook();
};