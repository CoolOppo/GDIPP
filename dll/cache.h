#pragma once

typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

struct Font_Info
{
    LONG  lfWeight;
    BYTE  lfItalic;
	TCHAR lfFaceName[LF_FACESIZE];

	Font_Info(LONG weight, BYTE italic, TCHAR *face_name) : lfWeight(weight), lfItalic(italic)
	{
		lstrcpyn(lfFaceName, face_name, LF_FACESIZE);
	}
};

uint32_t SuperFastHash (const char * data, int len);
FT_Error Face_Requester(FTC_FaceID face_id, FT_Library library, FT_Pointer request_data, FT_Face *aface);