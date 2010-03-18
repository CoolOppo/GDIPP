#include "stdafx.h"
#include "font_man.h"

// table name for GetFontData() to get whole ttc file data
#define TTCF_FONT_TABLE mmioFOURCC('t', 't', 'c', 'f')

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

HFONT _gdimm_font_man::create_linked_font(const LOGFONTW &font_attr, const WCHAR *font_family)
{
	LOGFONTW new_font_attr = font_attr;
	
	/*
	this reset is essential to make GetGlyphIndices work correctly
	for example, lfOutPrecision might be OUT_PS_ONLY_PRECIS for Myriad Pro
	if create HFONT of Microsoft YaHei with such lfOutPrecision, GetGlyphIndices always fails
	*/
	new_font_attr.lfOutPrecision = OUT_DEFAULT_PRECIS;
	wcsncpy_s(new_font_attr.lfFaceName, font_family, LF_FACESIZE);

	const HFONT new_hfont = CreateFontIndirectW(&new_font_attr);
	assert(new_hfont != NULL);
	SelectObject(_font_holder, new_hfont);

	UINT metric_size = GetOutlineTextMetricsW(_font_holder, 0, NULL);
	if (metric_size == 0)
		return NULL;

	_metric_buf.resize(metric_size);
	metric_size = GetOutlineTextMetricsW(_font_holder, metric_size, (OUTLINETEXTMETRICW*) &_metric_buf[0]);
	assert(metric_size != 0);

	return new_hfont;
}

DWORD _gdimm_font_man::get_font_size(HFONT hfont, DWORD &table_header)
{
	SelectObject(_font_holder, hfont);

	table_header = TTCF_FONT_TABLE;

	// specify 0 as table index to retrieve font data from ttf files
	// specify ttcf to retrieve from ttc files
	DWORD font_size = GetFontData(_font_holder, table_header, 0, NULL, 0);
	if (font_size == GDI_ERROR)
	{
		table_header = 0;
		font_size = GetFontData(_font_holder, table_header, 0, NULL, 0);
		assert(font_size != GDI_ERROR);
	}

	return font_size;
}

void _gdimm_font_man::use_mapping(const WCHAR *font_face)
{
	// the mapping name consists of two parts: prefix and font full name
	// prefix is fixed for all mapping names, defined in mapping_prefix
	// font full name is extracted from physical font file via GetOutlineTextMetrics()

	const unsigned int font_id = _font_ids[font_face];
	font_info &info = _loaded_fonts[font_id];

	const wstring mapping_prefix = L"Global\\gdimm_";
	const wstring mapping_name = mapping_prefix + font_face;
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

long _gdimm_font_man::register_font(HFONT new_hfont, const WCHAR *font_face, bool *font_exist)
{
	SelectObject(_font_holder, new_hfont);

	const map<wstring, long>::const_iterator iter = _font_ids.find(font_face);
	bool found_hfont = (iter != _font_ids.end());

	if (font_exist != NULL)
		*font_exist = found_hfont;

	if (found_hfont)
	{
		_loaded_fonts[iter->second].hfont = new_hfont;
		return iter->second;
	}
	else
	{
		const long new_font_id = _loaded_fonts.size();
		_font_ids[font_face] = new_font_id;

		font_info new_font_info = {0};
		new_font_info.hfont = new_hfont;
		new_font_info.owned = false;
		new_font_info.stream.size = get_font_size(new_hfont, new_font_info.table_header);
		new_font_info.stream.descriptor.value = new_font_id;
		new_font_info.stream.read = stream_IoFunc;
		new_font_info.stream.close = stream_CloseFunc;

		_loaded_fonts[new_font_id] = new_font_info;
		//use_mapping(font_face);
		return new_font_id;
	}
}

long _gdimm_font_man::lookup_font(const LOGFONTW &font_attr, const WCHAR *font_family, const WCHAR *&font_face)
{
	const HFONT linked_font = create_linked_font(font_attr, font_family);
	if (linked_font == NULL)
		return -1;

	// check if the linked font is registered
	font_face = metric_face_name(&_metric_buf[0]);
	bool font_exist;
	const long font_id = register_font(linked_font, font_face, &font_exist);
	_loaded_fonts[font_id].owned = true;

	// if registered, use the existing hfont, and delete the new one
	if (font_exist)
	{
		SelectObject(_font_holder, _loaded_fonts[font_id].hfont);
		DeleteObject(linked_font);
	}

	return font_id;
}

WORD _gdimm_font_man::get_glyph_index(WCHAR ch)
{
	WORD glyph_index;

	DWORD converted = GetGlyphIndices(_font_holder, &ch, 1, &glyph_index, GGI_MARK_NONEXISTING_GLYPHS);
	assert(converted == 1);

	return glyph_index;
}