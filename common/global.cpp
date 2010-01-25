#include "stdafx.h"
#include "global.h"

BOOL is_font_true_type(HDC hdc)
{
	TEXTMETRIC metrics;
	GetTextMetrics(hdc, &metrics);
	return metrics.tmPitchAndFamily & TMPF_TRUETYPE;
}

void debug_output(const TCHAR *str)
{
	FILE *f = fopen("C:\\ft_debug.txt", "a+");
	fwprintf(f, TEXT("%s\n"), str);
	fclose(f);
}

void debug_output(const TCHAR *str, unsigned int c)
{
	FILE *f = fopen("C:\\ft_debug.txt", "a+");

	for (size_t i = 0; i < c; i++)
		fwprintf(f, TEXT("%c"), str[i] + 0x1D);
	fwprintf(f, TEXT("\n"));

	fclose(f);
}

void debug_output(DWORD num)
{
	FILE *f = fopen("C:\\ft_debug.txt", "a+");
	fwprintf(f, TEXT("%u\n"), num);
	fclose(f);
}