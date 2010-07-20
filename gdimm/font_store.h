#pragma once

#include "os2_metrics.h"

using namespace std;

class gdimm_font_store
{
	friend class gdimm_font_man;

	/*
	font storage stores two kinds of fonts: registered fonts and linked fonts

	registered fonts are created outside the font manager
	they are considered temporary, not managed by font manager
	registered fonts have non-negative font id
	the font information are shared by all threads, however the font holder is stored in TLS

	linked fonts are created by font manager, for font linking
	every linked font are kept alive until the font manager is destructed
	linked fonts have negative font id
	the handle and information of linked fonts are shared by all threads, however the font holder is stored in TLS
	*/

	struct font_info
	{
		// handle of the linked font for destruction
		// NULL for registered fonts
		HFONT linked_hfont;

		// the buffer of the outline metrics of the linked font
		// empty for registered fonts
		vector<BYTE> metric_buf;

		// used to retrieve font data from GetFontData
		DWORD table_header;
		DWORD face_index;

		FT_StreamRec stream;
		gdimm_os2_metrics os2_metrics;
	};

	// face name -> font id
	// we use this map because FreeType callback only have face id
	map<wstring, long> _reg_name_to_id;
	map<wstring, long> _linked_name_to_id;
	// font id -> font info
	// we use this map because vector internally free and re-allocate existing entries
	// pointers become invalid
	map<long, font_info> _id_to_info;

public:
	~gdimm_font_store();
};

