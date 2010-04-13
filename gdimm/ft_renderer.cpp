#include "stdafx.h"
#include "ft_renderer.h"
#include "ft.h"
#include "gdimm.h"

FT_BitmapGlyphRec empty_glyph = {};

ft_renderer::ft_renderer(gdimm_text *text): gdimm_renderer(text)
{
	_ft_scaler.pixel = 1;
	_ft_scaler.x_res = 0;
	_ft_scaler.y_res = 0;
	_using_cache_node = NULL;
}

ft_renderer::~ft_renderer()
{
	// the cache node associated with this renderer is not used any more
	// the glyph cache can reclaim it safely
	if (_using_cache_node != NULL)
		*_using_cache_node = false;
}

FT_ULong ft_renderer::get_load_flags(FT_Render_Mode render_mode, const WCHAR *font_name)
{
	const font_setting_cache *setting_cache = setting_cache_instance.lookup(font_name);

	FT_ULong load_flags = FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH |
		(setting_cache->embedded_bitmap ? 0 : FT_LOAD_NO_BITMAP);

	if (setting_cache->hinting)
	{
		load_flags |= (setting_cache->auto_hinting ? FT_LOAD_FORCE_AUTOHINT : FT_LOAD_DEFAULT);

		switch (render_mode)
		{
		case FT_RENDER_MODE_NORMAL:
			load_flags |= (setting_cache->light_mode ? FT_LOAD_TARGET_LIGHT : FT_LOAD_TARGET_NORMAL);
		case FT_RENDER_MODE_MONO:
			load_flags |= FT_LOAD_TARGET_MONO;
		case FT_RENDER_MODE_LCD:
			load_flags |= (setting_cache->light_mode ? FT_LOAD_TARGET_LIGHT : FT_LOAD_TARGET_LCD);
		}
	}
	else
		load_flags |= FT_LOAD_NO_HINTING;

	return load_flags;
}

void ft_renderer::oblique_outline(const FT_Outline *outline, double angle)
{
	// the advancement of slant on X-axis direction
	const double slant_adv = tan(pi * angle / 180);

	FT_Matrix oblique_mat = {to_16dot16(1), to_16dot16(slant_adv), 0, to_16dot16(1)};
	FT_Outline_Transform(outline, &oblique_mat);
}

void ft_renderer::update_embolden(const TT_OS2 &os2_table, const WCHAR *font_name)
{
	const font_setting_cache *setting_cache = setting_cache_instance.lookup(font_name);
	_embolden = setting_cache->embolden;

	// if non-regular weight is demanded, and the font has no native weighted glyphs, do embolden
	// the embolden weight is based on the difference between demanded weight and the regular weight
	if (_text->_font_attr.lfWeight != FW_DONTCARE)
	{
		const static LONG weight_class_max[] = {550, 611};
		const static double weight_embolden[] = {0, 0.5};
		const static double max_embolden = 1;

		double text_embolden = max_embolden, font_embolden = max_embolden;

		for (int i = 0; i < sizeof(weight_class_max) / sizeof(LONG); i++)
		{
			if (_text->_font_attr.lfWeight <= weight_class_max[i])
			{
				text_embolden = weight_embolden[i];
				break;
			}
		}

		for (int i = 0; i < sizeof(weight_class_max) / sizeof(LONG); i++)
		{
			if (os2_table.usWeightClass <= weight_class_max[i])
			{
				font_embolden = weight_embolden[i];
				break;
			}
		}

		_embolden += text_embolden - font_embolden;
	}
}

