#include "stdafx.h"
#include "ft_renderer.h"
#include "ft.h"
#include "gdimm.h"

void ft_renderer::oblique_outline(const FT_Outline *outline, double angle)
{
	// the advancement of slant on X-axis direction
	const double slant_adv = tan(pi * angle / 180);

	FT_Matrix oblique_mat = {to_16dot16(1), to_16dot16(slant_adv), 0, to_16dot16(1)};
	FT_Outline_Transform(outline, &oblique_mat);
}

FT_ULong ft_renderer::get_load_flags(FT_Render_Mode render_mode, const WCHAR *font_name)
{
	bool auto_hinting = false;
	setting_cache_instance.lookup("auto_hinting", font_name, auto_hinting);
	bool embedded_bitmap = false;
	setting_cache_instance.lookup("embedded_bitmap", font_name, embedded_bitmap);
	bool hinting = true;
	setting_cache_instance.lookup("hinting", font_name, hinting);
	bool light_mode = true;
	setting_cache_instance.lookup("light_mode", font_name, light_mode);

	FT_ULong load_flags = FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH |
		(embedded_bitmap ? 0 : FT_LOAD_NO_BITMAP);

	if (hinting)
	{
		load_flags |= (auto_hinting ? FT_LOAD_FORCE_AUTOHINT : FT_LOAD_DEFAULT);

		switch (render_mode)
		{
		case FT_RENDER_MODE_NORMAL:
			load_flags |= (light_mode ? FT_LOAD_TARGET_LIGHT : FT_LOAD_TARGET_NORMAL);
		case FT_RENDER_MODE_MONO:
			load_flags |= FT_LOAD_TARGET_MONO;
		case FT_RENDER_MODE_LCD:
			load_flags |= (light_mode ? FT_LOAD_TARGET_LIGHT : FT_LOAD_TARGET_LCD);
		}
	}
	else
		load_flags |= FT_LOAD_NO_HINTING;

	return load_flags;
}

ft_renderer::ft_renderer(gdimm_text *text): gdimm_renderer(text)
{
	_ft_scaler.pixel = 1;
	_ft_scaler.x_res = 0;
	_ft_scaler.y_res = 0;
}

FT_BitmapGlyph ft_renderer::render_glyph(WORD glyph_index, const WCHAR *font_face)
{
	FT_Error ft_error;

	FT_Glyph glyph, cached_glyph;

	// FTC_ImageCache_LookupScaler is not thread-safe
	{
		critical_section interlock(CS_TEXT);
		ft_error = FTC_ImageCache_LookupScaler(ft_glyph_cache, &_ft_scaler, _load_flags, glyph_index, &cached_glyph, NULL);
		if (ft_error != 0)
			return NULL;
	}

	// some fonts are embedded with pre-rendered glyph bitmap
	// in that case, use original ExtTextOutW
	if (cached_glyph->format != FT_GLYPH_FORMAT_OUTLINE)
		return NULL;

	FT_Glyph_Copy(cached_glyph, &glyph);
	FT_Outline *glyph_outline = &((FT_OutlineGlyph) glyph)->outline;

	// if italic style is requested, and the font has no native italic glyphs,
	// do oblique transformation
	if (_text->_font_attr.lfItalic && _text->_outline_metrics->otmItalicAngle == 0)
		oblique_outline(glyph_outline, 20);

	// glyph outline -> glyph bitmap conversion
	double embolden = 0.0;
	setting_cache_instance.lookup("embolden", font_face, embolden);
	if (embolden != 0.0)
	{
		ft_error = FT_Outline_Embolden(glyph_outline, to_26dot6(embolden));
		assert(ft_error == 0);
	}

	// FT_Glyph_To_Bitmap is not thread-safe
	{
		critical_section interlock(CS_TEXT);
		ft_error = FT_Glyph_To_Bitmap(&glyph, _render_mode, NULL, TRUE);
		assert(ft_error == 0);
	}

	return (FT_BitmapGlyph) glyph;
}

void ft_renderer::update_glyph_pos(UINT options, CONST INT *lpDx)
{
	// is ETO_PDY is set, lpDx contains both x increment and y displacement
	const int dx_factor = ((options & ETO_PDY) ? 2 : 1);

	for (size_t i = 0; i < _glyphs.size(); i++)
	{
		_glyph_pos[i] = _text->_cursor;
		_glyph_pos[i].x += _glyphs[i]->left;

		POINT glyph_advance = {0, from_16dot16(_glyphs[i]->root.advance.y)};
		const LONG char_advance = from_16dot16(_glyphs[i]->root.advance.x) + _char_extra;

		if (lpDx == NULL)
			glyph_advance.x = char_advance;
		else
		{
			glyph_advance.x = lpDx[i * dx_factor];

			// the last element in lpDx may be 0
			// in that case, we use the character advancement
			if (glyph_advance.x == 0 && i == _glyphs.size() - 1)
				glyph_advance.x = char_advance;
		}

		_text->_cursor.x += glyph_advance.x;
		_text->_cursor.y += glyph_advance.y;
	}
}

