#include "stdafx.h"
#include "gdipp_client/global.h"
#include "gdipp_config/exclude_config.h"
#include "gdipp_lib/helper.h"

namespace gdipp
{
	
exclude_config exclude_config_instance;

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

			wchar_t this_proc_name[MAX_PATH];
			DWORD dw_ret = GetModuleBaseNameW(GetCurrentProcess(), NULL, this_proc_name, MAX_PATH);
			assert(dw_ret != 0);

			gdipp::exclude_config_instance.load(gdipp::config_file_instance);

			if (gdipp::exclude_config_instance.is_process_excluded(this_proc_name))
				return FALSE;

			OSVERSIONINFO ver_info = {sizeof(OSVERSIONINFO)};
			b_ret = GetVersionEx(&ver_info);
			if (!b_ret)
				return FALSE;
			gdipp::os_support_directwrite = (ver_info.dwMajorVersion >= 6);

			gdipp::client_config_instance.load(gdipp::config_file_instance);

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
