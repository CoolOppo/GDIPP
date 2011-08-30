#pragma once

#include "gdipp_lib/api.h"

namespace gdipp
{

// convert a string to template value if possible
// helper function to convert raw config strings to values
template <typename T>
void wcs_convert(const wchar_t *str, T *converted)
{
	if (str != NULL)
		std::wistringstream(str) >> *converted;
}

struct GDIPP_API wstring_ci_less
{
	bool operator()(const std::wstring &string1, const std::wstring &string2) const;
};

GDIPP_API BOOL get_dir_file_path(HMODULE h_module, const wchar_t *file_name, wchar_t *out_path);

GDIPP_API void init_minidump();
GDIPP_API void register_minidump_module(HMODULE h_module);

GDIPP_API char get_gdi_weight_class(unsigned short weight);

// generate hash of traits for the specified font configuration
// returned integer is used as key of configuration map
GDIPP_API unsigned long get_render_config_trait(char weight_class, bool italic, LONG height, const wchar_t *font_name);

}
