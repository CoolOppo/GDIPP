// gdimm.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "gdimm.h"
#include "ft.h"
#include "lock.h"
#include <gdipp_common.h>

HMODULE h_self = NULL;

gdimm_font_link font_link_instance;
gdimm_font_man font_man_instance;
gdimm_gamma gamma_instance;
gdimm_hook hook_instance;
gdimm_setting_cache setting_cache_instance;
gdimm_glyph_cache glyph_cache_instance;

const wchar_t *metric_family_name(const OUTLINETEXTMETRICW *outline_metric)
{
	return (const wchar_t*)((BYTE*) outline_metric + (UINT) outline_metric->otmpFamilyName);
}

const wchar_t *metric_face_name(const OUTLINETEXTMETRICW *outline_metric)
{
	return (const wchar_t*)((BYTE*) outline_metric + (UINT) outline_metric->otmpFaceName);
}

const wchar_t *metric_style_name(const OUTLINETEXTMETRICW *outline_metric)
{
	return (const wchar_t*)((BYTE*) outline_metric + (UINT) outline_metric->otmpStyleName);
}

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
		gdipp_register_module(hModule);

		// get setting file path
		wchar_t setting_path[MAX_PATH];
		b_ret = gdipp_get_dir_file_path(hModule, L"gdipp_setting.xml", setting_path);
		assert(b_ret);

		gdipp_init_setting();

		b_ret = gdipp_load_setting(setting_path);
		assert(b_ret);

		gdipp_uninit_setting();

		if (gdipp_is_process_excluded(NULL))
			return FALSE;

		gdimm_lock::initialize();
		initialize_freetype();

		font_man_instance.tls_index = TlsAlloc();
		assert(font_man_instance.tls_index != TLS_OUT_OF_INDEXES);
		font_man_instance.create_linked_font_holder();

		return hook_instance.hook();
	case DLL_THREAD_ATTACH:
		font_man_instance.create_linked_font_holder();

		break;
	case DLL_THREAD_DETACH:
		font_man_instance.delete_linked_font_holder();

		break;
	case DLL_PROCESS_DETACH:
		hook_instance.unhook();

		font_man_instance.delete_linked_font_holder();
		TlsFree(font_man_instance.tls_index);

		destroy_freetype();
		gdimm_lock::release();

		break;
	}

	return TRUE;
}
