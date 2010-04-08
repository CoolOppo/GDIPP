// gdimm.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "gdimm.h"
#include "ft.h"
#include <gdipp_common.h>

HMODULE h_self = NULL;

gdimm_font_link font_link_instance;
gdimm_font_man font_man_instance;
gdimm_gamma gamma_instance;
gdimm_hook hook_instance;
gdimm_setting_cache setting_cache_instance;

CRITICAL_SECTION critical_section::_cs[_CS_TYPE_COUNT_];

const WCHAR *metric_family_name(const OUTLINETEXTMETRICW *outline_metric)
{
	return (const WCHAR*)((BYTE*) outline_metric + (UINT) outline_metric->otmpFamilyName);
}

const WCHAR *metric_face_name(const OUTLINETEXTMETRICW *outline_metric)
{
	return (const WCHAR*)((BYTE*) outline_metric + (UINT) outline_metric->otmpFaceName);
}

const WCHAR *metric_style_name(const OUTLINETEXTMETRICW *outline_metric)
{
	return (const WCHAR*)((BYTE*) outline_metric + (UINT) outline_metric->otmpStyleName);
}

BOOL APIENTRY DllMain(
	HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		h_self = hModule;
		register_module(hModule);

		if (is_process_excluded(NULL))
			return FALSE;

		critical_section::initialize();
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
		critical_section::release();

		break;
	}

	return TRUE;
}
