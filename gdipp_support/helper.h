#pragma once

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

struct wstring_ci_less
{
	bool operator()(const std::wstring &string1, const std::wstring &string2) const;
};

char get_gdi_weight_class(unsigned short weight);

// generate hash of traits for the specified font configuration
// returned integer is used as key of configuration map
uint32_t get_render_config_trait(char weight_class, bool italic, LONG height, const wchar_t *font_name);

}