const FT_BitmapGlyph ft_renderer::render_glyph(WORD glyph_index, const WCHAR *font_face)
{
	FT_Error ft_error;

	// lookup if there is already a cached glyph
	const gdimm_glyph_cache::cache_trait trait = {_ft_scaler.face_id, _ft_scaler.width, _ft_scaler.height, _render_mode, _load_flags};
	FT_BitmapGlyph bmp_glyph = glyph_cache_instance.lookup(trait, glyph_index, _using_cache_node);
	if (bmp_glyph != NULL)
	{
		// this renderer is using the cache node
		// the glyph cache should not reclaim it until the rendering is finished
		*_using_cache_node = true;
		return bmp_glyph;
	}

	// no cached glyph, lookup outline and rasterize

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

	// it seems faster if oblique first, and then embolden
	// if italic style is demanded, and the font is not natively italic, do oblique transformation
	const bool need_oblique = (_text->_font_attr.lfItalic && _text->_outline_metrics->otmItalicAngle == 0);
	const bool need_embolden = (_embolden != 0.0);
	const bool need_glyph_copy = (need_oblique || need_embolden);

	if (need_glyph_copy)
	{
		FT_Glyph_Copy(cached_glyph, &glyph);
		FT_Outline *glyph_outline = &((FT_OutlineGlyph) glyph)->outline;

		if (need_oblique)
			oblique_outline(glyph_outline, 20);

		if (need_embolden)
		{
			ft_error = FT_Outline_Embolden(glyph_outline, to_26dot6(_embolden));
			assert(ft_error == 0);
		}
	}
	else
		glyph = cached_glyph;

	// glyph outline -> glyph bitmap conversion
	// FT_Glyph_To_Bitmap is not thread-safe
	{
		critical_section interlock(CS_TEXT);
		ft_error = FT_Glyph_To_Bitmap(&glyph, _render_mode, NULL, need_glyph_copy);
		assert(ft_error == 0);
	}

	// add glyph into cache
	bmp_glyph = (const FT_BitmapGlyph) glyph;
	glyph_cache_instance.add(trait, glyph_index, bmp_glyph, _using_cache_node);
	*_using_cache_node = true;

	return bmp_glyph;
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
	const WCHAR *dc_font_family = metric_family_name(_text->_outline_metrics);
	wstring curr_font_face = _text->_font_face;
	const WCHAR *curr_font_family = dc_font_family;

	// Windows renders monochrome bitmap better than FreeType
	if (render_mode == FT_RENDER_MODE_MONO && !_text->_setting_cache->render_mono)
		return false;

	_render_mode = render_mode;
	_load_flags = get_load_flags(render_mode, _text->_font_face);

	/*
	glyph metrics setup

	while the height in FreeType scaler has the same meaning as the height value in LOGFONT structure, the widths are different
	what we know is, when the width in LOGFONT is the xAvgCharWidth defined in the TrueType OS/2 table,
	the corresponding FreeType scaler width is the height
	therefore we need conversion when LOGFONT width is not 0
	simple calculation yields new_freetype_width = logfont_width * em_square / xAvgCharWidth
	note that the tmAveCharWidth field in TEXTMETRIC is the actual LOGFONT width, never be 0
	*/

	_ft_scaler.face_id = (FTC_FaceID) font_man_instance.register_font(_text->_hdc_text, _text->_font_face);
	_ft_scaler.height = _text->_outline_metrics->otmTextMetrics.tmHeight - _text->_outline_metrics->otmTextMetrics.tmInternalLeading;
	TT_OS2 os2_table = font_man_instance.get_os2_table(_ft_scaler.face_id);
	update_embolden(os2_table, _text->_font_face);

	if (_text->_font_attr.lfWidth == 0)
		_ft_scaler.width = _ft_scaler.height;
	else
	{
		// compare the xAvgCharWidth against the current average char width
		_ft_scaler.width = MulDiv(_text->_outline_metrics->otmTextMetrics.tmAveCharWidth, _text->_outline_metrics->otmEMSquare, os2_table.xAvgCharWidth);
	}

	if (options & ETO_GLYPH_INDEX)
	{
		// directly render glyph indices with the current DC font

		_glyphs.resize(c);
		_glyph_pos.resize(c);

		for (UINT i = 0; i < c; i++)
		{
			_glyphs[i] = render_glyph(lpString[i], _text->_font_face);
			assert(_glyphs[i] != NULL);
		}
	}
	else
	{
		UINT rendered_count = 0;
		int font_link_index = 0;
		bool glyph_all_empty = true;
		wstring final_string = lpString;
		wstring glyph_indices;

		_glyphs.resize(c);
		_glyph_pos.resize(c);
		glyph_indices.resize(c);

		while (true)
		{
			font_man_instance.get_glyph_indices((long) _ft_scaler.face_id, &final_string[0], c, &glyph_indices[0]);

			for (UINT i = 0; i < c; i++)
			{
				if (glyph_indices[i] == 0xffff || final_string[i] == 0)
					continue;

				if (iswcntrl(final_string[i]))
					_glyphs[i] = (const FT_BitmapGlyph) &empty_glyph;
				else
				{
					_glyphs[i] = render_glyph(glyph_indices[i], curr_font_face.c_str());
					assert(_glyphs[i] != NULL);

					if (_glyphs[i]->bitmap.buffer != NULL)
						glyph_all_empty = false;
				}

				final_string[i] = 0;
				rendered_count += 1;
			}

			if (rendered_count == c)
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

			os2_table = font_man_instance.get_os2_table(_ft_scaler.face_id);
			update_embolden(os2_table, curr_font_face.c_str());
		}

		if (glyph_all_empty)
			return false;
	}

	update_glyph_pos(options, lpDx);

	return true;
}