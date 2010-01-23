#include "stdafx.h"
#include "font.h"
#include <mmsystem.h>

#define mapping_name_prefix TEXT("Global\\gdimm_")

// table name for GetFontData() to get whole ttc file data
#define ttcf_header mmioFOURCC('t', 't', 'c', 'f')

DWORD _gdimm_font::get_font_size(HDC hdc, DWORD *table_header)
{
	DWORD header = ttcf_header;

	// specify 0 as table index to retrieve font data from ttf files
	// specify ttcf to retrieve from ttc files
	DWORD font_size = GetFontData(hdc, header, 0, NULL, 0);
	if (font_size == GDI_ERROR)
	{
		header = 0;
		font_size = GetFontData(hdc, header, 0, NULL, 0);
		assert(font_size != GDI_ERROR);
	}

	if (table_header != NULL)
		*table_header = header;

	return font_size;
}

unsigned long _gdimm_font::stream_IoFunc(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count)
{
	// callback function, called when freetype requests font data

	// count == 0 means seek operation
	if (count == 0)
		return 0;

	font_info info = gdimm_font::instance().loaded_fonts[stream->descriptor.value];

	// use mapping
	/*memcpy(buffer, (unsigned char*)info.mapping_start + offset, count);
	return count;*/

	DWORD read_size = GetFontData(info.hdc, info.table_header, offset, buffer, count);
	assert(read_size != GDI_ERROR);
	return read_size;
}

void _gdimm_font::stream_CloseFunc(FT_Stream stream)
{

}

void _gdimm_font::use_mapping(const t_string &font_full_name)
{
	unsigned int font_index = font_indices[font_full_name];
	font_info &info = loaded_fonts[font_index];

	t_string mapping_name = mapping_name_prefix + font_full_name;
	HANDLE h_mapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, info.stream.size, mapping_name.c_str());
	assert(h_mapping != NULL);
	bool new_mapping = (GetLastError() != ERROR_ALREADY_EXISTS);

	// map memory
	info.mapping_start = MapViewOfFile(h_mapping, FILE_MAP_ALL_ACCESS, 0, 0, info.stream.size);
	assert(info.mapping_start != NULL);

	if (new_mapping)
	{
		// load font data to the mapped address
		DWORD read_size = GetFontData(info.hdc, info.table_header, 0, info.mapping_start, info.stream.size);
		assert(read_size != GDI_ERROR);
	}
}

LOGFONT _gdimm_font::get_font_attr(HDC hdc)
{
	HFONT curr_font_handle = (HFONT) GetCurrentObject(hdc, OBJ_FONT);
	assert(curr_font_handle != NULL);

	// retrieve LOGFONT from HFONT
	LOGFONT font_attr;
	int obj_size = GetObject(curr_font_handle, sizeof(LOGFONT), &font_attr);
	assert(obj_size != 0);

	// if lfWidth is 0, make it equal to lfHeight
	font_attr.lfHeight = abs(font_attr.lfHeight);
	if (font_attr.lfWidth == 0)
		font_attr.lfWidth = font_attr.lfHeight;

	return font_attr;
}

t_string _gdimm_font::get_font_full_name(HDC hdc)
{
	// the mapping name consists of two parts: prefix and font full name
	// prefix is fixed for all mapping names, defined in mapping_name_prefix
	// font full name is extracted from physical font file via GetOutlineTextMetrics()

	int metric_size = GetOutlineTextMetrics(hdc, 0, NULL);
	assert(metric_size != 0);

	BYTE *metric_buf = new BYTE[metric_size];
	OUTLINETEXTMETRIC *metric = (OUTLINETEXTMETRIC*) metric_buf;
	metric_size = GetOutlineTextMetrics(hdc, metric_size, metric);
	assert(metric_size != 0);

	t_string full_name = (TCHAR*)(metric_buf + (UINT)metric->otmpFullName);

	delete[] metric_buf;
	return full_name;
}

unsigned int _gdimm_font::lookup_index(const t_string &font_full_name, HDC hdc)
{
	map<t_string, unsigned int>::const_iterator iter = font_indices.find(font_full_name);
	if (iter == font_indices.end())
	{
		unsigned int new_index = loaded_fonts.size();
		font_indices[font_full_name] = new_index;

		font_info new_font;
		new_font.hdc = hdc;

		memset(&new_font.stream, 0, sizeof(new_font.stream));
		new_font.stream.base = NULL;
		new_font.stream.size = get_font_size(hdc, &new_font.table_header);
		new_font.stream.pos = 0;
		new_font.stream.descriptor.value = new_index;
		new_font.stream.read = stream_IoFunc;
		new_font.stream.close = stream_CloseFunc;

		loaded_fonts[new_index] = new_font;
		//use_mapping(font_full_name);
		return new_index;
	}
	else
	{
		// update HDC
		unsigned int old_index = iter->second;
		loaded_fonts[old_index].hdc = hdc;
		return old_index;
	}
}

font_info &_gdimm_font::get_info(unsigned int index)
{
	return loaded_fonts[index];
}