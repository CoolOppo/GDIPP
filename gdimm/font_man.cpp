#include "stdafx.h"
#include "font_man.h"
#include <mmsystem.h>

// table name for GetFontData() to get whole ttc file data
#define ttcf_header mmioFOURCC('t', 't', 'c', 'f')

DWORD _gdimm_font_man::get_font_size(HDC hdc, DWORD *table_header)
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

unsigned long _gdimm_font_man::stream_IoFunc(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count)
{
	// callback function, called when freetype requests font data

	// count == 0 means seek operation
	if (count == 0)
		return 0;

	const font_info info = gdimm_font_man::instance()._loaded_fonts[stream->descriptor.value];

	// use mapping
	/*memcpy(buffer, (unsigned char*) info.mapping_start + offset, count);
	return count;*/

	const DWORD read_size = GetFontData(info.hdc, info.table_header, offset, buffer, count);
	assert(read_size != GDI_ERROR);
	return read_size;
}

void _gdimm_font_man::stream_CloseFunc(FT_Stream stream)
{

}

void _gdimm_font_man::use_mapping(const TCHAR *font_full_name)
{
	// the mapping name consists of two parts: prefix and font full name
	// prefix is fixed for all mapping names, defined in mapping_prefix
	// font full name is extracted from physical font file via GetOutlineTextMetrics()

	const unsigned int font_index = _font_indices[font_full_name];
	font_info &info = _loaded_fonts[font_index];

	const t_string mapping_prefix = TEXT("Global\\gdimm_");
	const t_string mapping_name = mapping_prefix + font_full_name;
	const HANDLE h_mapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, info.stream.size, mapping_name.c_str());
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

unsigned int _gdimm_font_man::lookup_index(HDC hdc, const TCHAR *font_full_name)
{
	const map<t_string, long>::const_iterator iter = _font_indices.find(font_full_name);
	if (iter == _font_indices.end())
	{
		const long new_index = _loaded_fonts.size();
		_font_indices[font_full_name] = new_index;

		font_info new_font = {0};
		new_font.hdc = hdc;
		new_font.stream.size = get_font_size(hdc, &new_font.table_header);
		new_font.stream.descriptor.value = new_index;
		new_font.stream.read = stream_IoFunc;
		new_font.stream.close = stream_CloseFunc;

		_loaded_fonts[new_index] = new_font;
		//use_mapping(font_full_name);
		return new_index;
	}
	else
	{
		// update HDC
		const unsigned int old_index = iter->second;
		_loaded_fonts[old_index].hdc = hdc;
		return old_index;
	}
}

font_info &_gdimm_font_man::get_info(unsigned int index)
{
	return _loaded_fonts[index];
}