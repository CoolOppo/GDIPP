#pragma once

struct FontTrait
{
	LONG  lfWeight;
	BYTE  lfItalic;
	TCHAR lfFaceName[LF_FACESIZE];
};

class FontInfo
{
private:
	static uint32_t SuperFastHash (const char * data, int len);

public:
	LOGFONT font_attr;

	FontTrait GetTrait();
	uint32_t GetFaceID(FontTrait *use_trait = NULL);
};