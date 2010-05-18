#include "stdafx.h"
#include "ft_text.h"
#include "text_helper.h"
#include "ft.h"
#include "gdimm.h"
#include "lock.h"

FT_BitmapGlyphRec empty_glyph = {};

gdimm_ft_text::gdimm_ft_text()
:
_cache_node_ptr(NULL)
{
	_ft_scaler.pixel = 1;
	_ft_scaler.x_res = 0;
	_ft_scaler.y_res = 0;
}

FT_ULong gdimm_ft_text::get_load_flags(FT_Render_Mode render_mode, const wchar_t *font_name)
{
	const font_setting_cache *setting_cache = setting_cache_instance.lookup(font_name);

	FT_ULong load_flags = FT_LOAD_CROP_BITMAP |
		(setting_cache->embedded_bitmap ? 0 : FT_LOAD_NO_BITMAP);

	if (setting_cache->hinting == 0)
		load_flags |= FT_LOAD_NO_HINTING;
	else
	{
		switch (setting_cache->hinting)
		{
		case 1:
			load_flags |= FT_LOAD_TARGET_LIGHT;
			break;
		case 3:
			load_flags |= FT_LOAD_TARGET_MONO;
			break;
		default:
			{
				if (render_mode == FT_RENDER_MODE_LCD)
					load_flags |= FT_LOAD_TARGET_LCD;
				else
					load_flags |= FT_LOAD_TARGET_NORMAL;
				break;
			}
		}

		switch (setting_cache->auto_hinting)
		{
		case 0:
			load_flags |= FT_LOAD_NO_AUTOHINT;
			break;
		case 2:
			load_flags |= FT_LOAD_FORCE_AUTOHINT;
			break;
		default:
			load_flags |= FT_LOAD_DEFAULT;
			break;
		}
	}

	return load_flags;
}

void gdimm_ft_text::oblique_outline(const FT_Outline *outline, double angle)
{
	// the advancement of slant on X-axis direction
	const double slant_adv = tan(pi * angle / 180);

	FT_Matrix oblique_mat = {to_16dot16(1), to_16dot16(slant_adv), 0, to_16dot16(1)};
	FT_Outline_Transform(outline, &oblique_mat);
}

void gdimm_ft_text::update_embolden(FT_UShort weight_class, const wchar_t *font_name)
{
	const font_setting_cache *setting_cache = setting_cache_instance.lookup(font_name);
	_embolden = setting_cache->embolden;

	// if non-regular weight is demanded, and the font has no native weighted glyphs, do embolden
	// the embolden weight is based on the difference between demanded weight and the regular weight
	if (_font_attr.lfWeight != FW_DONTCARE)
	{
		/*
		emulate GDI behavior:
		weight 1 - 550 are rendered as Regular
		551 - 611 are Semibold
		612 - infinity are Bold
		*/
		const static LONG weight_class_max[] = {550, 611};
		const static FT_F26Dot6 weight_embolden[] = {0, 16};
		const static FT_F26Dot6 max_embolden = 32;

		FT_F26Dot6 text_embolden = max_embolden;
		FT_F26Dot6 font_embolden = max_embolden;

		for (int i = 0; i < sizeof(weight_class_max) / sizeof(LONG); i++)
		{
			if (_font_attr.lfWeight <= weight_class_max[i])
			{
				text_embolden = weight_embolden[i];
				break;
			}
		}

		for (int i = 0; i < sizeof(weight_class_max) / sizeof(LONG); i++)
		{
			if (weight_class <= weight_class_max[i])
			{
				font_embolden = weight_embolden[i];
				break;
			}
		}

		_embolden += text_embolden - font_embolden;
	}
}

