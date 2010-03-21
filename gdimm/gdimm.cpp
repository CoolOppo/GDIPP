// gdimm.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "gdimm.h"
#include "ft.h"

HMODULE h_self = NULL;

gdimm_font_link font_link_instance;
gdimm_font_man font_man_instance;
gdimm_gamma gamma_instance;
gdimm_hook hook_instance;
gdimm_setting_cache setting_cache_instance;
gdipp_setting setting_instance;

CRITICAL_SECTION critical_section::_cs[_CS_TYPE_COUNT_];

BOOL APIENTRY DllMain(
	HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		setting_instance.init(hModule);
		if (setting_instance.is_process_excluded(NULL))
			return FALSE;

		h_self = hModule;

		critical_section::initialize();
		initialize_freetype();

		return hook_instance.hook();
	case DLL_PROCESS_DETACH:
		hook_instance.unhook();
		destroy_freetype();
		critical_section::release();

		break;
	}

	return TRUE;
}
