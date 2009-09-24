#pragma once

#include <map>
using namespace std;

class gdimm_Text
{
private:
	FT_Library lib;
	COLORREF fg_color;
	COLORREF bg_color;
	unsigned int px_width;
	unsigned int px_height;
	
	FTC_Manager cache_man;
	FTC_CMapCache cmap_cache;
	FTC_ImageCache glyph_cache;

	void GetTextInfo(HDC hdc);
	void DrawBitmapMono(HDC hdc, FT_Bitmap bitmap, FT_Vector pos);
	void DrawBitmap256(HDC hdc, FT_Bitmap bitmap, FT_Vector pos);
	void DrawBitmapLCD(HDC hdc, FT_Bitmap bitmap, FT_Vector pos);
	void DrawBitmap(HDC hdc, FT_Bitmap bitmap, FT_Vector pos);

public:
	LOGFONT curr_font;
	FT_Vector cursor;
	const RECT *clip_rect;

	gdimm_Text();
	~gdimm_Text();
	BOOL TextOut(HDC hdc, CONST INT * lpDx, const TCHAR *text, unsigned int count);
};