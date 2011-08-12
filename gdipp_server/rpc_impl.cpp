#include "stdafx.h"
#include "rpc_impl.h"
#include <algorithm>
#include <vector>
#include <MurmurHash3.h>
#include "gdipp_lib/gdipp_lib.h"
#include "gdipp_rpc/gdipp_rpc.h"
#include "gdipp_server/ggo_renderer.h"
#include "gdipp_server/glyph_run.h"
#include "gdipp_server/helper.h"

namespace gdipp
{

dc_pool dc_pool_instance;
font_man font_man_instance;
glyph_cache glyph_cache_instance;

uint32_t generate_render_trait(const LOGFONTW *logfont, int render_mode)
{
	// the LOGFONTW structure and render mode are the minimal set that uniquely determine font metrics used by any renderer

	// exclude the bytes after the face name, which may contain junk data
	const int lf_metric_size = sizeof(LOGFONTW) - sizeof(logfont->lfFaceName);
	const int lf_facename_size = static_cast<const int>((wcslen(logfont->lfFaceName) * sizeof(wchar_t)));
	const int lf_total_size = lf_metric_size + lf_facename_size;

	uint32_t render_trait;
#ifdef _M_X64
	MurmurHash3_x64_32(logfont, lf_total_size, render_mode, &render_trait);
#else
	MurmurHash3_x86_32(logfont, lf_total_size, render_mode, &render_trait);
#endif
	return render_trait;
}

/*
sqlite3 *index_db_instance;

// rpc index functions

bool rpc_index_initialize()
{
	int i_ret;

	i_ret = sqlite3_initialize();
	if (i_ret != SQLITE_OK)
		return false;

	// open SQLite database in memory
	i_ret = sqlite3_open(":memory:", &index_db_instance);
	if (i_ret != SQLITE_OK)
		return false;

	// create index tables
	i_ret = sqlite3_exec(index_db_instance, "CREATE TABLE session_index ('session_address' INTEGER NOT NULL)", NULL, NULL, NULL);
	if (i_ret != SQLITE_OK)
		return false;

	i_ret = sqlite3_exec(index_db_instance, "CREATE TABLE glyph_run_index ('glyph_run_address' INTEGER NOT NULL)", NULL, NULL, NULL);
	if (i_ret != SQLITE_OK)
		return false;

	return true;
}

bool rpc_index_shutdown()
{
	return (sqlite3_shutdown() == SQLITE_OK);
}

const rpc_session *rpc_index_lookup_session(GDIPP_RPC_SESSION_HANDLE h_session)
{
	int i_ret;

	const int session_id = reinterpret_cast<int>(h_session);
	const rpc_session *curr_session = NULL;

	sqlite3_stmt *select_stmt;

	i_ret = sqlite3_prepare_v2(index_db_instance, "SELECT session_address FROM session_index WHERE ROWID = ?", -1, &select_stmt, NULL);
	assert(i_ret == SQLITE_OK);

	i_ret = sqlite3_bind_int(select_stmt, 0, session_id);
	assert(i_ret == SQLITE_OK);

	i_ret = sqlite3_step(select_stmt);
	if (i_ret == SQLITE_ROW)
	{
#ifdef _M_X64
		curr_session = reinterpret_cast<const rpc_session *>(sqlite3_column_int64(select_stmt, 0));
#else
		curr_session = reinterpret_cast<const rpc_session *>(sqlite3_column_int(select_stmt, 0));
#endif
	}

	i_ret = sqlite3_finalize(select_stmt);
	assert(i_ret == SQLITE_OK);

	return curr_session;
}

const glyph_run *rpc_index_lookup_glyph_run(GDIPP_RPC_GLYPH_RUN_HANDLE h_glyph_run)
{
	int i_ret;

	const int glyph_run_id = reinterpret_cast<int>(h_glyph_run);
	const glyph_run *curr_glyph_run = NULL;

	sqlite3_stmt *select_stmt;

	i_ret = sqlite3_prepare_v2(index_db_instance, "SELECT glyph_run_address FROM glyph_run_index WHERE ROWID = ?", -1, &select_stmt, NULL);
	assert(i_ret == SQLITE_OK);

	i_ret = sqlite3_bind_int(select_stmt, 0, glyph_run_id);
	assert(i_ret == SQLITE_OK);

	i_ret = sqlite3_step(select_stmt);
	if (i_ret == SQLITE_ROW)
	{
#ifdef _M_X64
		curr_glyph_run = reinterpret_cast<const glyph_run *>(sqlite3_column_int64(select_stmt, 0));
#else
		curr_glyph_run = reinterpret_cast<const glyph_run *>(sqlite3_column_int(select_stmt, 0));
#endif
	}

	i_ret = sqlite3_finalize(select_stmt);
	assert(i_ret == SQLITE_OK);

	return curr_glyph_run;
}
}*/

DWORD WINAPI start_gdipp_rpc_server(LPVOID lpParameter)
{
	//bool b_ret;
	RPC_STATUS rpc_status;

	//b_ret = rpc_index_initialize();
	//if (!b_ret)
	//	return 1;

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
	//bool b_ret;
	RPC_STATUS rpc_status;

	rpc_status = RpcMgmtStopServerListening(NULL);
	if (rpc_status != RPC_S_OK)
		return false;

	//b_ret = rpc_index_shutdown();
	//return b_ret;

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

GDIPP_RPC_SESSION_HANDLE gdipp_rpc_begin_session( 
	/* [in] */ handle_t h_gdipp_rpc,
	/* [size_is][in] */ const byte *logfont_buf,
	/* [in] */ unsigned long logfont_size,
	/* [in] */ int render_mode)
{
	// register font with given LOGFONT structure
	const LOGFONTW *logfont = reinterpret_cast<const LOGFONTW *>(logfont_buf);
	void *font_id = gdipp::font_man_instance.register_font(logfont, logfont_size);
	if (font_id == NULL)
		return NULL;

	HDC hdc = gdipp::dc_pool_instance.claim();
	assert(hdc != NULL);

	gdipp::rpc_session *new_session = reinterpret_cast<gdipp::rpc_session *>(MIDL_user_allocate(sizeof(gdipp::rpc_session)));

	new_session->font_id = font_id;
	new_session->hdc = hdc;

	const std::vector<BYTE> *metric_buf = gdipp::font_man_instance.get_font_metrics(font_id);
	const OUTLINETEXTMETRICW *outline_metrics = reinterpret_cast<const OUTLINETEXTMETRICW *>(&(*metric_buf)[0]);
	// generate config trait and retrieve font-specific config
	const LONG point_size = (logfont->lfHeight > 0 ? logfont->lfHeight : -MulDiv(logfont->lfHeight, 72, outline_metrics->otmTextMetrics.tmDigitizedAspectY));
	const gdipp::config_trait curr_config_trait(gdipp::get_gdi_weight_class(static_cast<unsigned short>(outline_metrics->otmTextMetrics.tmWeight)),
		!!outline_metrics->otmTextMetrics.tmItalic,
		point_size,
		metric_face_name(outline_metrics));
	new_session->font_config = gdipp::get_font_config_cache(&curr_config_trait);

	new_session->render_mode = static_cast<FT_Render_Mode>(render_mode);
	new_session->render_trait = gdipp::generate_render_trait(logfont, render_mode);

	// create session renderer
	/*switch (new_session->font_config->renderer)
	{
	case gdipp::RENDERER_CLEARTYPE:
		break;
	case gdipp::RENDERER_DIRECTWRITE:
		break;
	case gdipp::RENDERER_FREETYPE:
		break;
	case gdipp::RENDERER_GETGLYPHOUTLINE:
		new_session->renderer = new gdipp::ggo_renderer(new_session);
		break;
	case gdipp::RENDERER_WIC:
		break;
	default:
		break;
	}*/
	new_session->renderer = new gdipp::ggo_renderer(new_session);

	return new_session;
}

unsigned long gdipp_rpc_get_font_size( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_SESSION_HANDLE h_session,
    /* [in] */ unsigned long table,
    /* [in] */ unsigned long offset)
{
	const gdipp::rpc_session *curr_session = reinterpret_cast<const gdipp::rpc_session *>(h_session);

	return gdipp::font_man_instance.get_font_data(curr_session->font_id, table, offset, NULL, 0);
}

unsigned long gdipp_rpc_get_font_data( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_SESSION_HANDLE h_session,
    /* [in] */ unsigned long table,
    /* [in] */ unsigned long offset,
    /* [size_is][out] */ byte *data_buf,
    /* [in] */ unsigned long buf_size)
{
	const gdipp::rpc_session *curr_session = reinterpret_cast<const gdipp::rpc_session *>(h_session);

	return gdipp::font_man_instance.get_font_data(curr_session->font_id, table, offset, data_buf, buf_size);
}

unsigned long gdipp_rpc_get_font_metrics_size( 
	/* [in] */ handle_t h_gdipp_rpc,
	/* [context_handle_noserialize][in] */ GDIPP_RPC_SESSION_HANDLE h_session)
{
	const gdipp::rpc_session *curr_session = reinterpret_cast<const gdipp::rpc_session *>(h_session);

	const std::vector<BYTE> *metric_buf = gdipp::font_man_instance.get_font_metrics(curr_session->font_id);

	return static_cast<unsigned long>(metric_buf->size());
}

unsigned long gdipp_rpc_get_font_metrics_data( 
	/* [in] */ handle_t h_gdipp_rpc,
	/* [context_handle_noserialize][in] */ GDIPP_RPC_SESSION_HANDLE h_session,
	/* [size_is][out] */ byte *metrics_buf,
	/* [in] */ unsigned long buf_size)
{
	const gdipp::rpc_session *curr_session = reinterpret_cast<const gdipp::rpc_session *>(h_session);

	const std::vector<BYTE> *metric_buf = gdipp::font_man_instance.get_font_metrics(curr_session->font_id);

	const DWORD copy_size = min(static_cast<DWORD>(metric_buf->size()), buf_size);
	CopyMemory(metrics_buf, &(*metric_buf)[0], copy_size);

	return copy_size;
}

unsigned long gdipp_rpc_get_glyph_indices( 
	/* [in] */ handle_t h_gdipp_rpc,
	/* [context_handle_noserialize][in] */ GDIPP_RPC_SESSION_HANDLE h_session,
	/* [size_is][string][in] */ const wchar_t *str,
	/* [in] */ int count,
	/* [size_is][out] */ unsigned short *gi)
{
	const gdipp::rpc_session *curr_session = reinterpret_cast<const gdipp::rpc_session *>(h_session);

	return gdipp::font_man_instance.get_glyph_indices(curr_session->font_id, str, count, gi);
}

boolean gdipp_rpc_make_bitmap_glyph_run( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [context_handle_noserialize][in] */ GDIPP_RPC_SESSION_HANDLE h_session,
    /* [string][in] */ wchar_t *str,
    /* [in] */ unsigned int count,
    /* [in] */ boolean is_glyph_index,
    /* [out] */ gdipp_rpc_bitmap_glyph_run **glyph_run_ptr)
{
	const gdipp::rpc_session *curr_session = reinterpret_cast<const gdipp::rpc_session *>(h_session);

	bool b_ret;

	// generate unique identifier for the string
	gdipp::uint128_t string_id;
#ifdef _M_X64
	MurmurHash3_x64_128(str, count * sizeof(wchar_t), is_glyph_index, &string_id);
#else
	MurmurHash3_x86_128(str, count * sizeof(wchar_t), is_glyph_index, &string_id);
#endif // _M_X64

	// check if a glyph run cached for the same rendering environment and string
	const gdipp::glyph_run *glyph_run = gdipp::glyph_cache_instance.lookup_glyph_run(curr_session->render_trait, string_id);
	if (!glyph_run)
	{
		// no cached glyph run. render new glyph run
		gdipp::glyph_run *new_glyph_run = new gdipp::glyph_run(count);
		b_ret = curr_session->renderer->render(!!is_glyph_index, str, count, new_glyph_run);
		if (!b_ret)
			return false;

		// and cache it
		gdipp::glyph_cache_instance.store_glyph_run(curr_session->render_trait, string_id, new_glyph_run);
		glyph_run = new_glyph_run;
	}

	// convert internal glyph run to RPC exchangable format

	// allocate space for glyphs and boxes
	*glyph_run_ptr = reinterpret_cast<gdipp_rpc_bitmap_glyph_run *>(MIDL_user_allocate(sizeof(gdipp_rpc_bitmap_glyph_run)));
	gdipp_rpc_bitmap_glyph_run *rpc_glyph_run = *glyph_run_ptr;
	rpc_glyph_run->count = glyph_run->glyphs.size();
	rpc_glyph_run->glyphs = reinterpret_cast<gdipp_rpc_bitmap_glyph *>(MIDL_user_allocate(sizeof(gdipp_rpc_bitmap_glyph) * rpc_glyph_run->count));
	rpc_glyph_run->ctrl_boxes = reinterpret_cast<RECT *>(MIDL_user_allocate(sizeof(RECT) * rpc_glyph_run->count));
	rpc_glyph_run->black_boxes = reinterpret_cast<RECT *>(MIDL_user_allocate(sizeof(RECT) * rpc_glyph_run->count));

	for (unsigned int i = 0; i < rpc_glyph_run->count; ++i)
	{
		const FT_BitmapGlyph bmp_glyph = reinterpret_cast<const FT_BitmapGlyph>(glyph_run->glyphs[i]);
		rpc_glyph_run->glyphs[i].left = bmp_glyph->left;
		rpc_glyph_run->glyphs[i].top = bmp_glyph->top;
		rpc_glyph_run->glyphs[i].rows = bmp_glyph->bitmap.rows;
		rpc_glyph_run->glyphs[i].width = bmp_glyph->bitmap.width;
		rpc_glyph_run->glyphs[i].pitch = bmp_glyph->bitmap.pitch;
		rpc_glyph_run->glyphs[i].buffer = reinterpret_cast<byte *>(MIDL_user_allocate(bmp_glyph->bitmap.rows * abs(bmp_glyph->bitmap.pitch)));
		rpc_glyph_run->glyphs[i].pixel_mode = bmp_glyph->bitmap.pixel_mode;
	}

	std::copy(glyph_run->ctrl_boxes.begin(), glyph_run->ctrl_boxes.end(), rpc_glyph_run->ctrl_boxes);
	std::copy(glyph_run->black_boxes.begin(), glyph_run->black_boxes.end(), rpc_glyph_run->black_boxes);

	return true;
}

boolean gdipp_rpc_make_outline_glyph_run( 
	/* [in] */ handle_t h_gdipp_rpc,
	/* [context_handle_noserialize][in] */ GDIPP_RPC_SESSION_HANDLE h_session,
	/* [string][in] */ wchar_t *str,
	/* [in] */ unsigned int count,
	/* [in] */ boolean is_glyph_index,
	/* [out] */ gdipp_rpc_outline_glyph_run **glyph_run_ptr)
{
	return false;
}

boolean gdipp_rpc_end_session( 
    /* [in] */ handle_t h_gdipp_rpc,
    /* [out][in] */ GDIPP_RPC_SESSION_HANDLE *h_session)
{
	const gdipp::rpc_session *curr_session = reinterpret_cast<const gdipp::rpc_session *>(*h_session);

	delete curr_session->renderer;

	gdipp::dc_pool_instance.free(curr_session->hdc);

	MIDL_user_free(*h_session);
	*h_session = NULL;

	return true;
}

void __RPC_USER GDIPP_RPC_SESSION_HANDLE_rundown(GDIPP_RPC_SESSION_HANDLE h_session)
{
	boolean b_ret;

	b_ret = gdipp_rpc_end_session(NULL, &h_session);
	assert(b_ret);
}
