#pragma once

#include "font_info.h"

class gdimm_Text
{
private:
	FontInfo curr_font;
	COLORREF fg_color;
	COLORREF bg_color;

	void GetTextInfo(HDC hdc);
	void DrawBitmapMono(HDC hdc, FT_Bitmap bitmap, FT_Vector pos);
	void DrawBitmap256(HDC hdc, FT_Bitmap bitmap, FT_Vector pos);
	void DrawBitmapLCD(HDC hdc, FT_Bitmap bitmap, FT_Vector pos);
	void DrawBitmap(HDC hdc, FT_Bitmap bitmap, FT_Vector pos);

public:
	FT_Vector cursor;
	const RECT *clip_rect;

	gdimm_Text();
	~gdimm_Text();
	BOOL TextOut(HDC hdc, CONST INT * lpDx, const TCHAR *text, unsigned int count);
};