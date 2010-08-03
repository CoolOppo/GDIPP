#include "stdafx.h"
#include "gdimm.h"
#include "freetype.h"
#include "lock.h"
#include <gdipp_common.h>

HMODULE h_self = NULL;

gdimm_font_link font_link_instance;
gdimm_font_store font_store_instance;
gdimm_gamma gamma_instance;
gdimm_hook hook_instance;
gdimm_obj_registry obj_reg_instance;
gdimm_setting_cache setting_cache_instance;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
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

		// return false if setting file does not exist
		gdipp_load_setting(setting_path);

		if (gdipp_is_process_excluded(NULL))
			return FALSE;

		gdimm_lock::initialize();
		initialize_freetype();

		return hook_instance.hook();

	case DLL_PROCESS_DETACH:
		hook_instance.unhook();
		destroy_freetype();
		gdimm_lock::release();

		break;
	}

	return TRUE;
}
