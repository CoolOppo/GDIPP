#pragma once

#include <windows.h>

namespace gdipp
{

class config_trait
{
public:
	config_trait(char weight_class, bool italic, LONG height, const wchar_t *font_name);
	~config_trait();

	char get_weight_class() const;
	bool get_italic() const;
	LONG get_height() const;
	const wchar_t *get_font_name() const;
	const BYTE *get_data() const;
	int get_size() const;

private:
/*
	char weight_class;
	bool italic;
	LONG height;
	const wchar_t *font_name;
*/

	BYTE *_setting_data;
	int _setting_size;
};

}
