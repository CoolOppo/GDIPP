#include "stdafx.h"
#include "ft.h"

extern FT_Engine ft_eng;

void Test(HDC hdc)
{
	ft_eng.opaque = false;

	const TCHAR testString[] = TEXT("The quick brown fox jumps over the lazy dog. 1234567890");
	const unsigned int stringLen = lstrlen(testString);

	HFONT f;

	ft_eng.cursor.x = 0; ft_eng.cursor.y = 0;
	f = CreateFont(-8, 0, 0, 0, 400, 0, 0, 0, 1, 0, 0, 0, 0, TEXT("Arial"));
	SelectObject(hdc, f);
	SetTextColor(hdc, RGB(0, 0, 0));
	SetBkColor(hdc, RGB(255, 255, 255));
	ft_eng.TextOut(hdc, testString, stringLen);
	DeleteObject(f);

	ft_eng.cursor.x = 25; ft_eng.cursor.y = 50;
	f = CreateFont(-11, 0, 0, 0, 400, 0, 0, 0, 1, 0, 0, 0, 0, TEXT("Courier New"));
	SelectObject(hdc, f);
	ft_eng.TextOut(hdc, testString, stringLen);
	DeleteObject(f);

	ft_eng.cursor.x = 50; ft_eng.cursor.y = 100;
	f = CreateFont(-13, 0, 0, 0, 400, 0, 0, 0, 1, 0, 0, 0, 0, TEXT("Lucida Sans Unicode"));
	SelectObject(hdc, f);
	ft_eng.TextOut(hdc, testString, stringLen);
	DeleteObject(f);

	ft_eng.cursor.x = 75; ft_eng.cursor.y = 150;
	SetTextColor(hdc, RGB(128, 0, 128));
	f = CreateFont(-16, 0, 0, 0, 400, 0, 0, 0, 1, 0, 0, 0, 0, TEXT("Tahoma"));
	SelectObject(hdc, f);
	ft_eng.TextOut(hdc, testString, stringLen);
	DeleteObject(f);

	ft_eng.cursor.x = 100; ft_eng.cursor.y = 200;
	SetTextColor(hdc, RGB(32, 128, 64));
	f = CreateFont(-24, 0, 0, 0, 400, 0, 0, 0, 1, 0, 0, 0, 0, TEXT("Times New Roman"));
	SelectObject(hdc, f);
	ft_eng.TextOut(hdc, testString, stringLen);
	DeleteObject(f);

	ft_eng.cursor.x = 125; ft_eng.cursor.y = 275;
	SetTextColor(hdc, RGB(33, 66, 99));
	f = CreateFont(-32, 0, 0, 0, 400, 0, 0, 0, 1, 0, 0, 0, 0, TEXT("Verdana"));
	SelectObject(hdc, f);
	ft_eng.TextOut(hdc, TEXT("AV"), 2);
	DeleteObject(f);
}