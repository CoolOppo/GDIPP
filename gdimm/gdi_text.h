#pragma once

#include "text.h"
using namespace std;

class gdimm_gdi_text : public gdimm_text
{
	friend class gdimm_renderer;
	friend class ggo_renderer;
	friend class ft_renderer;

	// device context attributes
	POINT _cursor;
	RGBQUAD _text_rgb;
	int _char_extra;

	vector<const FT_BitmapGlyph> _glyphs;
	vector<POINT> _glyph_pos;

	// gamma ramps for gray, red, green, blue
	const BYTE *_gamma_ramps[4];

	POINT get_adjusted_origin(const POINT &origin, int width) const;

	void set_bmp_bits_mono(const FT_Bitmap &src_bitmap,
		const POINT &src_pos,
		BYTE *dest_bits,
		const POINT &dest_pos,
		const SIZE &dest_size) const;
	void set_bmp_bits_gray(const FT_Bitmap &src_bitmap,
		const POINT &src_pos,
		BYTE *dest_bits,
		const POINT &dest_pos,
		const SIZE &dest_size,
		WORD bmp_alpha) const;
	void set_bmp_bits_lcd(const FT_Bitmap &src_bitmap,
		const POINT &src_pos,
		BYTE *dest_bits,
		const POINT &dest_pos,
		const SIZE &dest_size,
		WORD bmp_alpha) const;
	bool draw_glyphs(UINT options,
		CONST RECT *lprect) const;

	virtual bool render(UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx, FT_Render_Mode render_mode) = 0;

public:
	virtual bool init(HDC hdc);
	bool text_out(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx);
};