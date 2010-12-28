#include "stdafx.h"
#include "setting_trait.h"

gdimm_setting_trait::gdimm_setting_trait(char weight_class, bool italic, LONG height, const wchar_t *font_name)
{
	const int font_name_len = static_cast<int>(wcslen(font_name) + 1);
	_setting_size = sizeof(weight_class) + sizeof(italic) + sizeof(height) + font_name_len * sizeof(wchar_t);
	_setting_data = new BYTE[_setting_size];

	*reinterpret_cast<char *>(_setting_data) = weight_class;
	*reinterpret_cast<bool *>(_setting_data + sizeof(char)) = italic;
	*reinterpret_cast<LONG *>(_setting_data + sizeof(char) + sizeof(bool)) = height;
	wcscpy_s(reinterpret_cast<wchar_t *>(_setting_data + sizeof(char) + sizeof(bool) + sizeof(LONG)), font_name_len, font_name);
}

gdimm_setting_trait::~gdimm_setting_trait()
{
	delete[] _setting_data;
}

char gdimm_setting_trait::get_weight_class() const
{
	return *reinterpret_cast<char *>(_setting_data);
}

bool gdimm_setting_trait::get_italic() const
{
	return *reinterpret_cast<bool *>(_setting_data + sizeof(char));
}

LONG gdimm_setting_trait::get_height() const
{
	return *reinterpret_cast<LONG *>(_setting_data + sizeof(char) + sizeof(bool));
}

const wchar_t *gdimm_setting_trait::get_font_name() const
{
	return reinterpret_cast<const wchar_t *>(_setting_data + sizeof(char) + sizeof(bool) + sizeof(LONG));
}

const BYTE *gdimm_setting_trait::get_data() const
{
	return _setting_data;
}

int gdimm_setting_trait::get_size() const
{
	return _setting_size;
}