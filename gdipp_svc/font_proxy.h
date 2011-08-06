#pragma once

#include <map>

class gdimm_font_man
{
	/*
	font manager manages two kinds of font: local font and remote font

	registered fonts are created outside gdipp
	they are considered temporary, not managed by the font manager
	registered fonts have non-negative font id

	linked fonts are created by gdipp for font linking
	every linked font are kept alive until the font storage is destructed
	linked fonts have negative font id
	*/

public:
	static unsigned long stream_io(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count);
	static void stream_close(FT_Stream stream);
	static int lookup_kern(const FTC_Scaler scaler, WORD left_glyph, WORD right_glyph);

	gdimm_font_man();
	~gdimm_font_man();

	const font_info *get_font_info(long font_id);
	FT_Stream lookup_stream(long font_id);
	ULONG lookup_face_index(long font_id);
	const os2_metrics *lookup_os2_metrics(long font_id);

	long register_font(HDC font_holder, const wchar_t *font_face);
	long link_font(const LOGFONTW &linked_font_attr, std::wstring &linked_font_face);
	void get_glyph_indices(long font_id, const wchar_t *str, int count, wchar_t *gi);

private:
	static DWORD get_font_size(HDC font_holder, DWORD &table_header);
	static DWORD get_font_size(long font_id, DWORD &table_header);
	static ULONG get_ttc_face_index(HDC font_holder, DWORD ttc_file_size);
	static ULONG get_ttc_face_index(long font_id, DWORD ttc_file_size);

	// face name -> font id
	// we use this std::map because FreeType callback only have face id
	std::map<std::wstring, long> _name_to_id;
	// font id -> font info
	// we use this std::map because std::vector internally free and re-allocate existing entries
	// pointers become invalid
	std::map<long, font_info> _id_to_info;
};
