#include "stdafx.h"
#include "font_man.h"
#include "freetype.h"
#include "gdimm.h"
#include "helper_func.h"

gdimm_font_man::gdimm_font_man()
{
	_linked_font_holder = CreateCompatibleDC(NULL);
}

gdimm_font_man::~gdimm_font_man()
{
	DeleteDC(_linked_font_holder);
}

unsigned long gdimm_font_man::stream_io(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count)
{
	// callback function, called when freetype requests font data

	// count == 0 means seek operation
	if (count == 0)
		return 0;

	const long font_id = stream->descriptor.value;
	const gdimm_font_man *font_man = font_store_instance.lookup_thread_font_man();
	HDC font_holder;

	if (font_id >= 0)
		font_holder = font_man->_reg_font_holder;
	else
		font_holder = font_man->_linked_font_holder;

	const font_info *info = font_store_instance.lookup_font(font_id);
	assert(info != NULL);
	
	const DWORD read_size = GetFontData(font_holder, info->table_header, offset, buffer, count);
	assert(read_size == count);

	return read_size;
}

void gdimm_font_man::stream_close(FT_Stream stream)
{
	// GetFontData() needs no close
}

FT_Stream gdimm_font_man::lookup_stream(long font_id)
{
	font_info *info = font_store_instance.lookup_font(font_id);
	assert(info != NULL);

	return &info->stream;
}

ULONG gdimm_font_man::lookup_face_index(long font_id)
{
	const font_info *info = font_store_instance.lookup_font(font_id);
	assert(info != NULL);

	return info->face_index;
}

const gdimm_os2_metrics *gdimm_font_man::lookup_os2_metrics(long font_id)
{
	const font_info *info = font_store_instance.lookup_font(font_id);
	assert(info != NULL);

	return &info->os2_metrics;
}

int gdimm_font_man::lookup_kern(const FTC_Scaler scaler, WORD left_glyph, WORD right_glyph)
{
	FT_Error ft_error;

	FT_Size size;
	ft_error = FTC_Manager_LookupSize(ft_cache_man, scaler, &size);
	assert(ft_error == 0);

	FT_Vector delta;
	ft_error = FT_Get_Kerning(size->face, left_glyph, right_glyph, FT_KERNING_DEFAULT, &delta);
	assert(ft_error == 0);

	return int_from_26dot6(delta.x);
}

long gdimm_font_man::register_font(HDC font_holder, const wchar_t *font_face)
{
	BOOL b_ret;

	_reg_font_holder = font_holder;
	const long font_id = font_store_instance.register_font(font_holder, font_face);
	
	b_ret = font_store_instance.register_thread_font_man(this);
	assert(b_ret);
	
	return font_id;
}

long gdimm_font_man::link_font(const LOGFONTW &linked_font_attr, wstring &linked_font_face)
{
	BOOL b_ret;

	const HFONT linked_hfont = CreateFontIndirectW(&linked_font_attr);
	if (linked_hfont == NULL)
		return 0;

	SelectObject(_linked_font_holder, linked_hfont);

	const long font_id = font_store_instance.link_font(_linked_font_holder, linked_hfont, linked_font_face);
	if (font_id == 0)
		return 0;

	b_ret = font_store_instance.register_thread_font_man(this);
	assert(b_ret);

	return font_id;
}

void gdimm_font_man::get_glyph_indices(long font_id, const wchar_t *str, int count, wchar_t *gi)
{
	HDC font_holder;

	if (font_id >= 0)
		font_holder = _reg_font_holder;
	else
		font_holder = _linked_font_holder;

	DWORD converted = GetGlyphIndices(font_holder, str, count, reinterpret_cast<LPWORD>(gi), GGI_MARK_NONEXISTING_GLYPHS);
	assert(converted == count);
}