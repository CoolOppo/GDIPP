#include "stdafx.h"
#include "font.h"
#include <Mmsystem.h>

#define mapping_name_prefix TEXT("Global\\gdimm_font_")

gdimm_font::gdimm_font(HDC hdc)
{
	font_holder = hdc;
}

/*gdimm_font::~gdimm_font()
{
	DeleteDC(font_holder);

	// release handle of the file mappings
	// when all processes release handle, font data are released by Windows
	for (font_table::const_iterator iter = loaded_fonts.begin(); iter != loaded_fonts.end(); iter++)
	{
		UnmapViewOfFile(iter->second.data_start);
		CloseHandle(iter->second.file_mapping);
	}
}*/

void gdimm_font::get_mapping_name(HDC hdc)
{
	// the mapping name consists of two parts: prefix and font full name
	// prefix is fixed for all mapping names, defined in mapping_name_prefix
	// font full name is extracted from physical font file via GetOutlineTextMetrics()

	int metric_size;
	metric_size = GetOutlineTextMetrics(hdc, 0, NULL);
	BYTE *metric_buf = new BYTE[metric_size];
	OUTLINETEXTMETRIC *metric = (OUTLINETEXTMETRIC*) metric_buf;
	GetOutlineTextMetrics(hdc, metric_size, metric);
	curr_mapping_name = mapping_name_prefix;
	curr_mapping_name += (TCHAR*)(metric_buf + (UINT)metric->otmpFullName);
	delete[] metric_buf;
}

void gdimm_font::get_font_info(HDC hdc)
{
	curr_font_handle = (HFONT)GetCurrentObject(hdc, OBJ_FONT);
	assert(curr_font_handle != NULL);

	// retrieve LOGFONT from HFONT
	DWORD dwRet = GetObject(curr_font_handle, sizeof(LOGFONT), &curr_font_attr);
	assert(dwRet != 0);

	// if lfWidth is 0, make it equal to lfHeight
	curr_font_attr.lfHeight = abs(curr_font_attr.lfHeight);
	if (curr_font_attr.lfWidth == 0)
		curr_font_attr.lfWidth = curr_font_attr.lfHeight;

	get_mapping_name(hdc);

	// if this process has loaded the font, use the mapped address directly
	font_table::const_iterator iter = loaded_fonts.find(curr_mapping_name);	
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
	DWORD font_size = GetFontData(hdc, font_table, 0, NULL, 0);
	if (font_size == GDI_ERROR)
	{
		font_table = 0;
		font_size = GetFontData(hdc, font_table, 0, NULL, 0);
		assert(font_size != GDI_ERROR);
	}

	// check if the font has been created by other processes
	HANDLE hmapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, curr_mapping_name.c_str());
	bool new_font = (hmapping == NULL);

	if (new_font)
	{
		hmapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, font_size, curr_mapping_name.c_str());
		assert(hmapping != NULL);
	}

	// map memory
	LPVOID mapping_start = MapViewOfFile(hmapping, FILE_MAP_ALL_ACCESS, 0, 0, font_size);
	assert(mapping_start != NULL);

	if (new_font)
	{
		// load font data to the mapped address
		font_size = GetFontData(hdc, font_table, 0, mapping_start, font_size);
		assert(font_size != GDI_ERROR);
	}

	// store mapping information
	font_mapping font_mapping = {mapping_start, font_size, hmapping};
	pair<font_table::const_iterator, bool> ret = loaded_fonts.insert(string_to_mapping(curr_mapping_name, font_mapping));
	curr_font_mapping = &ret.first->second;
}

void gdimm_font::load_font()
{
	get_font_info(font_holder);
}

void gdimm_font::load_font(const TCHAR *font_name)
{
	lstrcpyn(curr_font_attr.lfFaceName, font_name, LF_FACESIZE);
	HFONT linked_font = CreateFontIndirect(&curr_font_attr);
	assert(CreateFontIndirect != NULL);
	HDC hdc_mem = CreateCompatibleDC(font_holder);
	SelectObject(hdc_mem, linked_font);
	get_font_info(hdc_mem);
	DeleteObject(linked_font);
	DeleteObject(hdc_mem);
}