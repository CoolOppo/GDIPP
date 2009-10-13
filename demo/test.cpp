#include "stdafx.h"
#include <ObjBase.h>
#include <mlang.h>

void TestDIB(HDC hdc)
{
	unsigned char bits[] = {0x80, 0x80, 0x80, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00};
	unsigned char mask[] = {0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00};
	
	BITMAPINFO bmi;
	memset(&bmi, 0, sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = 3;
	bmi.bmiHeader.biHeight = -4;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 8;
	bmi.bmiHeader.biCompression = BI_RGB;
	
	SetDIBitsToDevice(hdc, 10, 10, 3, 4, 0, 0, 0, 4, mask, &bmi, DIB_RGB_COLORS);
	/*HBITMAP hbmp = CreateDIBitmap(hdc, &bmi.bmiHeader, CBM_INIT, bits, &bmi, DIB_RGB_COLORS);
	HBITMAP hmask = CreateDIBitmap(hdc, &bmi.bmiHeader, CBM_INIT, mask, &bmi, DIB_RGB_COLORS);
	HDC hdc_mem = CreateCompatibleDC(hdc);
	SelectObject(hdc_mem, hmask);
	BitBlt(hdc, 10, 10, 3, 4, hdc_mem, 0, 0, SRCAND);
	SelectObject(hdc_mem, hbmp);
	BitBlt(hdc, 10, 10, 3, 3, hdc_mem, 0, 0, SRCPAINT);*/
}

void TestDDB(HDC hdc)
{
	int w = 8;
	int h = 8;

	unsigned char bits[] = {126, 0, 96, 0, 96, 0, 126, 0, 96, 0, 96, 0, 96, 0, 96, 0};

	HDC hdc_mem = CreateCompatibleDC(hdc);
	HBITMAP hbmp = CreateBitmap(w, h, 1, 1, bits);
	SelectObject(hdc_mem, hbmp);

	BOOL a = BitBlt(hdc, 10, 10, w, h, hdc_mem, 0, 0, SRCCOPY);

	DeleteObject(hbmp);
	DeleteDC(hdc_mem);
}

void TestFontLink(HDC hdc)
{
	TCHAR *text = TEXT("HelloÄãºÃ");
	HFONT tahoma = CreateFont(-15, 0, 0, 0, 400, 0, 0, 0, 0, 0, 0, 0, 0, TEXT("Tahoma"));
	SelectObject(hdc, tahoma);

	CoInitialize(NULL);

	IMLangFontLink2 *fl;
	CoCreateInstance(CLSID_CMultiLanguage, NULL, CLSCTX_ALL, IID_IMLangFontLink2, (LPVOID*)&fl);

	DWORD font_cp;
	HFONT curr_font = (HFONT) GetCurrentObject(hdc, OBJ_FONT);
	fl->GetFontCodePages(hdc, curr_font, &font_cp);

	int str_start = 0;
	int str_len = lstrlen(text);

	while (str_len > 0)
	{
		DWORD str_cp;
		long processed;
		fl->GetStrCodePages(text + str_start, str_len, font_cp, &str_cp, &processed);

		HFONT new_font;
		fl->MapFont(hdc, str_cp, 0, &new_font);

		LOGFONT lf;
		GetObject(new_font, sizeof(LOGFONT), &lf);

		HFONT old_font = (HFONT) SelectObject(hdc, new_font);
		ExtTextOut(hdc, 10 + str_start * 11, 10, 0, NULL, text + str_start, processed, NULL);
		SelectObject(hdc, old_font);
		fl->ReleaseFont(new_font);

		str_start += processed;
		str_len -= processed;
	}
	
	fl->Release();
	CoUninitialize();
	DeleteObject(tahoma);
}

void Test(HDC hdc)
{
	//TestFontLink(hdc);
}