#include "stdafx.h"
#include "gdimm.h"

// table name for GetFontData() to get whole ttc file data
#define TTCF_FONT_TABLE mmioFOURCC('t', 't', 'c', 'f')

gdimm_font_man::~gdimm_font_man()
{
	// delete fonts created by the font manager

	for (map<long, font_info>::const_iterator iter = _loaded_fonts.begin(); iter != _loaded_fonts.end(); iter++)
	{
		if (iter->second.owned)
			DeleteObject(iter->second.hfont);
	}
}

unsigned long gdimm_font_man::stream_IoFunc(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count)
{
	// callback function, called when freetype requests font data

	// count == 0 means seek operation
	if (count == 0)
		return 0;

	const font_info &info = font_man_instance._loaded_fonts[stream->descriptor.value];

	// use mapping
	/*memcpy(buffer, (unsigned char*) info.mapping_start + offset, count);
	return count;*/

	const DWORD read_size = GetFontData(info.font_holder, info.table_header, offset, buffer, count);
	assert(read_size != GDI_ERROR);
	return read_size;
}

HFONT gdimm_font_man::create_linked_font(HDC font_holder, const LOGFONTW &font_attr, const WCHAR *font_family, const WCHAR *&font_face)
{
	LOGFONTW new_font_attr = font_attr;
	
	/*
	this reset is essential to make GetGlyphIndices work correctly
	for example, lfOutPrecision might be OUT_PS_ONLY_PRECIS for Myriad Pro
	if create HFONT of Microsoft YaHei with such lfOutPrecision, GetGlyphIndices always fails
	*/
	new_font_attr.lfOutPrecision = OUT_DEFAULT_PRECIS;
	wcsncpy_s(new_font_attr.lfFaceName, font_family, LF_FACESIZE);

	HFONT new_hfont = CreateFontIndirectW(&new_font_attr);
	assert(new_hfont != NULL);
	SelectObject(font_holder, new_hfont);

	UINT metric_size = GetOutlineTextMetricsW(font_holder, 0, NULL);
	if (metric_size == 0)
		return NULL;

	vector<BYTE> metric_buf;
	metric_buf.resize(metric_size);
	metric_size = GetOutlineTextMetricsW(font_holder, metric_size, (OUTLINETEXTMETRICW*) &metric_buf[0]);
	assert(metric_size != 0);

	font_face = metric_face_name(&metric_buf[0]);

	return new_hfont;
}

DWORD gdimm_font_man::get_font_size(HDC font_holder, HFONT hfont, DWORD &table_header)
{
	table_header = TTCF_FONT_TABLE;

	// specify 0 as table index to retrieve font data from ttf files
	// specify ttcf to retrieve from ttc files
	DWORD font_size = GetFontData(font_holder, table_header, 0, NULL, 0);
	if (font_size == GDI_ERROR)
	{
		table_header = 0;
		font_size = GetFontData(font_holder, table_header, 0, NULL, 0);
		assert(font_size != GDI_ERROR);
	}

	return font_size;
}

void gdimm_font_man::use_mapping(HDC font_holder, const WCHAR *font_face)
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
		DWORD read_size = GetFontData(font_holder, info.table_header, 0, info.mapping_start, info.stream.size);
		assert(read_size != GDI_ERROR);
	}
}

long gdimm_font_man::register_font(HDC font_holder, HFONT new_hfont, const WCHAR *font_face)
{
	SelectObject(font_holder, new_hfont);

	const map<wstring, long>::const_iterator iter = _font_ids.find(font_face);
	if (iter == _font_ids.end())
	{
		const long new_font_id = _loaded_fonts.size();
		_font_ids[font_face] = new_font_id;

		font_info new_font_info = {0};
		new_font_info.font_holder = font_holder;
		new_font_info.hfont = new_hfont;
		new_font_info.owned = false;
		new_font_info.stream.size = get_font_size(font_holder, new_hfont, new_font_info.table_header);
		new_font_info.stream.descriptor.value = new_font_id;
		new_font_info.stream.read = stream_IoFunc;
		new_font_info.stream.close = stream_CloseFunc;

		_loaded_fonts[new_font_id] = new_font_info;
		//use_mapping(font_face);
		return new_font_id;
	}
	else
	{
		// if the existing font is created by us, delete it
		if (_loaded_fonts[iter->second].owned)
			DeleteObject(_loaded_fonts[iter->second].hfont);

		// update the cache with the new hfont
		_loaded_fonts[iter->second].font_holder = font_holder;
		_loaded_fonts[iter->second].hfont = new_hfont;
		_loaded_fonts[iter->second].owned = false;

		return iter->second;
	}
}

long gdimm_font_man::lookup_font(HDC font_holder, const LOGFONTW &font_attr, const WCHAR *font_family, const WCHAR *&font_face)
{
	HFONT linked_font = create_linked_font(font_holder, font_attr, font_family, font_face);
	if (linked_font == NULL)
		return -1;

	const long font_id = register_font(font_holder, linked_font, font_face);
	_loaded_fonts[font_id].owned = true;

	return font_id;
}

WORD gdimm_font_man::get_glyph_index(HDC font_holder, WCHAR ch)
{
	WORD glyph_index;

	DWORD converted = GetGlyphIndices(font_holder, &ch, 1, &glyph_index, GGI_MARK_NONEXISTING_GLYPHS);
	assert(converted == 1);

	return glyph_index;
}