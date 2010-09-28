#include "stdafx.h"
#include "gdimm.h"
#include "freetype.h"
#include "lock.h"
#include <gdipp_common.h>

HMODULE h_self = NULL;
bool os_support_directwrite;

gdimm_font_link font_link_instance;
gdimm_font_store font_store_instance;
gdimm_gamma gamma_instance;
gdimm_hook hook_instance;
gdimm_setting_cache setting_cache_instance;

sqlite3 *glyph_cache_db = NULL;
sqlite3 *glyph_run_cache_db = NULL;

bool initialize_cache_db()
{
	int i_ret;

	if (glyph_cache_db == NULL && glyph_run_cache_db == NULL)
	{
		wchar_t base_db_path[MAX_PATH], curr_db_path[MAX_PATH];
		if (!gdipp_get_dir_file_path(h_self, L"cache", base_db_path))
			return false;

		i_ret = sqlite3_initialize();
		assert(i_ret == SQLITE_OK);

		wcscpy_s(curr_db_path, base_db_path);
		PathAppendW(curr_db_path, L"glyph.sqlite");
		i_ret = sqlite3_open16(curr_db_path, &glyph_cache_db);
		if (i_ret != SQLITE_OK)
			glyph_cache_db = NULL;
		
		wcscpy_s(curr_db_path, base_db_path);
		PathAppendW(curr_db_path, L"glyph_run.sqlite");
		i_ret = sqlite3_open16(curr_db_path, &glyph_run_cache_db);
		if (i_ret != SQLITE_OK)
			glyph_run_cache_db = NULL;

		if (glyph_cache_db != NULL || glyph_run_cache_db != NULL)
			return true;
		else
			sqlite3_shutdown();
	}

	return false;
}

bool destroy_cache_db()
{
	int i_ret;
	bool b_ret = true;

	if (glyph_cache_db != NULL)
	{
		i_ret = sqlite3_close(glyph_cache_db);
		b_ret &= (i_ret == SQLITE_OK);
	}

	if (glyph_run_cache_db != NULL)
	{
		i_ret = sqlite3_close(glyph_run_cache_db);
		b_ret &= (i_ret == SQLITE_OK);
	}

	i_ret = sqlite3_shutdown();
	return b_ret & (i_ret == SQLITE_OK);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	BOOL b_ret;

	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
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

			OSVERSIONINFO ver_info = {sizeof(OSVERSIONINFO)};
			b_ret = GetVersionEx(&ver_info);
			if (!b_ret)
				return FALSE;
			os_support_directwrite = (ver_info.dwMajorVersion >= 6);

			//initialize_cache_db();
			gdimm_lock::initialize();
			initialize_freetype();

			if (!hook_instance.hook())
				return FALSE;

			break;
		}
		case DLL_PROCESS_DETACH:
			hook_instance.unhook();
			destroy_freetype();
			gdimm_lock::finalize();
			//destroy_cache_db();

			break;
	}

	return TRUE;
}
