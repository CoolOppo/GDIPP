#include "stdafx.h"
#include "font_store.h"

gdimm_font_store::~gdimm_font_store(void)
{
	for (map<long, font_info>::const_iterator iter = _id_to_info.begin(); iter != _id_to_info.upper_bound(-1); iter++)
	{
		// delete linked fonts
		DeleteObject(iter->second.linked_hfont);
	}
}