const FT_BitmapGlyph gdimm_ft_text::render_glyph(WORD glyph_index, const wchar_t *font_face)
{
	FT_Error ft_error;

	// if italic style is demanded, and the font has italic glyph, do oblique transformation
	const bool is_oblique = (_font_attr.lfItalic && !_is_italic);
	
	// each renderer instance add the reference count to the glyph cache at most once
	// only add reference count in the first time
	const bool need_add_ref = (_cache_node_ptr == NULL);

	// lookup if there is already a cached glyph
	const gdimm_glyph_cache::cache_trait trait = {_ft_scaler.face_id, _ft_scaler.width, _ft_scaler.height, _embolden, is_oblique, _render_mode, _load_flags};
	FT_BitmapGlyph bmp_glyph = glyph_cache_instance.lookup_glyph(trait, glyph_index, _cache_node_ptr);
	if (bmp_glyph != NULL)
	{
		// this renderer is using the cache node
		// the glyph cache should not reclaim it until the rendering is finished
		if (need_add_ref)
			gdimm_glyph_cache::add_ref(_cache_node_ptr);

		return bmp_glyph;
	}

	// no cached glyph, lookup outline and rasterize

	FT_Glyph glyph, cached_glyph;

	// this FreeType function seems not thread-safe
	{
		gdimm_lock lock(LOCK_TEXT);
		ft_error = FTC_ImageCache_LookupScaler(ft_glyph_cache, &_ft_scaler, _load_flags, glyph_index, &cached_glyph, NULL);
		if (ft_error != 0)
			return NULL;
	}

	// some fonts are embedded with pre-rendered glyph bitmap
	// in that case, use original ExtTextOutW
	if (cached_glyph->format != FT_GLYPH_FORMAT_OUTLINE)
		return NULL;

	const bool need_embolden = (_embolden != 0);
	const bool need_glyph_copy = (is_oblique || need_embolden);

	if (need_glyph_copy)
	{
		FT_Glyph_Copy(cached_glyph, &glyph);
		FT_Outline *glyph_outline = &((FT_OutlineGlyph) glyph)->outline;

		// it seems faster if oblique first, and then embolden
		if (is_oblique)
			oblique_outline(glyph_outline, 10);

		if (need_embolden)
		{
			ft_error = FT_Outline_Embolden(glyph_outline, _embolden);
			assert(ft_error == 0);
		}
	}
	else
		glyph = cached_glyph;

	// glyph outline -> glyph bitmap conversion
	// this FreeType function seems not thread-safe
	{
		gdimm_lock lock(LOCK_TEXT);
		ft_error = FT_Glyph_To_Bitmap(&glyph, _render_mode, NULL, need_glyph_copy);
		if (ft_error != 0)
			return NULL;
	}

	// add glyph into cache
	bmp_glyph = (const FT_BitmapGlyph) glyph;
	glyph_cache_instance.add_glyph(trait, glyph_index, bmp_glyph, _cache_node_ptr);

	if (need_add_ref)
		gdimm_glyph_cache::add_ref(_cache_node_ptr);

	return bmp_glyph;
}

void gdimm_ft_text::update_glyph_pos(UINT options, CONST INT *lpDx)
{
	POINT pen_pos = _cursor;

	// is ETO_PDY is set, lpDx contains both x increment and y displacement
	const int advance_factor = ((options & ETO_PDY) ? 2 : 1);

	for (size_t i = 0; i < _glyphs.size(); i++)
	{
		_glyph_pos[i].x = pen_pos.x - _cursor.x + _glyphs[i]->left;
		_glyph_pos[i].y = pen_pos.y - _cursor.y;

		const LONG char_advance = from_16dot16(_glyphs[i]->root.advance.x) + _char_extra;
		LONG advance_x;

		if (lpDx == NULL)
			advance_x = char_advance;
		else
		{
			advance_x = lpDx[i * advance_factor];

			// the last element in lpDx may be 0
			// in that case, we use the character advancement
			if ((advance_x == 0) && (i == _glyphs.size() - 1))
				advance_x = char_advance;
		}

		pen_pos.x += advance_x;
		pen_pos.y += from_16dot16(_glyphs[i]->root.advance.y);

	}

	_cursor = pen_pos;
}

