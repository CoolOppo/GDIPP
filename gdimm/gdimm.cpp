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
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		h_self = hModule;
		gdipp_register_minidump_module(hModule);

		// get setting file path
		wchar_t setting_path[MAX_PATH];
		if (!gdipp_get_dir_file_path(hModule, L"gdipp_setting.xml", setting_path))
			return FALSE;

		gdipp_init_setting();
		if (!gdipp_load_setting(setting_path))
			return FALSE;

		if (gdipp_is_process_excluded(NULL))
			return FALSE;

		gdimm_lock::initialize();
		initialize_freetype();

		return hook_instance.hook();

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		gdimm_hook::delete_tls_text();

		break;

	case DLL_PROCESS_DETACH:
		destroy_freetype();
		gdimm_hook::delete_tls_text();
		gdimm_hook::cleanup();
		hook_instance.unhook();
		gdimm_lock::release();

		break;
	}

	return TRUE;
}
