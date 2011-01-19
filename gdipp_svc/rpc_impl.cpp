#include "stdafx.h"
#include "rpc_impl.h"
#include "ggo_renderer.h"
#include "helper.h"
#include <MurmurHash2.h>
#include <gdipp_lib.h>
#include <gdipp_rpc.h>

gdipp_font_man font_man_instance;
gdipp_pool<HDC> dc_pool_instance;
gdipp_glyph_cache glyph_cache_instance;

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

unsigned int generate_render_trait(const LOGFONTW *logfont, int render_mode)
{
	// the LOGFONTW structure and render mode are the minimal set that uniquely determine font metrics used by any renderer

	// exclude the bytes after the face name, which may contain junk data
	const int lf_metric_size = sizeof(LOGFONTW) - sizeof(logfont->lfFaceName);
	const int lf_facename_size = static_cast<const int>((wcslen(logfont->lfFaceName) * sizeof(wchar_t)));
	const int lf_total_size = lf_metric_size + lf_facename_size;
	
	return MurmurHash2(logfont, lf_total_size, render_mode);
}

GDIPP_RPC_SESSION_HANDLE gdipp_rpc_begin_session( 
	/* [in] */ handle_t h_gdipp_rpc,
	/* [size_is][in] */ const byte *logfont_buf,
	/* [in] */ unsigned long logfont_size,
	/* [in] */ int render_mode)
{
	// register font with given LOGFONT structure
	const LOGFONTW *logfont = reinterpret_cast<const LOGFONTW *>(logfont_buf);
	unsigned long font_id = font_man_instance.register_font(logfont, logfont_size);
	if (font_id == 0)
		return NULL;
	
	HDC hdc = reinterpret_cast<HDC>(dc_pool_instance.claim());
	if (hdc == NULL)
		return NULL;

	gdipp_rpc_session *new_session = reinterpret_cast<gdipp_rpc_session *>(MIDL_user_allocate(sizeof(gdipp_rpc_session)));

	new_session->font_id = font_id;
	new_session->hdc = hdc;

	const OUTLINETEXTMETRICW *outline_metrics = reinterpret_cast<const OUTLINETEXTMETRICW *>(font_man_instance.get_font_metrics(font_id));
	// generate setting trait and retrieve font specific setting
	const LONG point_size = (logfont->lfHeight > 0 ? logfont->lfHeight : -MulDiv(logfont->lfHeight, 72, outline_metrics->otmTextMetrics.tmDigitizedAspectY));
	const gdimm_setting_trait setting_trait(get_gdi_weight_class(static_cast<unsigned short>(outline_metrics->otmTextMetrics.tmWeight)),
		!!outline_metrics->otmTextMetrics.tmItalic,
		point_size,
		metric_face_name(outline_metrics));
	new_session->font_setting = gdipp_get_gdimm_setting_cache(&setting_trait);

	new_session->render_mode = static_cast<FT_Render_Mode>(render_mode);
	new_session->render_trait = generate_render_trait(logfont, render_mode);

	// create session renderer
	switch (new_session->font_setting->renderer)
	{
	case RENDERER_CLEARTYPE:
		break;
	default:
		new_session->renderer = new gdimm_ggo_renderer(new_session);
		break;
	}

	return new_session;
}

unsigned long gdipp_rpc_get_font_data_size( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_SESSION_HANDLE h_session,
    /* [in] */ unsigned long table,
    /* [in] */ unsigned long offset)
{
	const gdipp_rpc_session *curr_session = reinterpret_cast<const gdipp_rpc_session *>(h_session);

	return font_man_instance.get_font_raw_data(curr_session->font_id, table, offset, NULL, 0);
}

unsigned long gdipp_rpc_get_font_data( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_SESSION_HANDLE h_session,
    /* [in] */ unsigned long table,
    /* [in] */ unsigned long offset,
    /* [size_is][out] */ byte *data_buf,
    /* [in] */ unsigned long buf_size)
{
	const gdipp_rpc_session *curr_session = reinterpret_cast<const gdipp_rpc_session *>(h_session);

	return font_man_instance.get_font_raw_data(curr_session->font_id, table, offset, data_buf, buf_size);
}

unsigned long gdipp_rpc_get_font_metrics_size( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_SESSION_HANDLE h_session)
{
	const gdipp_rpc_session *curr_session = reinterpret_cast<const gdipp_rpc_session *>(h_session);

	return font_man_instance.get_font_metrics(curr_session->font_id)->size();
}

unsigned long gdipp_rpc_get_font_metrics_data( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_SESSION_HANDLE h_session,
    /* [size_is][out] */ byte *metrics_buf,
    /* [in] */ unsigned long buf_size)
{
	const gdipp_rpc_session *curr_session = reinterpret_cast<const gdipp_rpc_session *>(h_session);

	const vector<BYTE> *metric_buf = font_man_instance.get_font_metrics(curr_session->font_id);

	const DWORD copy_size = min(static_cast<DWORD>(metric_buf->size()), buf_size);
	CopyMemory(metrics_buf, &metric_buf[0], copy_size);

	return copy_size;
}

unsigned long gdipp_rpc_get_glyph_indices( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_SESSION_HANDLE h_session,
    /* [size_is][string][in] */ const wchar_t *str,
    /* [in] */ int count,
    /* [size_is][out] */ unsigned short *gi)
{
	const gdipp_rpc_session *curr_session = reinterpret_cast<const gdipp_rpc_session *>(h_session);

	return font_man_instance.get_glyph_indices(reinterpret_cast<unsigned long>(h_session), str, count, gi);
}

GDIPP_RPC_GLYPH_RUN_HANDLE gdipp_rpc_make_glyph_run( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_SESSION_HANDLE h_session,
    /* [string][in] */ wchar_t *str,
    /* [in] */ unsigned int count,
    /* [in] */ boolean is_glyph_index)
{
	const gdipp_rpc_session *curr_session = reinterpret_cast<const gdipp_rpc_session *>(h_session);

	glyph_run new_glyph_run;
	curr_session->renderer->fetch_glyph_run(!!is_glyph_index, str, count, new_glyph_run);

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
	const gdipp_rpc_session *curr_session = reinterpret_cast<const gdipp_rpc_session *>(*h_session);

	delete curr_session->renderer;

	dc_pool_instance.free(curr_session->hdc);
	
	MIDL_user_free(*h_session);
	*h_session = NULL;

	return true;
}

void __RPC_USER GDIPP_RPC_SESSION_HANDLE_rundown(GDIPP_RPC_SESSION_HANDLE h_session)
{
	boolean b_ret;

	if (h_session)
	{
		b_ret = gdipp_rpc_end_session(NULL, &h_session);
		assert(b_ret);
	}
}

void __RPC_USER GDIPP_RPC_GLYPH_RUN_HANDLE_rundown(GDIPP_RPC_GLYPH_RUN_HANDLE h_glyph_run)
{
}