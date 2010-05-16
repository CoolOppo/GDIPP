// gdimm.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "gdimm.h"
#include "ft.h"
#include "lock.h"
#include <gdipp_common.h>

HMODULE h_self = NULL;

gdimm_font_link font_link_instance;
gdimm_gamma gamma_instance;
gdimm_hook hook_instance;
gdimm_setting_cache setting_cache_instance;
gdimm_glyph_cache glyph_cache_instance;

BOOL APIENTRY DllMain(
	HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved)
{
	BOOL b_ret;

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		h_self = hModule;
		gdipp_register_minidump_module(hModule);

		// get setting file path
		wchar_t setting_path[MAX_PATH];
		b_ret = gdipp_get_dir_file_path(hModule, L"gdipp_setting.xml", setting_path);
		assert(b_ret);

		gdipp_init_setting();
		b_ret = gdipp_load_setting(setting_path);
		assert(b_ret);

		if (gdipp_is_process_excluded(NULL))
			return FALSE;

		gdimm_lock::initialize();
		initialize_freetype();

		gdimm_hook::tls_index = TlsAlloc();
		assert(gdimm_hook::tls_index != TLS_OUT_OF_INDEXES);

		return hook_instance.hook();
	case DLL_THREAD_DETACH:
		gdimm_hook::delete_tls_text();
		break;
	case DLL_PROCESS_DETACH:
		hook_instance.unhook();
		gdimm_hook::delete_tls_text();
		TlsFree(gdimm_hook::tls_index);

		destroy_freetype();
		gdimm_lock::release();

		break;
	}

	return TRUE;
}