bool ft_renderer::render(UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx, FT_Render_Mode render_mode)
{
	FT_Error ft_error;

	const WCHAR *dc_font_face = metric_face_name(_text->_outline_metrics);
	const WCHAR *dc_font_family = metric_family_name(_text->_outline_metrics);
	wstring curr_font_face = dc_font_face;
	const WCHAR *curr_font_family = dc_font_family;

	// Windows renders monochrome bitmap better than FreeType
	bool render_mono = false;
	setting_cache_instance.lookup("render_mono", dc_font_face, render_mono);
	if (render_mode == FT_RENDER_MODE_MONO && !render_mono)
		return false;

	_render_mode = render_mode;
	_load_flags = get_load_flags(render_mode, dc_font_face);

	/*
	glyph metrics setup

	while the height in FreeType scaler has the same meaning as the height value in LOGFONT structure, the widths are different
	what we know is, when the width in LOGFONT is the xAvgCharWidth defined in the TrueType OS/2 table,
	the corresponding FreeType scaler width is the height
	therefore we need conversion when LOGFONT width is not 0
	simple calculation yields new_freetype_width = logfont_width * em_square / xAvgCharWidth
	note that the tmAveCharWidth field in TEXTMETRIC is the actual LOGFONT width, never be 0
	*/

	_ft_scaler.face_id = (FTC_FaceID) font_man_instance.register_font(_text->_hdc_text, dc_font_face);
	_ft_scaler.height = _text->_outline_metrics->otmTextMetrics.tmHeight - _text->_outline_metrics->otmTextMetrics.tmInternalLeading;

	if (_text->_font_attr.lfWidth == 0)
		_ft_scaler.width = _ft_scaler.height;
	else
	{
		// get the OS/2 table of the current font
		FT_Face ft_face;
		ft_error = FTC_Manager_LookupFace(ft_cache_man, _ft_scaler.face_id, &ft_face);
		assert(ft_error == 0);
		TT_OS2 *os2_table = (TT_OS2*) FT_Get_Sfnt_Table(ft_face, ft_sfnt_os2);

		// compare the xAvgCharWidth against the current average char width
		_ft_scaler.width = MulDiv(_text->_outline_metrics->otmTextMetrics.tmAveCharWidth, _text->_outline_metrics->otmEMSquare, os2_table->xAvgCharWidth);
	}

	if (options & ETO_GLYPH_INDEX)
	{
		// directly render glyph indices with the current DC font

		_glyphs.resize(c);
		for (UINT i = 0; i < c; i++)
		{
			_glyphs[i] = render_glyph(lpString[i], dc_font_face);
			assert(_glyphs[i] != NULL);
		}
	}
	else
	{
		UINT rendered_count = 0;
		int font_link_index = 0;
		wstring str;
		wstring glyph_indices;

		str.reserve(c);
		for (UINT i = 0; i < c; i++)
		{
			// filter out control characters
			if (!iswcntrl(lpString[i]))
				str.push_back(lpString[i]);
		}

		const size_t str_length = str.size();
		_glyphs.resize(str_length);
		glyph_indices.resize(str_length);

		while (true)
		{
			font_man_instance.get_glyph_indices((long) _ft_scaler.face_id, &str[0], str_length, &glyph_indices[0]);

			for (UINT i = 0; i < str_length; i++)
			{
				if (glyph_indices[i] == 0xffff || str[i] == 0)
					continue;

					_glyphs[i] = render_glyph(glyph_indices[i], curr_font_face.c_str());
					assert(_glyphs[i] != NULL);

				str[i] = 0;
				rendered_count += 1;
			}

			if (rendered_count == str_length)
				break;
			
			// font linking
			curr_font_family = font_link_instance.lookup(dc_font_family, font_link_index);
			font_link_index += 1;

			if (curr_font_family == NULL)
				return false;

			_ft_scaler.face_id = (FTC_FaceID) font_man_instance.lookup_font(_text->_font_attr, curr_font_family, curr_font_face);
			if (_ft_scaler.face_id == NULL)
				return false;

			if (!_text->get_render_mode(curr_font_face.c_str(), _render_mode))
				return false;

			_load_flags = get_load_flags(_render_mode, curr_font_face.c_str());
		}
	}

	_glyph_pos.resize(_glyphs.size());
	update_glyph_pos(options, lpDx);

	return true;
}