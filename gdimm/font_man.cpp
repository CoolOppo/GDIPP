#include "stdafx.h"
#include "font_man.h"
#include <mmsystem.h>
#include <string>
using namespace std;

// table name for GetFontData() to get whole ttc file data
#define ttcf_header mmioFOURCC('t', 't', 'c', 'f')

_gdimm_font_man::~_gdimm_font_man()
{
	// delete fonts created by the font manager

	for (map<long, font_info>::const_iterator iter = _loaded_fonts.begin(); iter != _loaded_fonts.end(); iter++)
	{
		if (iter->second.owned)
			DeleteObject(iter->second.hfont);
	}
}

unsigned long _gdimm_font_man::stream_IoFunc(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count)
{
	// callback function, called when freetype requests font data

	// count == 0 means seek operation
	if (count == 0)
		return 0;

	const font_info &info = gdimm_font_man::instance()._loaded_fonts[stream->descriptor.value];

	// use mapping
	/*memcpy(buffer, (unsigned char*) info.mapping_start + offset, count);
	return count;*/

	const DWORD read_size = GetFontData(gdimm_font_man::instance()._font_holder, info.table_header, offset, buffer, count);
	assert(read_size != GDI_ERROR);
	return read_size;
}

DWORD _gdimm_font_man::get_font_size(HFONT hfont, DWORD *table_header)
{
	SelectObject(_font_holder, hfont);

	DWORD header = ttcf_header;

	// specify 0 as table index to retrieve font data from ttf files
	// specify ttcf to retrieve from ttc files
	DWORD font_size = GetFontData(_font_holder, header, 0, NULL, 0);
	if (font_size == GDI_ERROR)
	{
		header = 0;
		font_size = GetFontData(_font_holder, header, 0, NULL, 0);
		assert(font_size != GDI_ERROR);
	}

	if (table_header != NULL)
		*table_header = header;

	return font_size;
}

void _gdimm_font_man::use_mapping(const WCHAR *font_full_name)
{
	// the mapping name consists of two parts: prefix and font full name
	// prefix is fixed for all mapping names, defined in mapping_prefix
	// font full name is extracted from physical font file via GetOutlineTextMetrics()

	const unsigned int font_id = _font_ids[font_full_name];
	font_info &info = _loaded_fonts[font_id];

	const wstring mapping_prefix = L"Global\\gdimm_";
	const wstring mapping_name = mapping_prefix + font_full_name;
	const HANDLE h_mapping = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, info.stream.size, mapping_name.c_str());
	assert(h_mapping != NULL);
	const bool new_mapping = (GetLastError() != ERROR_ALREADY_EXISTS);

	// map memory
	info.mapping_start = MapViewOfFile(h_mapping, FILE_MAP_ALL_ACCESS, 0, 0, info.stream.size);
	assert(info.mapping_start != NULL);

	if (new_mapping)
	{
		// load font data to the mapped address
		DWORD read_size = GetFontData(_font_holder, info.table_header, 0, info.mapping_start, info.stream.size);
		assert(read_size != GDI_ERROR);
	}
}

long _gdimm_font_man::register_font(HFONT hfont, const WCHAR *font_family, const WCHAR *font_style)
{
	const wstring font_full_name = wstring(font_family) + L" " + font_style;
	const map<wstring, long>::const_iterator iter = _font_ids.find(font_full_name.c_str());

	SelectObject(_font_holder, hfont);

	if (iter == _font_ids.end())
	{
		const long new_font_id = _loaded_fonts.size();
		_font_ids[font_full_name] = new_font_id;

		font_info new_font = {0};
		new_font.hfont = hfont;
		new_font.owned = false;
		new_font.stream.size = get_font_size(hfont, &new_font.table_header);
		new_font.stream.descriptor.value = new_font_id;
		new_font.stream.read = stream_IoFunc;
		new_font.stream.close = stream_CloseFunc;

		_loaded_fonts[new_font_id] = new_font;
		//use_mapping(font_full_name);
		return new_font_id;
	}
	else
	{
		_loaded_fonts[iter->second].hfont = hfont;
		return iter->second;
	}
}

long _gdimm_font_man::lookup_font(const LOGFONTW &font_attr, const WCHAR *font_family, const WCHAR *font_style)
{
	const wstring font_full_name = wstring(font_family) + L" " + font_style;
	const map<wstring, long>::const_iterator iter = _font_ids.find(font_full_name.c_str());

	if (iter == _font_ids.end())
	{
		LOGFONTW linked_font_attr = font_attr;
		wcsncpy_s(linked_font_attr.lfFaceName, font_family, LF_FACESIZE);
		HFONT new_hfont = CreateFontIndirectW(&linked_font_attr);

		long font_id = register_font(new_hfont, font_family, font_style);
		_loaded_fonts[font_id].owned = true;

		return font_id;
	}
	else
	{
		long font_id = iter->second;
		SelectObject(_font_holder, _loaded_fonts[font_id].hfont);

		return font_id;
	}
}