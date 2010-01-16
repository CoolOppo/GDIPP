#include "stdafx.h"
#include "fontlink.h"
#include <algorithm>

#define MAX_VALUE_NAME 16383
#define ERROR_MSG_LEN 1024

gdimm_fontlink *gdimm_fontlink::_instance = NULL;

gdimm_fontlink::gdimm_fontlink()
{
	get_fontlink_info();
}

void gdimm_fontlink::get_fontlink_info()
{
	DWORD ret, enum_ret;
	DWORD curr_index;

	const TCHAR *FontLink = TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\FontLink\\SystemLink");
	const TCHAR *Fonts = TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts");

	HKEY key_fl, key_ft;
	ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, FontLink, 0, KEY_QUERY_VALUE, &key_fl);
	assert(ret == ERROR_SUCCESS);
	ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, Fonts, 0, KEY_QUERY_VALUE, &key_ft);
	assert(ret == ERROR_SUCCESS);

	map<t_string, t_string> fonts_table;

	curr_index = 0;
	do 
	{
		TCHAR value_name[MAX_VALUE_NAME];
		TCHAR value_data[MAX_PATH];
		DWORD name_len = MAX_VALUE_NAME;
		DWORD data_len = MAX_PATH;
		DWORD value_type;

		enum_ret = RegEnumValue(key_ft, curr_index, value_name, &name_len, NULL, &value_type, (BYTE*)value_data, &data_len);

		t_string curr_face = value_name;
		t_string font_file = value_data;
		transform(font_file.begin(), font_file.end(), font_file.begin(), ::tolower);
		curr_face = curr_face.substr(0, curr_face.find('(') - 1);
		fonts_table[font_file] = curr_face;

		curr_index++;
	} while (enum_ret == ERROR_SUCCESS);

	curr_index = 0;
	do 
	{
		TCHAR value_name[MAX_VALUE_NAME];
		TCHAR value_data[MAX_PATH];
		DWORD name_len = MAX_VALUE_NAME;
		DWORD data_len = MAX_PATH;
		DWORD value_type;

		enum_ret = RegEnumValue(key_fl, curr_index, value_name, &name_len, NULL, &value_type, (BYTE*)value_data, &data_len);

		size_t line_start = 0;
		vector<t_string> font_list;
		
		while (true)
		{
			t_string curr_font = value_data + line_start;
			if (curr_font.length() == 0)
				break;

			size_t comma_pos = curr_font.find(',');
			t_string font_face;

			if (comma_pos == string::npos)
			{
				transform(curr_font.begin(), curr_font.end(), curr_font.begin(), ::tolower);
				font_face = fonts_table[curr_font];
			}
			else
				font_face = curr_font.substr(comma_pos + 1);

			font_list.push_back(font_face);

			line_start += curr_font.length() + 1;
		}

		fl_table[value_name] = font_list;
		curr_index++;
	} while (enum_ret == ERROR_SUCCESS);

	ret = RegCloseKey(key_ft);
	assert(ret == ERROR_SUCCESS);
	ret = RegCloseKey(key_fl);
	assert(ret == ERROR_SUCCESS);
}

gdimm_fontlink *gdimm_fontlink::instance()
{
	if (!_instance)
		_instance = new gdimm_fontlink();

	return _instance;
}

const TCHAR *gdimm_fontlink::lookup(const TCHAR *font_name, size_t index) const
{
	fl_mapping::const_iterator iter = fl_table.find(font_name);

	if (iter == fl_table.end())
		return NULL;
	else
	{
		if (index < iter->second.size())
			return iter->second[index].c_str();
		else
			return NULL;
	}
}