#include "stdafx.h"
#include "ft_painter.h"
#include "helper_func.h"
#include "freetype.h"
#include "gdimm.h"
#include "lock.h"

/*
empty glyph is glyph with no bitmap data and zero bounding box
it acts as a placeholder of non-printable characters (e.g. control characters)
it provides unified interface to glyph drawer
*/
FT_BitmapGlyphRec empty_glyph = {};

FTC_ScalerRec gdimm_ft_text::get_scaler(const OUTLINETEXTMETRICW *outline_metrics, bool width_specified, FT_Short xAvgCharWidth)
{
	/*
	while the height in FreeType scaler has the same meaning as the height value in LOGFONT structure, the width is different
	what we know is, when the width in LOGFONT is the xAvgCharWidth (from the OS/2 table), the corresponding FreeType scaler width is the height
	therefore we need conversion when LOGFONT width is not 0
	simple calculation yields freetype_width = logfont_width * em_square / xAvgCharWidth
	note that the tmAveCharWidth field in TEXTMETRIC is the actual LOGFONT width, which is never 0
	*/

	assert(outline_metrics != NULL);

	FTC_ScalerRec scaler = {};
	scaler.height = outline_metrics->otmTextMetrics.tmHeight - outline_metrics->otmTextMetrics.tmInternalLeading;
	scaler.pixel = 1;

	if (width_specified)
	{
		// compare the xAvgCharWidth against the current average char width
		scaler.width = outline_metrics->otmTextMetrics.tmAveCharWidth * outline_metrics->otmEMSquare / xAvgCharWidth;
	}
	else
		scaler.width = scaler.height * outline_metrics->otmTextMetrics.tmDigitizedAspectX / outline_metrics->otmTextMetrics.tmDigitizedAspectY;

	return scaler;
}

