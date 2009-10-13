#pragma once

#include <map>
#include <string>
using namespace std;

// font mapping information
struct FontMapping
{
	void *data_start;
	DWORD data_length;
	HANDLE file_mapping;
};

class gdimm_Font
{
private:
	// the font table maps the File Mapping name to the mapping information
	typedef basic_string<TCHAR> t_string;
	typedef pair<t_string, FontMapping> String2Mapping;
	typedef map<t_string, FontMapping> FontTable;

	FontTable loaded_fonts;
	HDC font_holder;	// used to extract font information
	t_string curr_mapping_name;

	void UpdateMappingName();

public:
	HFONT curr_font_handle;
	LOGFONT curr_font_attr;
	const FontMapping *curr_font_mapping;

	gdimm_Font();
	~gdimm_Font();
	void GetFontInfo();
	void GetFontInfo(HDC hdc);
};