#include "stdafx.h"
#include "font_chg.h"
#include <algorithm>

#define MAX_KEY_NAME 255
#define ERROR_MSG_LEN 1024

_gdimm_font_chg::_gdimm_font_chg()
{
	get_font_link_info();
	get_font_sub_info();
}

void _gdimm_font_chg::get_font_link_info()
{
	// read font linking information from registry, and store in map

	DWORD dw_ret, enum_ret;
	DWORD curr_index;

	const TCHAR *FontLink = TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\FontLink\\SystemLink");
	const TCHAR *Fonts = TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts");

	HKEY key_fl, key_ft;
	dw_ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, FontLink, 0, KEY_QUERY_VALUE, &key_fl);
	assert(dw_ret == ERROR_SUCCESS);
	dw_ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, Fonts, 0, KEY_QUERY_VALUE, &key_ft);
	assert(dw_ret == ERROR_SUCCESS);

	map<t_string, t_string> fonts_table;

	// get font_file_name -> font_face mapping from the "Fonts" registry key
	curr_index = 0;
	while (true)
	{
		TCHAR value_name[MAX_KEY_NAME];
		TCHAR value_data[MAX_PATH];
		DWORD name_len = MAX_KEY_NAME;
		DWORD data_len = MAX_PATH;

		enum_ret = RegEnumValue(key_ft, curr_index, value_name, &name_len, NULL, NULL, (BYTE*)value_data, &data_len);
		if (enum_ret == ERROR_NO_MORE_ITEMS)
			break;
		assert(enum_ret == ERROR_SUCCESS);

		t_string curr_face = value_name;
		t_string font_file = value_data;
		transform(font_file.begin(), font_file.end(), font_file.begin(), ::tolower);
		curr_face = curr_face.substr(0, curr_face.find('(') - 1);
		fonts_table[font_file] = curr_face;

		curr_index++;
	} while (enum_ret == ERROR_SUCCESS);

	// get font_face -> font_file_name mapping from the "SystemLink" registry key
	// complete the font linking by composing the two mappings
	curr_index = 0;
	while (true)
	{
		TCHAR value_name[MAX_KEY_NAME];
		TCHAR *value_data;
		DWORD name_len = MAX_KEY_NAME;
		DWORD data_len = 0;

		enum_ret = RegEnumValue(key_fl, curr_index, value_name, &name_len, NULL, NULL, NULL, &data_len);
		if (enum_ret == ERROR_NO_MORE_ITEMS)
			break;
		assert(enum_ret == ERROR_SUCCESS);

		name_len = MAX_KEY_NAME;
		value_data = (TCHAR*)malloc(data_len);
		enum_ret = RegEnumValue(key_fl, curr_index, value_name, &name_len, NULL, NULL, (BYTE*)value_data, &data_len);
		assert(enum_ret == ERROR_SUCCESS);

		size_t line_start = 0;
		vector<t_string> font_list;
		
		while (true)
		{
			t_string curr_font = value_data + line_start;
			if (curr_font.length() == 0)
				break;

			size_t first_comma = curr_font.find(TEXT(','));
			t_string font_face;

			// if ttc file, use the specified font face
			// otherwise, use the mapping table
			if (first_comma != string::npos && lstrcmpi(curr_font.substr(first_comma - 3, 3).c_str(), TEXT("ttc")) == 0)
			{
				size_t second_comma = curr_font.find(TEXT(','), first_comma + 1);
				font_face = curr_font.substr(first_comma + 1, second_comma - first_comma - 1);
			}
			else
			{
				transform(curr_font.begin(), curr_font.end(), curr_font.begin(), tolower);
				font_face = fonts_table[curr_font.substr(0, first_comma)];
			}

			if (font_list.size() == 0 || font_face != font_list[font_list.size() - 1])
				font_list.push_back(font_face);

			// including the trailing '\0'
			line_start += curr_font.length() + 1;
		}

		fl_table[value_name] = font_list;
		free(value_data);
		curr_index++;
	} while (enum_ret == ERROR_SUCCESS);

	dw_ret = RegCloseKey(key_ft);
	assert(dw_ret == ERROR_SUCCESS);
	dw_ret = RegCloseKey(key_fl);
	assert(dw_ret == ERROR_SUCCESS);
}

void _gdimm_font_chg::get_font_sub_info()
{
	// read font substitutes information from registry, and store in map

	DWORD dw_ret, enum_ret;
	DWORD curr_index;

	const TCHAR *FontSubstitutes = TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\FontSubstitutes");

	HKEY key_fs;
	dw_ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, FontSubstitutes, 0, KEY_QUERY_VALUE, &key_fs);
	assert(dw_ret == ERROR_SUCCESS);

	curr_index = 0;
	while (true)
	{
		TCHAR value_name[MAX_KEY_NAME];
		TCHAR value_data[MAX_KEY_NAME];
		DWORD name_len = MAX_KEY_NAME;
		DWORD data_len = MAX_KEY_NAME;

		enum_ret = RegEnumValue(key_fs, curr_index, value_name, &name_len, NULL, NULL, (BYTE*)value_data, &data_len);
		if (enum_ret == ERROR_NO_MORE_ITEMS)
			break;
		assert(enum_ret == ERROR_SUCCESS);

		fs_table[value_name] = value_data;
		curr_index++;
	} while (enum_ret == ERROR_SUCCESS);

	dw_ret = RegCloseKey(key_fs);
	assert(dw_ret == ERROR_SUCCESS);
}

const TCHAR *_gdimm_font_chg::fl_lookup(const TCHAR *font_name, size_t index) const
{
	fl_mapping::const_iterator iter = fl_table.find(font_name);

	if (iter == fl_table.end())
		return NULL;
	else
	{
		if (index < iter->second.size())
			return iter->second[iter->second.size() - index - 1].c_str();
		else
			return NULL;
	}
}

const TCHAR *_gdimm_font_chg::fs_lookup(const TCHAR *font_name) const
{
	fs_mapping::const_iterator iter = fs_table.find(font_name);

	if (iter == fs_table.end())
		return NULL;
	else
		return iter->second.c_str();
}