#include "stdafx.h"
#include "helper.h"

namespace gdipp
{

bool wstring_ci_less::operator()(const std::wstring &string1, const std::wstring &string2) const
{
	return (_wcsicmp(string1.c_str(), string2.c_str()) < 0);
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

uint32_t get_render_config_trait(char weight_class, bool italic, LONG height, const wchar_t *font_name)
{
	const size_t font_name_len = wcslen(font_name) + 1;
	const size_t trait_size = sizeof(weight_class) + sizeof(italic) + sizeof(height) + font_name_len * sizeof(wchar_t);
	BYTE *trait_data = new BYTE[trait_size];

	*reinterpret_cast<char *>(trait_data) = weight_class;
	*reinterpret_cast<bool *>(trait_data + sizeof(char)) = italic;
	*reinterpret_cast<LONG *>(trait_data + sizeof(char) + sizeof(bool)) = height;
	wcscpy_s(reinterpret_cast<wchar_t *>(trait_data + sizeof(char) + sizeof(bool) + sizeof(LONG)), font_name_len, font_name);

	uint32_t trait_id;
	MurmurHash3_x86_32(trait_data, trait_size, 0, &trait_id);
	delete[] trait_data;

	return trait_id;
}

}
