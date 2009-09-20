#pragma once

#include <map>
using namespace std;

class FT_Engine
{
private:
	struct FontData
	{
		BYTE *lpvBuffer;
		DWORD cbData;
	};

	typedef map<LOGFONT, FontData> FontDataCache;

	FT_Library lib;
	unsigned int px_width;
	unsigned int px_height;
	COLORREF fg_color;
	COLORREF bg_color;
	LOGFONT curr_font;
	FontDataCache font_cache;

	void GetTextInfo(HDC hdc);
	void CacheFont(HDC hdc);
	void DrawBitmapMono(HDC hdc, FT_Bitmap bitmap, FT_Vector pos);
	void DrawBitmap256(HDC hdc, FT_Bitmap bitmap, FT_Vector pos);
	void DrawBitmapLCD(HDC hdc, FT_Bitmap bitmap, FT_Vector pos);
	void DrawBitmap(HDC hdc, FT_Bitmap bitmap, FT_Vector pos);

public:
	FT_Vector cursor;
	bool opaque;

	FT_Engine();
	~FT_Engine();
	BOOL TextOut(HDC hdc, const TCHAR *text, unsigned int count);
};