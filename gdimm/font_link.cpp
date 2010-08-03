#include "stdafx.h"
#include "font_link.h"

#define MAX_VALUE_NAME 1024

gdimm_font_link::gdimm_font_link()
{
	// read font linking information from registry, and store in map

	LONG l_ret;

	const wchar_t *Fonts = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";
	const wchar_t *FontLink = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\FontLink\\SystemLink";

	HKEY key_ft;
	l_ret = RegOpenKeyExW(HKEY_LOCAL_MACHINE, Fonts, 0, KEY_QUERY_VALUE, &key_ft);
	if (l_ret != ERROR_SUCCESS)
		return;

	HKEY key_fl;
	l_ret = RegOpenKeyExW(HKEY_LOCAL_MACHINE, FontLink, 0, KEY_QUERY_VALUE, &key_fl);
	if (l_ret != ERROR_SUCCESS)
	{
		l_ret = RegCloseKey(key_ft);
		return;
	}

	DWORD value_count;
	DWORD max_data_len;
	wchar_t value_name[MAX_VALUE_NAME];
	BYTE *value_data;

	// font file name -> font face name mapping
	map<wstring, wstring, wstring_ci_less> fonts_table;

	// get font_file_name -> font_face mapping from the "Fonts" registry key

	l_ret = RegQueryInfoKeyW(key_ft, NULL, NULL, NULL, NULL, NULL, NULL, &value_count, NULL, &max_data_len, NULL, NULL);
	assert(l_ret == ERROR_SUCCESS);

	// no font installed
	if (value_count == 0)
		return;

	// max_data_len is in BYTE
	value_data = (BYTE *)malloc(max_data_len);
	assert(value_data != NULL);

	for (DWORD i = 0; i < value_count; i++)
	{
		DWORD name_len = MAX_VALUE_NAME;
		DWORD data_len = max_data_len;

		l_ret = RegEnumValueW(key_ft, i, value_name, &name_len, NULL, NULL, value_data, &data_len);
		assert(l_ret == ERROR_SUCCESS);

		wstring curr_face = value_name;
		wstring font_file = (wchar_t *)value_data;
		curr_face = curr_face.substr(0, curr_face.find('(') - 1);
		fonts_table[font_file] = curr_face;
	}

	// get font_face -> font_file_name mapping from the "SystemLink" registry key
	// complete the font linking by composing the two mappings

	l_ret = RegQueryInfoKey(key_fl, NULL, NULL, NULL, NULL, NULL, NULL, &value_count, NULL, &max_data_len, NULL, NULL);
	assert(l_ret == ERROR_SUCCESS);

	// no font link information
	if (value_count == 0)
		return;

	value_data = (BYTE *)realloc(value_data, max_data_len);
	assert(value_data != NULL);
	
	for (DWORD i = 0; i < value_count; i++)
	{
		DWORD name_len = MAX_VALUE_NAME;
		DWORD data_len = max_data_len;

		l_ret = RegEnumValueW(key_fl, i, value_name, &name_len, NULL, NULL, value_data, &data_len);
		assert(l_ret == ERROR_SUCCESS);

		_link_table[value_name] = vector<font_link_node>();
		size_t line_start = 0;

		set<wstring, wstring_ci_less> curr_font_family_pool;

		while (line_start < data_len - sizeof(wchar_t))
		{
			font_link_node new_link;
			new_link.scaling = 1.0;

			wchar_t *curr_font = (wchar_t *)(value_data + line_start);

			// including the trailing '\0'
			line_start += (wcslen(curr_font) + 1) * sizeof(wchar_t);

			vector<wchar_t *> properties;
			wchar_t *curr_comma = curr_font - 1;
			while (curr_comma != NULL)
			{
				wchar_t *next_comma = wcschr(curr_comma + 1, L',');
				
				if (next_comma != NULL)
				{
					*next_comma = L'\0';
					properties.push_back(next_comma + 1);
				}

				curr_comma = next_comma;
			}

			// font family starts with alphabetic character

			size_t scaling_prop = properties.size();
			if (properties.empty() || !isalpha(*properties[0]))
			{
				// this is not a ttc file
				// lookup the Fonts table
				map<wstring, wstring, wstring_ci_less>::const_iterator iter = fonts_table.find(curr_font);
				if (iter != fonts_table.end())
					new_link.font_family = iter->second;

				scaling_prop = 0;
			}
			else if (isalpha(*properties[0]))
			{
				// this is a ttc file
				// use the specified font face
				if (fonts_table.find(curr_font) != fonts_table.end())
				{
					// trust the face name
					new_link.font_family = properties[0];
				}

				scaling_prop = 1;
			}

			if (scaling_prop + 2 == properties.size())
			{
				// scaling factors are provided
				// use only if both two factors are specified

				int factor1, factor2;
				wstringstream ss;

				ss << properties[scaling_prop];
				ss >> factor1;

				ss.clear();
				ss.str(L"");

				ss << properties[scaling_prop + 1];
				ss >> factor2;

				new_link.scaling = (factor1 / 128.0) * (96.0 / factor2);
			}

			if (!new_link.font_family.empty() && curr_font_family_pool.find(new_link.font_family) == curr_font_family_pool.end())
			{
				_link_table[value_name].push_back(new_link);
				curr_font_family_pool.insert(new_link.font_family);
			}
		}
	}

	free(value_data);

	l_ret = RegCloseKey(key_ft);
	l_ret = RegCloseKey(key_fl);
}

const font_link_node *gdimm_font_link::lookup_link(const wchar_t *font_name, size_t index) const
{
	const link_map::const_iterator iter = _link_table.find(font_name);

	if (iter == _link_table.end())
		return NULL;
	else
	{
		if (index < iter->second.size())
			return &iter->second[index];
		else
			return NULL;
	}
}

size_t gdimm_font_link::get_link_count(const wchar_t *font_name) const
{
	const link_map::const_iterator iter = _link_table.find(font_name);

	if (iter == _link_table.end())
		return 0;
	else
		return iter->second.size();
}