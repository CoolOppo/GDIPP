#include "stdafx.h"
#include "font_svc.h"
#include <gdipp_rpc.h>

gdipp_font_store font_store_instance;

void *__RPC_USER midl_user_allocate(size_t size)
{
	return HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, size);
}

void __RPC_USER midl_user_free(void *ptr)
{
	HeapFree(GetProcessHeap(), 0, ptr);
}

DWORD WINAPI start_gdipp_rpc_server(LPVOID lpParameter)
{
	RPC_STATUS rpc_status;
	
	rpc_status = RpcServerUseProtseqEpW(reinterpret_cast<RPC_WSTR>(L"ncalrpc"), RPC_C_PROTSEQ_MAX_REQS_DEFAULT, reinterpret_cast<RPC_WSTR>(L"gdipp"), NULL);
	if (rpc_status != RPC_S_OK)
		return 1;

	rpc_status = RpcServerRegisterIf(gdipp_rpc_v0_9_s_ifspec, NULL, NULL);
	if (rpc_status != RPC_S_OK)
		return 1;

	rpc_status = RpcServerListen(2, RPC_C_LISTEN_MAX_CALLS_DEFAULT, TRUE);
	if (rpc_status != RPC_S_OK)
		return 1;

	rpc_status = RpcMgmtWaitServerListen();
	if (rpc_status != RPC_S_OK)
		return 1;

	return 0;
}

bool stop_gdipp_rpc_server()
{
	RPC_STATUS rpc_status;

	rpc_status = RpcMgmtStopServerListening(NULL);

	return (rpc_status == RPC_S_OK);
}

/* [fault_status][comm_status] */ error_status_t gdipp_rpc_register_font( 
	/* [in] */ handle_t h_gdipp_rpc,
	/* [size_is][in] */ const byte *attr_buf,
	/* [in] */ unsigned long buf_size,
	/* [out] */ unsigned long *font_id)
{
	*font_id = font_store_instance.register_font(reinterpret_cast<const LOGFONTW *>(attr_buf), buf_size);
	return 0;
}

/* [fault_status][comm_status] */ error_status_t gdipp_rpc_get_font_data_size( 
	/* [in] */ handle_t h_gdipp_rpc,
	/* [in] */ unsigned long font_id,
	/* [in] */ unsigned long table,
	/* [in] */ unsigned long offset,
	/* [out] */ unsigned long *data_size)
{
	*data_size = font_store_instance.get_font_raw_data(font_id, table, offset, NULL, 0);
	return 0;
}

/* [fault_status][comm_status] */ error_status_t gdipp_rpc_get_font_data( 
	/* [in] */ handle_t h_gdipp_rpc,
	/* [in] */ unsigned long font_id,
	/* [in] */ unsigned long table,
	/* [in] */ unsigned long offset,
	/* [size_is][out] */ byte *data_buf,
	/* [in] */ unsigned long buf_size,
	/* [out] */ unsigned long *data_size)
{
	*data_size = font_store_instance.get_font_raw_data(font_id, table, offset, data_buf, buf_size);
	return 0;
}

/* [fault_status][comm_status] */ error_status_t gdipp_rpc_get_font_metrics_size( 
	/* [in] */ handle_t h_gdipp_rpc,
	/* [in] */ unsigned long font_id,
	/* [out] */ unsigned long *metrics_size)
{
	*metrics_size = font_store_instance.get_font_metrics(font_id, NULL, 0);
	return 0;
}

/* [fault_status][comm_status] */ error_status_t gdipp_rpc_get_font_metrics_data( 
	/* [in] */ handle_t h_gdipp_rpc,
	/* [in] */ unsigned long font_id,
	/* [size_is][out] */ byte *metrics_buf,
	/* [in] */ unsigned long buf_size,
	/* [out] */ unsigned long *metrics_size)
{
	*metrics_size = font_store_instance.get_font_metrics(font_id, metrics_buf, buf_size);
	return 0;
}

/* [fault_status][comm_status] */ error_status_t gdipp_rpc_get_glyph_indices( 
	/* [in] */ handle_t h_gdipp_rpc,
	/* [in] */ unsigned long font_id,
	/* [size_is][string][in] */ const wchar_t *str,
	/* [in] */ int count,
	/* [size_is][out] */ unsigned short *gi,
	/* [out] */ unsigned long *converted)
{
	*converted = font_store_instance.get_glyph_indices(font_id, str, count, gi);
	return 0;
}