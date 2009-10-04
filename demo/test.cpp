#include "stdafx.h"

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

void Test(HDC hdc)
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