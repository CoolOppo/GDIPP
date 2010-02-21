#include "stdafx.h"
#include "font_link.h"
#include <string>
#include <algorithm>
using namespace std;

#define MAX_KEY_NAME 255
#define ERROR_MSG_LEN 1024

void _gdimm_font_link::get_font_link_info()
{
	// read font linking information from registry, and store in map

	DWORD dw_ret;
	DWORD enum_ret;
	DWORD curr_index;

	const WCHAR *FontLink = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\FontLink\\SystemLink";
	const WCHAR *Fonts = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";

	HKEY key_fl, key_ft;
	dw_ret = RegOpenKeyExW(HKEY_LOCAL_MACHINE, FontLink, 0, KEY_QUERY_VALUE, &key_fl);
	assert(dw_ret == ERROR_SUCCESS);
	dw_ret = RegOpenKeyExW(HKEY_LOCAL_MACHINE, Fonts, 0, KEY_QUERY_VALUE, &key_ft);
	assert(dw_ret == ERROR_SUCCESS);

	map<wstring, wstring> fonts_table;

	// get font_file_name -> font_face mapping from the "Fonts" registry key
	curr_index = 0;
	while (true)
	{
		WCHAR value_name[MAX_KEY_NAME];
		WCHAR value_data[MAX_PATH];
		DWORD name_len = MAX_KEY_NAME;
		DWORD data_len = MAX_PATH;

		enum_ret = RegEnumValueW(key_ft, curr_index, value_name, &name_len, NULL, NULL, (BYTE*) value_data, &data_len);
		if (enum_ret == ERROR_NO_MORE_ITEMS)
			break;
		assert(enum_ret == ERROR_SUCCESS);

		wstring curr_face = value_name;
		wstring font_file = value_data;
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
		WCHAR value_name[MAX_KEY_NAME];
		WCHAR *value_data;
		DWORD name_len = MAX_KEY_NAME;
		DWORD data_len = 0;

		enum_ret = RegEnumValueW(key_fl, curr_index, value_name, &name_len, NULL, NULL, NULL, &data_len);
		if (enum_ret == ERROR_NO_MORE_ITEMS)
			break;
		assert(enum_ret == ERROR_SUCCESS);

		name_len = MAX_KEY_NAME;
		value_data = (WCHAR*) malloc(data_len);
		enum_ret = RegEnumValueW(key_fl, curr_index, value_name, &name_len, NULL, NULL, (BYTE*) value_data, &data_len);
		assert(enum_ret == ERROR_SUCCESS);

		size_t line_start = 0;
		vector<wstring> font_list;
		
		while (true)
		{
			wstring curr_font = value_data + line_start;
			if (curr_font.length() == 0)
				break;

			const size_t first_comma = curr_font.find(L',');
			wstring font_name;

			// if ttc file, use the specified font face
			// otherwise, use the mapping table
			if (first_comma != wstring::npos && _wcsicmp(curr_font.substr(first_comma - 3, 3).c_str(), L"ttc") == 0)
			{
				size_t second_comma = curr_font.find(L',', first_comma + 1);
				font_name = curr_font.substr(first_comma + 1, second_comma - first_comma - 1);
			}
			else
			{
				transform(curr_font.begin(), curr_font.end(), curr_font.begin(), tolower);
				font_name = fonts_table[curr_font.substr(0, first_comma)];
			}

			if (font_list.empty() || font_name != font_list[font_list.size() - 1])
				font_list.push_back(font_name);

			// including the trailing '\0'
			line_start += curr_font.length() + 1;
		}

		_link_table[value_name] = font_list;
		free(value_data);
		curr_index++;
	} while (enum_ret == ERROR_SUCCESS);

	dw_ret = RegCloseKey(key_ft);
	assert(dw_ret == ERROR_SUCCESS);
	dw_ret = RegCloseKey(key_fl);
	assert(dw_ret == ERROR_SUCCESS);
}

const WCHAR *_gdimm_font_link::lookup(const WCHAR *font_name, size_t index) const
{
	const map<wstring, vector<wstring>>::const_iterator iter = _link_table.find(font_name);

	if (iter == _link_table.end())
		return NULL;
	else
	{
		if (index < iter->second.size())
			return iter->second[iter->second.size() - index - 1].c_str();
		else
			return NULL;
	}
}

const size_t _gdimm_font_link::get_link_count(const WCHAR *font_name) const
{
	const map<wstring, vector<wstring>>::const_iterator iter = _link_table.find(font_name);

	if (iter == _link_table.end())
		return 0;
	else
		return iter->second.size();
}