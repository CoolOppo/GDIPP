#include "stdafx.h"
#include "gdipp_client.h"
#include "gdipp_lib/gdipp_lib.h"

namespace gdipp
{

HMODULE h_self = NULL;
bool os_support_directwrite;
RPC_BINDING_HANDLE h_gdipp_rpc;

gamma gamma_instance;
hook hook_instance;
gdimm_mem_man mem_man_instance;

bool init_rpc_client()
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

}

void __RPC_FAR *__RPC_USER MIDL_user_allocate(size_t size)
{
	return HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, size);
}

void __RPC_USER MIDL_user_free(void __RPC_FAR *ptr)
{
	HeapFree(GetProcessHeap(), 0, ptr);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	BOOL b_ret;

	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			gdipp::h_self = hModule;

			// get config file path
			wchar_t config_path[MAX_PATH];
			if (!gdipp::get_dir_file_path(hModule, L"client.conf", config_path))
				return FALSE;

			gdipp::init_config();

			// return false if config file does not exist
			if (!gdipp::load_config(config_path))
				return FALSE;

			if (gdipp::is_process_excluded(NULL))
				return FALSE;

			OSVERSIONINFO ver_info = {sizeof(OSVERSIONINFO)};
			b_ret = GetVersionEx(&ver_info);
			if (!b_ret)
				return FALSE;
			gdipp::os_support_directwrite = (ver_info.dwMajorVersion >= 6);

			gdipp::init_rpc_client();

			if (!gdipp::hook_instance.start())
				return FALSE;

			break;
		}
		case DLL_PROCESS_DETACH:
			gdipp::hook_instance.stop();
			break;
	}

	return TRUE;
}
