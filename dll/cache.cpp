#include "stdafx.h"
#include "cache.h"
#include "text.h"

#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

uint32_t SuperFastHash (const char * data, int len) {
uint32_t hash = len, tmp;
int rem;

    if (len <= 0 || data == NULL) return 0;

    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for (;len > 0; len--) {
        hash  += get16bits (data);
        tmp    = (get16bits (data+2) << 11) ^ hash;
        hash   = (hash << 16) ^ tmp;
        data  += 2*sizeof (uint16_t);
        hash  += hash >> 11;
    }

    /* Handle end cases */
    switch (rem) {
        case 3: hash += get16bits (data);
                hash ^= hash << 16;
                hash ^= data[sizeof (uint16_t)] << 18;
                hash += hash >> 11;
                break;
        case 2: hash += get16bits (data);
                hash ^= hash << 11;
                hash += hash >> 17;
                break;
        case 1: hash += *data;
                hash ^= hash << 10;
                hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}

FT_Error Face_Requester(FTC_FaceID face_id, FT_Library library, FT_Pointer request_data, FT_Face *aface)
{
	gdimm_Text *gdimm = (gdimm_Text*) request_data;

	HDC hdc_mem = CreateCompatibleDC(NULL);
	assert(hdc_mem != NULL);
	HFONT hfont = CreateFont(-11, 0, 0, 0,
		gdimm->curr_font.lfWeight,
		gdimm->curr_font.lfItalic,
		FALSE,
		FALSE,
		ANSI_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH,
		gdimm->curr_font.lfFaceName);
	assert(hfont != NULL);
	SelectObject(hdc_mem, hfont);

	DWORD needed = GetFontData(hdc_mem, 0, 0, NULL, 0);
	assert(needed != GDI_ERROR);
	BYTE *font_data = new BYTE[needed];
	needed = GetFontData(hdc_mem, 0, 0, font_data, needed);
	assert(needed != GDI_ERROR);

	DeleteObject(hfont);
	DeleteDC(hdc_mem);

	FT_Error error = FT_New_Memory_Face(library, font_data, needed, 0, aface);
	assert(error == 0);

	return 0;
}