#include "stdafx.h"
#include "global.h"

font_table loaded_fonts;

bool operator<(const font_mapping &mapping1, const font_mapping &mapping2)
{
	return (memcmp(&mapping1, &mapping2, sizeof(font_mapping)) < 0);
}

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

void debug_output(DWORD num)
{
	FILE *f = fopen("C:\\ft_debug.txt", "a+");
	fwprintf(f, TEXT("%u\n"), num);
	fclose(f);
}