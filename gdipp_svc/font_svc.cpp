#include "stdafx.h"
#include "font_svc.h"
#include <gdipp_rpc.h>

gdipp_font_man font_man_instance;

void __RPC_FAR *__RPC_USER MIDL_user_allocate(size_t size)
{
	return HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, size);
}

void __RPC_USER MIDL_user_free(void __RPC_FAR *ptr)
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

GDIPP_RPC_SESSION_HANDLE gdipp_rpc_begin_session( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [size_is][in] */ const byte *logfont_buf,
    /* [in] */ unsigned long logfont_size)
{
	return reinterpret_cast<GDIPP_RPC_SESSION_HANDLE>(font_man_instance.register_font(reinterpret_cast<const LOGFONTW *>(logfont_buf), logfont_size));
}

unsigned long gdipp_rpc_get_font_data_size( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_SESSION_HANDLE h_session,
    /* [in] */ unsigned long table,
    /* [in] */ unsigned long offset)
{
	return font_man_instance.get_font_raw_data(reinterpret_cast<unsigned long>(h_session), table, offset, NULL, 0);
}

unsigned long gdipp_rpc_get_font_data( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_SESSION_HANDLE h_session,
    /* [in] */ unsigned long table,
    /* [in] */ unsigned long offset,
    /* [size_is][out] */ byte *data_buf,
    /* [in] */ unsigned long buf_size)
{
	return font_man_instance.get_font_raw_data(reinterpret_cast<unsigned long>(h_session), table, offset, data_buf, buf_size);
}

unsigned long gdipp_rpc_get_font_metrics_size( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_SESSION_HANDLE h_session)
{
	return font_man_instance.get_font_metrics(reinterpret_cast<unsigned long>(h_session), NULL, 0);
}

unsigned long gdipp_rpc_get_font_metrics_data( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_SESSION_HANDLE h_session,
    /* [size_is][out] */ byte *metrics_buf,
    /* [in] */ unsigned long buf_size)
{
	return font_man_instance.get_font_metrics(reinterpret_cast<unsigned long>(h_session), metrics_buf, buf_size);
}

unsigned long gdipp_rpc_get_glyph_indices( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_SESSION_HANDLE h_session,
    /* [size_is][string][in] */ const wchar_t *str,
    /* [in] */ int count,
    /* [size_is][out] */ unsigned short *gi)
{
	return font_man_instance.get_glyph_indices(reinterpret_cast<unsigned long>(h_session), str, count, gi);
}

GDIPP_RPC_GLYPH_RUN_HANDLE gdipp_rpc_make_glyph_run( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_SESSION_HANDLE h_session,
    /* [string][in] */ wchar_t *str,
    /* [in] */ unsigned int count,
    /* [in] */ boolean is_glyph_index)
{
	return NULL;
}

unsigned long gdipp_rpc_get_glyph_run_size( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_GLYPH_RUN_HANDLE h_glyph_run)
{
	return 0;
}

boolean gdipp_rpc_get_glyph_run( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_GLYPH_RUN_HANDLE h_glyph_run,
    /* [size_is][out] */ byte *glyph_run_buf,
    /* [in] */ unsigned long glyph_run_size)
{
	return false;
}

boolean gdipp_rpc_release_glyph_run( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [out][in] */ GDIPP_RPC_GLYPH_RUN_HANDLE *h_glyph_run)
{
	return false;
}

boolean gdipp_rpc_end_session( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [out][in] */ GDIPP_RPC_SESSION_HANDLE *h_session)
{
	return false;
}

void __RPC_USER GDIPP_RPC_SESSION_HANDLE_rundown( GDIPP_RPC_SESSION_HANDLE )
{
}

void __RPC_USER GDIPP_RPC_GLYPH_RUN_HANDLE_rundown( GDIPP_RPC_GLYPH_RUN_HANDLE )
{
}