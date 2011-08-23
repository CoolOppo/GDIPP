#include "stdafx.h"
#include "gdipp_lib.h"
#include "gdipp_lib/minidump.h"

namespace gdipp
{

font_render_config_cache *cache = NULL;

BOOL get_dir_file_path(HMODULE h_module, const wchar_t *file_name, wchar_t *out_path)
{
	// append the file name after the module's resident directory name
	// if the module handle is NULL, use the current exe as the module

	DWORD dw_ret;
	BOOL b_ret;

	dw_ret = GetModuleFileNameW(h_module, out_path, MAX_PATH);
	if (dw_ret == 0)
		return FALSE;

	b_ret = PathRemoveFileSpecW(out_path);
	if (!b_ret)
		return FALSE;

	return PathAppendW(out_path, file_name);
}

void init_font_render_config_cache(const pugi::xpath_node_set &root)
{
	if (cache == NULL)
		cache = new font_render_config_cache(root);
}

render_config get_font_render_config(bool bold, bool italic, LONG height, const wchar_t *font_name)
{
	if (cache != NULL)
		return cache->get_render_config(bold, italic, height, font_name);

	return render_config();
}

void init_minidump()
{
	SetUnhandledExceptionFilter(minidump_filter);
}

void register_minidump_module(HMODULE h_module)
{
	h_minidump_modules.push_back(h_module);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		register_minidump_module(hModule);
		break;
	case DLL_PROCESS_DETACH:
		if (cache != NULL)
			delete cache;
		break;
	}

	return TRUE;
}

}