FT_ULong gdimm_ft_text::get_load_flags(const font_setting_cache *setting_cache, FT_Render_Mode render_mode)
{
	FT_ULong load_flags = FT_LOAD_CROP_BITMAP | FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH |
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

void gdimm_ft_text::oblique_outline(const FT_Outline *outline, double slant_adv)
{
	// advancement of slant on x-axis
	FT_Matrix oblique_mat = {to_16dot16(1), to_16dot16(slant_adv), 0, to_16dot16(1)};
	FT_Outline_Transform(outline, &oblique_mat);
}

FT_F26Dot6 gdimm_ft_text::get_embolden(const font_setting_cache *setting_cache, unsigned char font_weight_class)
{
	FT_F26Dot6 embolden = setting_cache->embolden;

	// if non-regular weight is demanded, and the font has no native weighted glyphs, do embolden
	// the embolden weight is based on the difference between demanded weight and the regular weight
	if (_font_attr.lfWeight != FW_DONTCARE)
	{
		const FT_F26Dot6 embolden_values[] = {-32, -16, 0, 16, 32};
		const unsigned char embolden_class_count = sizeof(embolden_values) / sizeof(FT_F26Dot6);
		const unsigned char regular_embolden_class = (embolden_class_count - 1) / 2;

		const unsigned char text_weight_class = get_gdi_weight_class((unsigned short) _context->outline_metrics->otmTextMetrics.tmWeight);
		char embolden_class = text_weight_class - font_weight_class + regular_embolden_class;

		if (embolden_class < 0)
			embolden_class = 0;
		else if (embolden_class >= embolden_class_count)
			embolden_class = embolden_class_count - 1;

		embolden += embolden_values[embolden_class];
	}

	return embolden;
}

const FT_BitmapGlyph gdimm_ft_text::render_glyph(WORD glyph_index,
	const FTC_Scaler scaler,
	FT_F26Dot6 embolden,
	FT_Render_Mode render_mode,
	FT_ULong load_flags,
	bool is_italic,
	const void *&cache_node_ptr)
{
	FT_Error ft_error;

	// if italic style is demanded, and the font has italic glyph, do oblique transformation
	const bool is_oblique = ((_context->outline_metrics->otmTextMetrics.tmItalic != 0) && !is_italic);
	
	// each renderer instance add the reference count to the glyph cache at most once
	// only add reference count in the first time for a glyph run
	const bool need_add_ref = (cache_node_ptr == NULL);

	// lookup if there is already a cached glyph
	const gdimm_glyph_cache::cache_trait trait = {scaler->face_id, scaler->width, scaler->height, embolden, is_oblique, render_mode, load_flags};
	FT_BitmapGlyph bmp_glyph = glyph_cache_instance.lookup_glyph(trait, glyph_index, cache_node_ptr);
	if (bmp_glyph != NULL)
	{
		// this renderer is using the cache node
		// the glyph cache should not reclaim it until the rendering is finished
		if (need_add_ref)
			gdimm_glyph_cache::add_ref(cache_node_ptr);

		return bmp_glyph;
	}

	// no cached glyph, lookup outline and rasterize

	FT_Glyph glyph, cached_glyph;

	// this FreeType function seems not thread-safe
	{
		gdimm_lock lock(LOCK_TEXT);
		ft_error = FTC_ImageCache_LookupScaler(ft_glyph_cache, scaler, load_flags, glyph_index, &cached_glyph, NULL);
		if (ft_error != 0)
			return NULL;
	}
	FT_Load_Glyph
	// some fonts are embedded with pre-rendered glyph bitmap
	// in that case, use original ExtTextOutW
	if (cached_glyph->format != FT_GLYPH_FORMAT_OUTLINE)
		return NULL;

	const bool need_embolden = (embolden != 0);
	const bool need_glyph_copy = (is_oblique || need_embolden);

	if (need_glyph_copy)
	{
		FT_Glyph_Copy(cached_glyph, &glyph);
		FT_Outline *glyph_outline = &((FT_OutlineGlyph) glyph)->outline;

		// it seems faster if oblique first, and then embolden
		if (is_oblique)
			oblique_outline(glyph_outline, 0.3);

		if (need_embolden)
		{
			ft_error = FT_Outline_Embolden(glyph_outline, embolden);
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
	bmp_glyph = (FT_BitmapGlyph) glyph;
	glyph_cache_instance.add_glyph(trait, glyph_index, bmp_glyph, cache_node_ptr);

	if (need_add_ref)
		gdimm_glyph_cache::add_ref(cache_node_ptr);

	return bmp_glyph;
}

void gdimm_ft_text::update_glyph_pos(UINT options, CONST INT *lpDx)
{
	POINT pen_pos = {};

	// is ETO_PDY is set, lpDx contains both x increment and y displacement
	const int advance_factor = ((options & ETO_PDY) ? 2 : 1);

	for (size_t i = 0; i < _glyphs.size(); i++)
	{
		pen_pos.x += _glyph_pos[i].x;
		_glyph_pos[i].x = pen_pos.x + _glyphs[i]->left;

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
}

bool gdimm_ft_text::render(UINT options, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
	wstring curr_font_face = metric_face_name(_context->outline_metrics);
	const wchar_t *dc_font_family = metric_family_name(_context->outline_metrics);
	const font_setting_cache *curr_setting_cache = _context->setting_cache;

	long font_id = _font_man.register_font(_context->hdc, curr_font_face.c_str());
	assert(font_id >= 0);

	const gdimm_os2_metrics *os2_metrics = _font_man.lookup_os2_metrics(font_id);

	FTC_ScalerRec scaler = get_scaler(_context->outline_metrics, _font_attr.lfWidth != 0, os2_metrics->get_xAvgCharWidth());
	scaler.face_id = (FTC_FaceID) font_id;

	FT_F26Dot6 curr_embolden = get_embolden(curr_setting_cache, os2_metrics->get_weight_class());
	FT_ULong curr_load_flags = get_load_flags(curr_setting_cache, _render_mode);

	if (options & ETO_GLYPH_INDEX)
	{
		// directly render glyph indices with the current DC font

		_glyphs.resize(c);
		_glyph_pos.resize(c);
		const void *cache_node_ptr = NULL;

		for (UINT i = 0; i < c; i++)
		{
			const FT_BitmapGlyph curr_glyph = render_glyph(lpString[i],
				&scaler,
				curr_embolden,
				_render_mode,
				curr_load_flags,
				os2_metrics->is_italic(),
				cache_node_ptr);
			if (curr_glyph == NULL)
				_glyphs[i] = &empty_glyph;
			else
				_glyphs[i] = curr_glyph;

			if (lpDx == NULL && curr_setting_cache->kerning && i > 0)
				_glyph_pos[i].x = _font_man.lookup_kern(&scaler, lpString[i-1], lpString[i]);
		}

		if (cache_node_ptr != NULL)
			_cache_node_ptrs.push_back(cache_node_ptr);
	}
	else
	{
		FT_Render_Mode curr_render_mode = _render_mode;

		UINT rendered_count = 0;
		int font_link_index = 0;
		wstring final_string(lpString, c);
		wstring glyph_indices(L"", c);

		_glyphs.resize(c);
		_glyph_pos.resize(c);

		while (true)
		{
			const void *cache_node_ptr = NULL;

			_font_man.get_glyph_indices((long) scaler.face_id, &final_string[0], c, &glyph_indices[0]);

			for (UINT i = 0; i < c; i++)
			{
				if (final_string[i] == L'\0')
					continue;

				// do not render control characters, even corresponding glyphs exist
				if (iswcntrl(final_string[i]))
					_glyphs[i] = (FT_BitmapGlyph) &empty_glyph;
				else if (glyph_indices[i] != 0xffff)
				{
					const FT_BitmapGlyph curr_glyph = render_glyph(glyph_indices[i],
						&scaler,
						curr_embolden,
						_render_mode,
						curr_load_flags, 
						os2_metrics->is_italic(),
						cache_node_ptr);
					if (curr_glyph == NULL)
						_glyphs[i] = &empty_glyph;
					else
						_glyphs[i] = curr_glyph;

					if (lpDx == NULL && curr_setting_cache->kerning && i > 0)
						_glyph_pos[i].x = _font_man.lookup_kern(&scaler, glyph_indices[i-1], glyph_indices[i]);
				}
				else
					continue;
					
				final_string[i] = L'\0';
				rendered_count += 1;
			}

			if (cache_node_ptr != NULL)
				_cache_node_ptrs.push_back(cache_node_ptr);

			if (rendered_count >= c)
			{
				assert(rendered_count == c);
				break;
			}

			// font linking

			const font_link_info *linked_info = font_link_instance.lookup_link(dc_font_family, font_link_index);
			font_link_index += 1;

			if (linked_info == NULL)
				return false;

			font_id = _font_man.lookup_font(_font_attr, linked_info->font_family.c_str(), curr_font_face);
			assert(font_id < 0);

			// reload metrics for the linked font

			// get linked font's metrics from OS/2 table
			os2_metrics = _font_man.lookup_os2_metrics(font_id);

			scaler = get_scaler(_font_man.lookup_outline_metrics(font_id));
			scaler.face_id = (FTC_FaceID) font_id;

			if (linked_info->scaling != 1.0)
			{
				// apply font linking scaling factor
				scaler.width = (FT_UInt)(scaler.width * linked_info->scaling);
				scaler.height = (FT_UInt)(scaler.height * linked_info->scaling);
			}

			const gdimm_font_trait font_trait = {curr_font_face.c_str(), os2_metrics->get_weight_class(), os2_metrics->is_italic()};
			curr_setting_cache = setting_cache_instance.lookup(font_trait);
			curr_embolden = get_embolden(curr_setting_cache, font_trait.weight_class);
			curr_load_flags = get_load_flags(curr_setting_cache, _render_mode);
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

bool gdimm_ft_text::begin(const dc_context *context)
{
	if (!gdimm_gdi_painter::begin(context))
		return false;

	_cache_node_ptrs.clear();

	return true;
}

void gdimm_ft_text::end()
{
	// the cache nodes associated with this renderer will not be used any more
	// the glyph cache can reclaim them safely
	for (list<const void *>::const_iterator iter = _cache_node_ptrs.begin(); iter != _cache_node_ptrs.end(); iter++)
		gdimm_glyph_cache::release(*iter);
}