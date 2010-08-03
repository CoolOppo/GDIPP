#include "stdafx.h"
#include "ggo_renderer.h"
#include "freetype.h"
#include "helper_def.h"
#include "helper_func.h"
#include "lock.h"

FT_Glyph gdimm_ggo_renderer::empty_glyph;

gdimm_ggo_renderer::gdimm_ggo_renderer()
{
	FT_Error ft_error;

	FT_GlyphSlotRec glyph_slot = {};
	glyph_slot.library = ft_lib;
	glyph_slot.format = FT_GLYPH_FORMAT_OUTLINE;
	ft_error = FT_Get_Glyph(&glyph_slot, &empty_glyph);
	assert(ft_error == 0);
}

bool gdimm_ggo_renderer::get_glyph_metrics(wchar_t ch, GLYPHMETRICS &glyph_metrics) const
{
	DWORD outline_buf_len = GetGlyphOutline(_context->hdc, ch, (_ggo_format | GGO_METRICS), &glyph_metrics, 0, NULL, &_matrix);
	return (outline_buf_len != GDI_ERROR);
}

const FT_BitmapGlyph gdimm_ggo_renderer::outline_to_bitmap(wchar_t ch, GLYPHMETRICS &glyph_metrics) const
{
	FT_Error ft_error;

	vector<FT_Vector> curve_points;
	vector<char> curve_tags;
	vector<short> contour_indices;

	FT_OutlineGlyphRec outline_glyph = 
	{
		*empty_glyph,
		{}
	};

	DWORD outline_buf_len = GetGlyphOutline(_context->hdc, ch, _ggo_format, &glyph_metrics, 0, NULL, &_matrix);
	assert(outline_buf_len != GDI_ERROR);

	if (outline_buf_len == 0)
	{
		// the glyph outline of this character is empty (e.g. space)
		if (!get_glyph_metrics(ch, glyph_metrics))
			return NULL;
	}
	else
	{
		BYTE *outline_buf = new BYTE[outline_buf_len];
		outline_buf_len = GetGlyphOutline(_context->hdc, ch, _ggo_format, &glyph_metrics, outline_buf_len, outline_buf, &_matrix);
		assert(outline_buf_len != GDI_ERROR);

		// parse outline coutours
		DWORD header_off = 0;
		do
		{
			const BYTE *header_ptr = outline_buf + header_off;
			const TTPOLYGONHEADER *header = (TTPOLYGONHEADER *)header_ptr;

			// FreeType uses 26.6 format, while Windows gives logical units
			const FT_Vector start_point = {to_26dot6(header->pfxStart.x), to_26dot6(header->pfxStart.y)};

			DWORD curve_off = sizeof(TTPOLYGONHEADER);
			while (curve_off < header->cb)
			{
				// the starting point of each curve is the last point of the previous curve or the starting point of the contour
				if (curve_off == sizeof(TTPOLYGONHEADER))
					curve_points.push_back(start_point);
				else
					curve_points.push_back(curve_points[curve_points.size() - 1]);
				// the first point is on the curve
				curve_tags.push_back(FT_CURVE_TAG_ON);

				const TTPOLYCURVE *curve = (TTPOLYCURVE *)(header_ptr + curve_off);
				char curr_tag;
				switch (curve->wType)
				{
				case TT_PRIM_LINE:
					curr_tag = FT_CURVE_TAG_ON;
					break;
				case TT_PRIM_QSPLINE:
					curr_tag = FT_CURVE_TAG_CONIC;
					break;
				case TT_PRIM_CSPLINE:
					curr_tag = FT_CURVE_TAG_CUBIC;
					break;
				}

				for (int j = 0; j < curve->cpfx; j++)
				{
					const FT_Vector curr_point = {to_26dot6(curve->apfx[j].x), to_26dot6(curve->apfx[j].y)};
					curve_points.push_back(curr_point);
					curve_tags.push_back(curr_tag);
				}
				// the last point is on the curve
				curve_tags[curve_tags.size() - 1] = FT_CURVE_TAG_ON;

				curve_off += sizeof(TTPOLYCURVE) + (curve->cpfx - 1) * sizeof(POINTFX);
			}

			contour_indices.push_back((short) curve_points.size() - 1);
			header_off += header->cb;
		} while (header_off < outline_buf_len);

		delete[] outline_buf;

		assert(curve_points.size() <= FT_OUTLINE_POINTS_MAX);

		outline_glyph.outline.n_contours = (short) contour_indices.size();
		outline_glyph.outline.n_points = (short) curve_points.size();
		outline_glyph.outline.points = &curve_points[0];
		outline_glyph.outline.tags = &curve_tags[0];
		outline_glyph.outline.contours = &contour_indices[0];
		outline_glyph.outline.flags = FT_OUTLINE_NONE;
	}

	/*
	once in possess of FT_Outline, there are several way to get FT_Bitmap

	1. FT_Outline_Render: could pass a callback span function to directly draw scanlines to DC
	   unfortunately it only output 8-bit bitmap
	2. FT_Outline_Get_Bitmap: merely a wrapper of FT_Outline_Render
	3. FT_Glyph_To_Bitmap: first conglyph_indicesuct FT_OutlineGlyph from FT_Outline, then render glyph to get FT_Bitmap
	   when conglyph_indicesucting FreeType glyph, the private clazz field must be provided
	   support 24-bit bitmap

	we use method 3
	*/

	if (_context->setting_cache->embolden != 0)
	{
		ft_error = FT_Outline_Embolden(&outline_glyph.outline, _context->setting_cache->embolden);
		assert(ft_error == 0);
	}

	// convert outline to bitmap
	FT_Glyph generic_glyph = (FT_Glyph) &outline_glyph;
	ft_error = FT_Glyph_To_Bitmap(&generic_glyph, _context->render_mode, NULL, FALSE);
	if (ft_error != 0)
		return NULL;

	return (const FT_BitmapGlyph) generic_glyph;
}

