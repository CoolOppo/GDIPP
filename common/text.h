#pragma once

#include <ObjBase.h>
#include <mlang.h>

class gdimm_Text
{
private:
	COLORREF fg_color;
	COLORREF bg_color;
	IMLangFontLink2 *mlang_fl;

	void GetCurrentColors(HDC hdc);
	void DrawBitmapMono(HDC hdc, FT_Bitmap bitmap, FT_Vector pos) const;
	void DrawBitmap256(HDC hdc, FT_Bitmap bitmap, FT_Vector pos) const;
	void DrawBitmapLCD(HDC hdc, FT_Bitmap bitmap, FT_Vector pos) const;
	void DrawBitmap(HDC hdc, FT_Bitmap bitmap, FT_Vector pos) const;
	BOOL StringOut(HDC hdc, const TCHAR *str, unsigned int count);

public:
	FT_Vector cursor;
	CONST RECT *clip_rect;
	CONST INT * distances;

	gdimm_Text();
	~gdimm_Text();
	BOOL TextOut(HDC hdc, const TCHAR *text, unsigned int count);
};