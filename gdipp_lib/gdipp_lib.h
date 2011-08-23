#pragma once

#include "gdipp_lib/font_render_config_cache.h"

namespace gdipp
{

#ifdef GDIPP_EXPORTS
#define GDIPP_API __declspec(dllexport)
#else
#define GDIPP_API __declspec(dllimport)
#endif // GDIPP_EXPORTS

GDIPP_API BOOL get_dir_file_path(HMODULE h_module, const wchar_t *file_name, wchar_t *out_path);

GDIPP_API void init_font_render_config_cache(const pugi::xpath_node_set &root);
GDIPP_API render_config get_font_render_config(bool bold, bool italic, LONG height, const wchar_t *font_name);

GDIPP_API void init_minidump();
GDIPP_API void register_minidump_module(HMODULE h_module);

}
