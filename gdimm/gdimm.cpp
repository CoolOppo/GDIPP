#include "stdafx.h"
#include "gdimm.h"
#include "lock.h"
#include "gdipp_lib/gdipp_lib.h"

HMODULE h_self = NULL;
bool os_support_directwrite;
RPC_BINDING_HANDLE h_gdipp_rpc;

gdimm_font_link font_link_instance;
gdimm_font_man font_man_instance;
gdimm_gamma gamma_instance;
gdimm_glyph_cache glyph_cache_instance;
gdimm_hook hook_instance;
gdimm_mem_man mem_man_instance;
gdimm_setting_cache setting_cache_instance;

void __RPC_FAR *__RPC_USER MIDL_user_allocate(size_t size)
{
	return HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, size);
}

void __RPC_USER MIDL_user_free(void __RPC_FAR *ptr)
{
	HeapFree(GetProcessHeap(), 0, ptr);
}

bool init_gdipp_rpc_client()
{
	RPC_WSTR binding_str;
	RPC_STATUS rpc_status;

	rpc_status = RpcStringBindingCompose(NULL, reinterpret_cast<RPC_WSTR>(L"ncalrpc"), NULL, reinterpret_cast<RPC_WSTR>(L"gdipp"), NULL, &binding_str);
	if (rpc_status != RPC_S_OK)
		return false;

	rpc_status = RpcBindingFromStringBinding(binding_str, &h_gdipp_rpc);
	if (rpc_status != RPC_S_OK)
		return false;

	rpc_status = RpcStringFree(&binding_str);
	if (rpc_status != RPC_S_OK)
		return false;

	return true;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	BOOL b_ret;

	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			h_self = hModule;

			// get setting file path
			wchar_t config_path[MAX_PATH];
			if (!get_dir_file_path(hModule, L"gdipp_config.xml", config_path))
				return FALSE;

			gdipp_init_setting();

			// return false if setting file does not exist
			if (!gdipp_load_setting(config_path))
				return FALSE;

			if (gdipp_is_process_excluded(NULL))
				return FALSE;

			OSVERSIONINFO ver_info = {sizeof(OSVERSIONINFO)};
			b_ret = GetVersionEx(&ver_info);
			if (!b_ret)
				return FALSE;
			os_support_directwrite = (ver_info.dwMajorVersion >= 6);

			gdimm_lock::initialize();
			init_gdipp_rpc_client();

			if (!hook_instance.hook())
				return FALSE;

			break;
		}
		case DLL_PROCESS_DETACH:
			hook_instance.unhook();
			gdimm_lock::finalize();

			break;
	}

	return TRUE;
}
