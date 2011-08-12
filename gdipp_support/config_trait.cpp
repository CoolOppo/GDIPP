#include "config_trait.h"

namespace gdipp
{

config_trait::config_trait(char weight_class, bool italic, LONG height, const wchar_t *font_name)
{
	const int font_name_len = static_cast<int>(wcslen(font_name) + 1);
	_config_size = sizeof(weight_class) + sizeof(italic) + sizeof(height) + font_name_len * sizeof(wchar_t);
	_config_data = new BYTE[_config_size];

	*reinterpret_cast<char *>(_config_data) = weight_class;
	*reinterpret_cast<bool *>(_config_data + sizeof(char)) = italic;
	*reinterpret_cast<LONG *>(_config_data + sizeof(char) + sizeof(bool)) = height;
	wcscpy_s(reinterpret_cast<wchar_t *>(_config_data + sizeof(char) + sizeof(bool) + sizeof(LONG)), font_name_len, font_name);
}

config_trait::~config_trait()
{
	delete[] _config_data;
}

char config_trait::get_weight_class() const
{
	return *reinterpret_cast<char *>(_config_data);
}

bool config_trait::get_italic() const
{
	return *reinterpret_cast<bool *>(_config_data + sizeof(char));
}

LONG config_trait::get_height() const
{
	return *reinterpret_cast<LONG *>(_config_data + sizeof(char) + sizeof(bool));
}

const wchar_t *config_trait::get_font_name() const
{
	return reinterpret_cast<const wchar_t *>(_config_data + sizeof(char) + sizeof(bool) + sizeof(LONG));
}

const BYTE *config_trait::get_data() const
{
	return _config_data;
}

int config_trait::get_size() const
{
	return _config_size;
}

}
