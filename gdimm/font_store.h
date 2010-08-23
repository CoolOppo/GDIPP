#pragma once

#include "font_man.h"
#include "helper_def.h"

using namespace std;

class gdimm_font_store
{
	/*
	font storage manages two kinds of font: registered font and linked font
	
	registered fonts are created outside gdipp
	they are considered temporary, not managed by the font manager
	registered fonts have non-negative font id

	linked fonts are created by gdipp for font linking
	every linked font are kept alive until the font storage is destructed
	linked fonts have negative font id
	*/

	// face name -> font id
	// we use this map because FreeType callback only have face id
	map<wstring, long> _reg_name_to_id;
	map<wstring, long> _linked_name_to_id;
	// font id -> font info
	// we use this map because vector internally free and re-allocate existing entries
	// pointers become invalid
	map<long, font_info> _id_to_info;

	DWORD _font_man_tls_index;

	static DWORD get_font_size(HDC font_holder, DWORD &table_header);
	static ULONG get_ttc_face_index(HDC font_holder, DWORD ttc_file_size);

public:
	gdimm_font_store();
	~gdimm_font_store();

	font_info *lookup_font(long font_id);
	long register_font(HDC font_holder, const wchar_t *font_face);
	long link_font(HDC font_holder, HFONT linked_hfont, wstring &linked_font_face);
	BOOL register_thread_font_man(gdimm_font_man *font_man);
	const gdimm_font_man *lookup_thread_font_man();
};