#include "stdafx.h"
#include "font.h"
#include <Mmsystem.h>

#define mapping_name_prefix TEXT("Global\\gdimm_font_")

bool operator<(const FontMapping &mapping1, const FontMapping &mapping2)
{
	return (memcmp(&mapping1, &mapping2, sizeof(FontMapping)) < 0);
}

gdimm_Font::gdimm_Font()
{
	font_holder = CreateCompatibleDC(NULL);
}

gdimm_Font::~gdimm_Font()
{
	DeleteDC(font_holder);

	// release handle of the file mappings
	// when all processes release handle, font data are released by Windows
	for (FontTable::const_iterator iter = loaded_fonts.begin(); iter != loaded_fonts.end(); iter++)
	{
		UnmapViewOfFile(iter->second.data_start);
		CloseHandle(iter->second.file_mapping);
	}
}

void gdimm_Font::UpdateMappingName()
{
	// the mapping name consists of two parts: prefix and font full name
	// prefix is fixed for all mapping names, defined in mapping_name_prefix
	// font full name is extracted from physical font file via GetOutlineTextMetrics()

	int metric_size;
	metric_size = GetOutlineTextMetrics(font_holder, 0, NULL);
	BYTE *metric_buf = new BYTE[metric_size];
	OUTLINETEXTMETRIC *metric = (OUTLINETEXTMETRIC*) metric_buf;
	GetOutlineTextMetrics(font_holder, metric_size, metric);
	curr_mapping_name = mapping_name_prefix;
	curr_mapping_name += (TCHAR*)(metric_buf + (UINT)metric->otmpFullName);
	delete[] metric_buf;
}

void gdimm_Font::GetFontInfo()
{
	// pre-condition: the current font handle is in curr_font_handle

	// retrieve LOGFONT from HFONT
	DWORD dwRet = GetObject(curr_font_handle, sizeof(LOGFONT), &curr_font_attr);
	assert(dwRet != 0);

	// if lfWidth is 0, make it equal to lfHeight
	curr_font_attr.lfHeight = abs(curr_font_attr.lfHeight);
	if (curr_font_attr.lfWidth == 0)
		curr_font_attr.lfWidth = curr_font_attr.lfHeight;

	SelectObject(font_holder, curr_font_handle);

	UpdateMappingName();

	// if this process has loaded the font, use the mapped address directly
	FontTable::const_iterator iter = loaded_fonts.find(curr_mapping_name);	
	if (iter != loaded_fonts.end())
	{
		curr_font_mapping = &iter->second;
		return;
	}

	// otherwise, load the font data, copy to the mapped memory, and store the mapping information

	// table name for GetFontData() to get whole ttc file data
	DWORD font_table = mmioFOURCC('t', 't', 'c', 'f');
	// specify 0 as table index to retrieve font data from ttf files
	// specify ttcf to retrieve from ttc files
	DWORD font_size = GetFontData(font_holder, font_table, 0, NULL, 0);
	if (font_size == GDI_ERROR)
	{
		font_table = 0;
		font_size = GetFontData(font_holder, font_table, 0, NULL, 0);
		assert(font_size != GDI_ERROR);
	}

	// check if the font has been created by other processes
	HANDLE hmapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, curr_mapping_name.c_str());
	bool new_font = (hmapping == NULL);
	BYTE *font_data;

	if (new_font)
	{
		// if not created yet, load the font and create file mapping
		font_data = new BYTE[font_size];
		font_size = GetFontData(font_holder, font_table, 0, font_data, font_size);
		assert(font_size != GDI_ERROR);

		hmapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, font_size, curr_mapping_name.c_str());
		assert(hmapping != NULL);
	}

	// map memory
	LPVOID mapping_start = MapViewOfFile(hmapping, FILE_MAP_ALL_ACCESS, 0, 0, font_size);
	assert(mapping_start != NULL);

	if (new_font)
	{
		// copy font data to the mapped address
		memcpy(mapping_start, font_data, font_size);
		delete[] font_data;
	}

	// store mapping information
	FontMapping font_mapping = {mapping_start, font_size, hmapping};
	pair<FontTable::const_iterator, bool> ret = loaded_fonts.insert(String2Mapping(curr_mapping_name, font_mapping));
	curr_font_mapping = &ret.first->second;
}

void gdimm_Font::GetFontInfo(HDC hdc)
{
	// get font handle from hdc
	curr_font_handle = (HFONT)GetCurrentObject(hdc, OBJ_FONT);
	assert(curr_font_handle != NULL);

	return GetFontInfo();
}