bool gdimm_ggo_renderer::render(LPCWSTR lpString, UINT c, bool is_glyph_index, CONST INT *lpDx, bool is_pdy, glyph_run &new_glyph_run)
{
	bool b_ret;

	POINT pen_pos = {};

	// identity matrix
	memset(&_matrix, 0, sizeof(MAT2));
	_matrix.eM11.value = 1;
	_matrix.eM22.value = 1;

	/*
	GetGlyphOutline is capable of returning cubic B¨¦zier curves
	although it generally require less points to define a curve with cubic rather than quadratic B¨¦zier curves,
	TrueType fonts internally store curves with quadratic B¨¦zier curves
	GetGlyphOutline has to do conversion, which takes time, and generates more points
	therefore, quadratic B¨¦zier curves are more favored
	*/
	_ggo_format = GGO_NATIVE;
	if (is_glyph_index)
		_ggo_format |= GGO_GLYPH_INDEX;

	if (_context->setting_cache->hinting == 0)
		_ggo_format |= GGO_UNHINTED;

	for (UINT i = 0; i < c; i++)
	{
		// we do not care about non-printable characters
		// solution for Windows Vista/7 Date
		if (!is_glyph_index && iswcntrl(lpString[i]))
			continue;

		GLYPHMETRICS glyph_metrics;
		glyph_info new_glyph;
		new_glyph.glyph = _glyph_cache.lookup_glyph(_context->log_font, lpString[i], is_glyph_index);
		if (new_glyph.glyph == NULL)
			new_glyph.glyph = outline_to_bitmap(lpString[i], glyph_metrics);
		else
		{
			b_ret = get_glyph_metrics(lpString[i], glyph_metrics);
			assert(b_ret);
		}

		new_glyph.bbox.left = pen_pos.x;
		new_glyph.bbox.top = pen_pos.y;
		
		pen_pos.x += glyph_metrics.gmCellIncX + _char_extra;
		pen_pos.y += glyph_metrics.gmCellIncY;

		new_glyph.bbox.right = pen_pos.x;
		new_glyph.bbox.bottom = pen_pos.x;

		if (new_glyph.glyph != NULL)
		{
			_glyph_cache.store_glyph(_context->log_font, lpString[i], is_glyph_index, new_glyph.glyph);
			new_glyph_run.push_back(new_glyph);
		}
	}

	return true;
}

bool gdimm_ggo_renderer::begin(const dc_context *context)
{
	if (!gdimm_renderer::begin(context))
		return false;

	return true;
}