bool gdimm_ft_text::render(UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx, FT_Render_Mode render_mode)
{
	const wchar_t *dc_font_family = metric_family_name(_outline_metrics);
	wstring curr_font_face = _font_face;
	const wchar_t *curr_font_family = dc_font_family;

	_render_mode = render_mode;
	_load_flags = get_load_flags(render_mode, curr_font_face.c_str());

	/*
	glyph metrics setup

	while the height in FreeType scaler has the same meaning as the height value in LOGFONT structure, the widths are different
	what we know is, when the width in LOGFONT is the xAvgCharWidth defined in the TrueType OS/2 table,
	the corresponding FreeType scaler width is the height
	therefore we need conversion when LOGFONT width is not 0
	simple calculation yields new_freetype_width = logfont_width * em_square / xAvgCharWidth
	note that the tmAveCharWidth field in TEXTMETRIC is the actual LOGFONT width, never be 0
	*/

	long font_id = _font_man.register_font(_hdc_text, curr_font_face.c_str());
	_ft_scaler.face_id = (FTC_FaceID) font_id;
	_ft_scaler.height = _outline_metrics->otmTextMetrics.tmHeight - _outline_metrics->otmTextMetrics.tmInternalLeading;
	update_embolden(_font_man.get_usWeightClass(font_id), curr_font_face.c_str());
	_is_italic = (_font_man.get_fsSelection(font_id) & 1);

	if (_font_attr.lfWidth == 0)
		_ft_scaler.width = _ft_scaler.height;
	else
	{
		// compare the xAvgCharWidth against the current average char width
		_ft_scaler.width = MulDiv(_outline_metrics->otmTextMetrics.tmAveCharWidth,
			_outline_metrics->otmEMSquare,
			_font_man.get_xAvgCharWidth(font_id));
	}

	if (options & ETO_GLYPH_INDEX)
	{
		// directly render glyph indices with the current DC font

		_glyphs.resize(c);
		_glyph_pos.resize(c);

		for (UINT i = 0; i < c; i++)
		{
			const FT_BitmapGlyph curr_glyph = render_glyph(lpString[i], curr_font_face.c_str());
			if (curr_glyph == NULL)
				_glyphs[i] = &empty_glyph;
			else
				_glyphs[i] = curr_glyph;
		}
	}
	else
	{
		UINT rendered_count = 0;
		int font_link_index = 0;
		wstring final_string = lpString;
		wstring glyph_indices;

		_glyphs.resize(c);
		_glyph_pos.resize(c);
		glyph_indices.resize(c);

		while (true)
		{
			_font_man.get_glyph_indices((long) _ft_scaler.face_id, &final_string[0], c, &glyph_indices[0]);

			for (UINT i = 0; i < c; i++)
			{
				if (glyph_indices[i] == 0xffff || final_string[i] == 0)
					continue;

				if (iswcntrl(final_string[i]))
					_glyphs[i] = (const FT_BitmapGlyph) &empty_glyph;
				else
				{
					const FT_BitmapGlyph curr_glyph = render_glyph(glyph_indices[i], curr_font_face.c_str());
					if (curr_glyph == NULL)
						_glyphs[i] = &empty_glyph;
					else
						_glyphs[i] = curr_glyph;
				}

				final_string[i] = 0;
				rendered_count += 1;
			}

			if (rendered_count == c)
				break;

			// font linking
			curr_font_family = font_link_instance.lookup_link(dc_font_family, font_link_index);
			font_link_index += 1;

			if (curr_font_family == NULL)
				return false;

			font_id = _font_man.lookup_font(_font_attr, curr_font_family, curr_font_face);
			assert(font_id != 0);

			_ft_scaler.face_id = (FTC_FaceID) font_id;
			if (_ft_scaler.face_id == NULL)
				return false;

			if (!get_ft_render_mode(curr_font_face.c_str(), _bmp_info.bmiHeader.biBitCount, _render_mode))
				return false;

			_load_flags = get_load_flags(_render_mode, curr_font_face.c_str());

			update_embolden(_font_man.get_usWeightClass(font_id), curr_font_face.c_str());
			_is_italic = (_font_man.get_fsSelection(font_id) & 1);
		}
	}

	bool glyph_all_empty = true;
	for (UINT i = 0; i < c; i++)
	{
		if (_glyphs[i]->bitmap.buffer != NULL)
			glyph_all_empty = false;
	}

	if (glyph_all_empty)
		return false;

	update_glyph_pos(options, lpDx);

	return true;
}

bool gdimm_ft_text::begin(HDC hdc, const OUTLINETEXTMETRICW *outline_metrics, const wchar_t *font_face, const font_setting_cache *setting_cache)
{
	if (!gdimm_gdi_text::begin(hdc, outline_metrics, font_face, setting_cache))
		return false;

	_cache_node_ptr = NULL;

	return true;
}

void gdimm_ft_text::end()
{
	// the cache node associated with this renderer is not used any more
	// the glyph cache can reclaim it safely
	if (_cache_node_ptr != NULL)
		gdimm_glyph_cache::release(_cache_node_ptr);
}