#include "stdafx.h"
#include "font_store.h"
#include "helper_func.h"
#include "lock.h"

gdimm_font_store::gdimm_font_store()
{
	_font_man_tls_index = TlsAlloc();
	assert(_font_man_tls_index != TLS_OUT_OF_INDEXES);
}

gdimm_font_store::~gdimm_font_store()
{
	BOOL b_ret;

	for (map<long, font_info>::const_iterator iter = _id_to_info.begin(); iter != _id_to_info.upper_bound(-1); iter++)
	{
		// unregister linked fonts
		//DeleteObject(iter->second.linked_hfont);
	}

	b_ret = TlsFree(_font_man_tls_index);
	assert(b_ret);
}

font_info *gdimm_font_store::lookup_font(long font_id)
{
	map<long, font_info>::iterator iter = _id_to_info.find(font_id);
	if (iter == _id_to_info.end())
		return NULL;

	return &iter->second;
}

long gdimm_font_store::register_font(HDC font_holder, const wchar_t *font_face)
{

}

BOOL gdimm_font_store::register_thread_font_man(gdimm_font_man *font_man)
{
}

const gdimm_font_man *gdimm_font_store::lookup_thread_font_man()
{
	return static_cast<const gdimm_font_man *>(TlsGetValue(_font_man_tls_index));
}
