#include "stdafx.h"
#include "gdipp_client/gamma.h"
#include "gdipp_client/global.h"
#include "gdipp_config/constant_client.h"
#include "gdipp_config/exclude_config.h"
#include "gdipp_lib/helper.h"
#include "gdipp_lib/lock.h"

namespace gdipp
{

HANDLE process_heap = GetProcessHeap();
	
bool init_rpc_client()
{
	if (process_heap == NULL)
		return false;

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
	return HeapAlloc(gdipp::process_heap, HEAP_GENERATE_EXCEPTIONS, size);
}

void __RPC_USER MIDL_user_free(void __RPC_FAR *ptr)
{
	HeapFree(gdipp::process_heap, 0, ptr);
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

			if (gdipp::exclude_config::is_process_excluded(gdipp::config_instance, this_proc_name))
				return FALSE;

			OSVERSIONINFO ver_info = {sizeof(OSVERSIONINFO)};
			b_ret = GetVersionEx(&ver_info);
			if (!b_ret)
				return FALSE;
			gdipp::os_support_directwrite = (ver_info.dwMajorVersion >= 6);

			gdipp::lock::initialize_locks();

			if (!gdipp::init_rpc_client())
				return FALSE;

			gdipp::client_config_instance.parse(gdipp::config_instance);

			if (!gdipp::hook_instance.start())
				return FALSE;

			break;
		}
		case DLL_PROCESS_DETACH:
			gdipp::hook_instance.stop();
			gdipp::lock::destory_locks();
			break;
	}

	return TRUE;
}
