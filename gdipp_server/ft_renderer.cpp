#include "stdafx.h"
#include "ft_renderer.h"
#include "gdipp_lib/lock.h"
#include "gdipp_server/freetype.h"
#include "gdipp_server/global.h"
#include "gdipp_server/helper.h"

namespace gdipp
{

ft_renderer::ft_renderer(rpc_session *render_session)
	: renderer(render_session)
{
}

FT_F26Dot6 ft_renderer::get_embolden_diff(char font_weight_class, char text_weight_class)
{
	// the embolden weight is based on the difference between demanded weight and the regular weight

	const FT_F26Dot6 embolden_values[] = {-32, -16, 0, 16, 32};
	const char embolden_class_count = sizeof(embolden_values) / sizeof(FT_F26Dot6);
	const char regular_embolden_class = (embolden_class_count - 1) / 2;

	char embolden_class = text_weight_class - font_weight_class + regular_embolden_class;

	if (embolden_class < 0)
		embolden_class = 0;
	else if (embolden_class >= embolden_class_count)
		embolden_class = embolden_class_count - 1;

	return embolden_values[embolden_class];
}

void ft_renderer::get_font_width_height(const OUTLINETEXTMETRICW *outline_metrics, FT_Short xAvgCharWidth, FT_UInt &font_width, FT_UInt &font_height)
{
	/*
	while the height in FreeType scaler has the same meaning as the height value in LOGFONT structure, the width is different
	what we know is, when the width in LOGFONT is the xAvgCharWidth (from the OS/2 table), the corresponding FreeType scaler width is the height
	therefore we need conversion when LOGFONT width is not 0
	simple calculation yields freetype_width = logfont_width * em_square / xAvgCharWidth
	note that the tmAveCharWidth field in TEXTMETRIC is the actual LOGFONT width, which is never 0
	*/

	assert(outline_metrics != NULL);

	font_height = outline_metrics->otmTextMetrics.tmHeight - outline_metrics->otmTextMetrics.tmInternalLeading;

	if (xAvgCharWidth == 0)
		font_width = font_height * outline_metrics->otmTextMetrics.tmDigitizedAspectX / outline_metrics->otmTextMetrics.tmDigitizedAspectY;
	else
	{
		// compare the xAvgCharWidth against the current average char width
		font_width = outline_metrics->otmTextMetrics.tmAveCharWidth * outline_metrics->otmEMSquare / xAvgCharWidth;
	}
}

FT_ULong ft_renderer::make_load_flags(const render_config_static *render_config, FT_Render_Mode render_mode)
{
	FT_ULong load_flags = FT_LOAD_CROP_BITMAP | (render_config->embedded_bitmap ? 0 : FT_LOAD_NO_BITMAP);

	if (render_config->hinting == 0)
		load_flags |= FT_LOAD_NO_HINTING;
	else
	{
		switch (render_config->hinting)
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

		switch (render_config->auto_hinting)
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

void ft_renderer::oblique_outline(const FT_Outline *outline, double slant_adv)
{
	// advance of slant on x-axis
	FT_Matrix oblique_mat = {float_to_16dot16(1), float_to_16dot16(slant_adv), 0, float_to_16dot16(1)};
	FT_Outline_Transform(outline, &oblique_mat);
}

bool ft_renderer::generate_outline_glyph(FT_Glyph *glyph,
	WORD glyph_index,
	const FTC_Scaler scaler,
	FT_F26Dot6 embolden,
	FT_ULong load_flags,
	bool is_italic) const
{
	FT_Error ft_error;

	FT_Glyph cached_glyph;

	{
		// the FreeType function seems not thread-safe
		lock l(lock::SERVER_FREETYPE);
		ft_error = FTC_ImageCache_LookupScaler(ft_glyph_cache, scaler, load_flags, glyph_index, &cached_glyph, NULL);
		if (ft_error != 0)
			return NULL;
	}

	// some fonts are embedded with pre-rendered glyph bitmap
	// in that case, use original ExtTextOutW
	if (cached_glyph->format != FT_GLYPH_FORMAT_OUTLINE)
		return NULL;

	// if italic style is demanded, and the font has italic glyph, do oblique transformation
	const OUTLINETEXTMETRICW *outline_metrics = reinterpret_cast<const OUTLINETEXTMETRICW *>(_session->outline_metrics_buf);
	const bool is_oblique = ((outline_metrics->otmTextMetrics.tmItalic != 0) && !is_italic);
	const bool need_embolden = (embolden != 0);
	const bool need_glyph_copy = (is_oblique || need_embolden);

	if (need_glyph_copy)
	{
		FT_Glyph_Copy(cached_glyph, glyph);
		FT_Outline *glyph_outline = &(reinterpret_cast<FT_OutlineGlyph>(*glyph)->outline);

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
		*glyph = cached_glyph;

	return need_glyph_copy;
}

const FT_Glyph ft_renderer::generate_bitmap_glyph(WORD glyph_index,
	const FTC_Scaler scaler,
	FT_Render_Mode render_mode,
	FT_F26Dot6 embolden,
	FT_ULong load_flags,
	bool is_italic,
	bool request_outline,
	uint128_t render_trait) const
{
	FT_Error ft_error;
	FT_Glyph glyph;

	if (request_outline)
	{
		generate_outline_glyph(&glyph, glyph_index, scaler, embolden, load_flags, is_italic);
		return glyph;
	}

	const glyph_cache::char_id_type char_id = glyph_cache::get_char_id(render_trait, glyph_index, true);
	glyph = glyph_cache_instance.lookup_glyph(char_id);
	if (glyph == NULL)
	{
		// no cached glyph, or outline glyph is requested, generate outline
		const bool is_local_glyph = generate_outline_glyph(&glyph, glyph_index, scaler, embolden, load_flags, is_italic);

		// outline -> bitmap conversion
		{
			// the FreeType function seems not thread-safe
			lock l(lock::SERVER_FREETYPE);
			ft_error = FT_Glyph_To_Bitmap(&glyph, render_mode, NULL, is_local_glyph);
			if (ft_error != 0)
				return NULL;
		}

		glyph_cache_instance.store_glyph(char_id, glyph);
	}

	return glyph;
}

bool ft_renderer::generate_glyph_run(bool is_glyph_index, LPCWSTR lpString, UINT c, glyph_run *new_glyph_run, bool request_outline)
{
	const OUTLINETEXTMETRICW *curr_outline_metrics = reinterpret_cast<const OUTLINETEXTMETRICW *>(_session->outline_metrics_buf);
	const render_config_static *curr_render_config = _session->render_config;
	uint128_t curr_render_trait = _session->render_trait;
	const wchar_t *curr_font_face = metric_face_name(curr_outline_metrics);
	const os2_metrics *curr_os2 = font_mgr_instance.lookup_os2_metrics(_session->font_id);
	const wchar_t *session_font_family = metric_family_name(curr_outline_metrics);

	FTC_ScalerRec scaler = {};
	scaler.face_id = _session->font_id;
	scaler.pixel = 1;
	get_font_width_height(curr_outline_metrics, (_session->log_font.lfWidth == 0 ? 0 : curr_os2->get_xAvgCharWidth()), scaler.height, scaler.width);

	FT_F26Dot6 curr_embolden = 0;
	if (_session->log_font.lfWeight != FW_DONTCARE)
	{
		// embolden if some weight is demanded
		curr_embolden = curr_render_config->embolden + get_embolden_diff(curr_os2->get_weight_class(), static_cast<char>(_session->log_font.lfWeight));
	}

	FT_ULong curr_load_flags = make_load_flags(curr_render_config, _session->render_mode);

	if (is_glyph_index)
	{
		// directly render glyph indices with the current DC font

		for (UINT i = 0; i < c; ++i)
		{
			const FT_Glyph new_glyph = generate_bitmap_glyph(lpString[i],
				&scaler,
				_session->render_mode,
				curr_embolden,
				curr_load_flags,
				curr_os2->is_italic(),
				request_outline,
				curr_render_trait);
			RECT ctrl_box = {}, black_box = {};

			if (new_glyph == NULL)
			{
				if (request_outline)
					return false;
			}
			else if (curr_render_config->kerning && i > 0 && !request_outline)
			{
				ctrl_box.left = get_freetype_kern(&scaler, lpString[i-1], lpString[i]);
				ctrl_box.right = ctrl_box.left;
			}

			new_glyph_run->glyphs.push_back(new_glyph);
			new_glyph_run->ctrl_boxes.push_back(ctrl_box);
			new_glyph_run->black_boxes.push_back(black_box);
		}
	}
	else
	{
		FT_Render_Mode curr_render_mode = _session->render_mode;

		UINT rendered_count = 0;
		int font_link_index = 0;
		std::wstring final_string(lpString, c);
		std::vector<unsigned short> glyph_indices(c);

		new_glyph_run->glyphs.resize(c);
		new_glyph_run->ctrl_boxes.resize(c);
		new_glyph_run->black_boxes.resize(c);

		while (true)
		{
			font_mgr_instance.lookup_glyph_indices(scaler.face_id, final_string.data(), c, &glyph_indices[0]);

			std::vector<FT_Glyph>::iterator glyph_iter;
			std::vector<RECT>::iterator ctrl_iter, black_iter;
			UINT i;
			for (glyph_iter = new_glyph_run->glyphs.begin(), ctrl_iter = new_glyph_run->ctrl_boxes.begin(), black_iter = new_glyph_run->black_boxes.begin(), i = 0;
				i < c; i++, glyph_iter++, ctrl_iter++, black_iter++)
			{
				if (final_string[i] == L'\0')
					continue;

				// do not render control characters, even the corresponding glyphs exist in font
				if (iswcntrl(final_string[i]) && !request_outline)
					*glyph_iter = NULL;
				else if (glyph_indices[i] != 0xffff)
				{
					*glyph_iter = generate_bitmap_glyph(glyph_indices[i],
						&scaler,
						curr_render_mode,
						curr_embolden,
						curr_load_flags, 
						curr_os2->is_italic(),
						request_outline,
						curr_render_trait);

					if (*glyph_iter == NULL)
					{
						if (request_outline)
							return false;
					}
					else if (curr_render_config->kerning && i > 0 && !request_outline)
					{
						ctrl_iter->left = get_freetype_kern(&scaler, glyph_indices[i-1], glyph_indices[i]);
						ctrl_iter->right = ctrl_iter->left;
					}
				}
				else
					continue;
					
				final_string[i] = L'\0';
				rendered_count += 1;
			}

			if (rendered_count >= c)
			{
				assert(rendered_count == c);
				break;
			}

			// font linking

			const font_link_node *curr_link = font_link_instance.lookup_link(session_font_family, font_link_index);
			if (curr_link == NULL)
				return false;
			font_link_index += 1;
			
			LOGFONTW linked_log_font = _session->log_font;
			/*
			this reset is essential to make GetGlyphIndices work correctly
			for example, lfOutPrecision might be OUT_PS_ONLY_PRECIS for Myriad Pro
			if create HFONT of Microsoft YaHei with such lfOutPrecision, GetGlyphIndices always fails
			*/
			linked_log_font.lfOutPrecision = OUT_DEFAULT_PRECIS;
			wcsncpy_s(linked_log_font.lfFaceName, curr_link->font_family.c_str(), LF_FACESIZE);

			BYTE *curr_outline_metrics_buf;
			unsigned long curr_outline_metrics_size;
			scaler.face_id = font_mgr_instance.register_font(&linked_log_font, &curr_outline_metrics_buf, &curr_outline_metrics_size);
			assert(scaler.face_id != NULL);

			// reload metrics for the linked font

			const OUTLINETEXTMETRICW *curr_outline_metrics = reinterpret_cast<const OUTLINETEXTMETRICW *>(curr_outline_metrics_buf);
			curr_font_face = metric_face_name(curr_outline_metrics);

			if (curr_link->scaling != 1.0)
			{
				// apply font linking scaling factor
				scaler.width = static_cast<FT_UInt>(scaler.width * curr_link->scaling);
				scaler.height = static_cast<FT_UInt>(scaler.height * curr_link->scaling);
			}

			curr_os2 = font_mgr_instance.lookup_os2_metrics(scaler.face_id);
			const char font_weight_class = curr_os2->get_weight_class();
			const LONG point_size = (linked_log_font.lfHeight > 0 ? linked_log_font.lfHeight : -MulDiv(linked_log_font.lfHeight, 72, curr_outline_metrics->otmTextMetrics.tmDigitizedAspectY));

			curr_render_config = font_render_config_cache_instance.get_font_render_config(!!font_weight_class,
				curr_os2->is_italic(),
				point_size,
				curr_font_face);

			delete[] curr_outline_metrics_buf;

			if (!get_render_mode(curr_render_config->render_mode, _session->bits_per_pixel, _session->log_font.lfQuality, &curr_render_mode))
				return false;

			curr_render_trait = generate_render_trait(&linked_log_font, curr_render_mode);

			curr_embolden = 0;
			if (linked_log_font.lfWeight != FW_DONTCARE)
				curr_embolden = curr_render_config->embolden + get_embolden_diff(font_weight_class, static_cast<char>(linked_log_font.lfWeight));

			curr_load_flags = make_load_flags(curr_render_config, _session->render_mode);
		}
	}

	return true;
}

bool ft_renderer::render(bool is_glyph_index, LPCWSTR lpString, UINT c, glyph_run *new_glyph_run)
{
	bool b_ret;

	b_ret = generate_glyph_run(is_glyph_index, lpString, c, new_glyph_run, false);
	if (!b_ret)
		return false;

	POINT pen_pos = {};

	std::vector<FT_Glyph>::iterator glyph_iter;
	std::vector<RECT>::iterator ctrl_iter, black_iter;
	for (glyph_iter = new_glyph_run->glyphs.begin(), ctrl_iter = new_glyph_run->ctrl_boxes.begin(), black_iter = new_glyph_run->black_boxes.begin();
		glyph_iter != new_glyph_run->glyphs.end(); ++glyph_iter, ++ctrl_iter, ++black_iter)
	{
		FT_Int glyph_left = 0, glyph_width = 0;
		FT_Vector glyph_advance = {};

		const FT_BitmapGlyph bmp_glyph = reinterpret_cast<FT_BitmapGlyph>(*glyph_iter);
		if (bmp_glyph != NULL)
		{
			glyph_left = bmp_glyph->left;
			glyph_width = get_glyph_bmp_width(bmp_glyph->bitmap);
			glyph_advance = bmp_glyph->root.advance;
		}

		ctrl_iter->left += pen_pos.x;
		ctrl_iter->top += pen_pos.y;
		black_iter->left = ctrl_iter->left + glyph_left;
		black_iter->top = ctrl_iter->top;

		pen_pos.x += int_from_16dot16(glyph_advance.x);
		pen_pos.y += int_from_16dot16(glyph_advance.y);

		ctrl_iter->right += pen_pos.x;
		ctrl_iter->bottom += pen_pos.y;
		black_iter->right = black_iter->left + glyph_width;
		black_iter->bottom = ctrl_iter->bottom;
	}

	return true;
}

}
