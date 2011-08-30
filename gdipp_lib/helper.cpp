#include "stdafx.h"
#include "helper.h"
#include "gdipp_lib/minidump.h"

namespace gdipp
{

bool wstring_ci_less::operator()(const std::wstring &string1, const std::wstring &string2) const
{
	return (_wcsicmp(string1.c_str(), string2.c_str()) < 0);
}

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

void init_minidump()
{
	SetUnhandledExceptionFilter(minidump_filter);
}

void register_minidump_module(HMODULE h_module)
{
	h_minidump_modules.push_back(h_module);
}

char get_gdi_weight_class(unsigned short weight)
{
	/*
	emulate GDI behavior:
	weight 1 to 550 are rendered as Regular
	551 to 611 are Semibold
	612 to infinity are Bold

	weight 0 is DONTCARE
	*/

	const long weight_class_max[] = {0, 550, 611};
	const char max_weight_class = sizeof(weight_class_max) / sizeof(long);

	for (char i = 0; i < max_weight_class; ++i)
	{
		if (weight <= weight_class_max[i])
			return i;
	}

	return max_weight_class;
}

unsigned long get_render_config_trait(char weight_class, bool italic, LONG height, const wchar_t *font_name)
{
	const size_t font_name_len = wcslen(font_name) + 1;
	const size_t trait_size = sizeof(weight_class) + sizeof(italic) + sizeof(height) + font_name_len * sizeof(wchar_t);
	BYTE *trait_data = new BYTE[trait_size];

	*reinterpret_cast<char *>(trait_data) = weight_class;
	*reinterpret_cast<bool *>(trait_data + sizeof(char)) = italic;
	*reinterpret_cast<LONG *>(trait_data + sizeof(char) + sizeof(bool)) = height;
	wcscpy_s(reinterpret_cast<wchar_t *>(trait_data + sizeof(char) + sizeof(bool) + sizeof(LONG)), font_name_len, font_name);

	unsigned long trait_id;
	MurmurHash3_x86_32(trait_data, trait_size, 0, &trait_id);
	delete[] trait_data;

	return trait_id;
}

}
