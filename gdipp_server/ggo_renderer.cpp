#include "stdafx.h"
#include "ggo_renderer.h"
#include "gdipp_lib/lock.h"
#include "gdipp_server/freetype.h"
#include "gdipp_server/global.h"
#include "gdipp_server/helper.h"

namespace gdipp
{

ggo_renderer::ggo_renderer(rpc_session *render_session)
	: renderer(render_session)
{
}

bool ggo_renderer::render(bool is_glyph_index, LPCWSTR lpString, UINT c, glyph_run *new_glyph_run)
{
	bool b_ret;

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

	if (_session->render_config->hinting == 0)
		_ggo_format |= GGO_UNHINTED;

	POINT pen_pos = {};

	for (UINT i = 0; i < c; ++i)
	{
		GLYPHMETRICS glyph_metrics = {};
		FT_Glyph new_glyph;

		// we do not care about non-printable characters
		// solution for Windows Vista/7 Date
		if (is_glyph_index || !iswcntrl(lpString[i]))
		{
			new_glyph = glyph_cache_instance.lookup_glyph(_session->render_trait, lpString[i], is_glyph_index);
			if (new_glyph == NULL)
			{
				new_glyph = outline_to_bitmap(lpString[i], glyph_metrics);
			}
			else
			{
				b_ret = get_glyph_metrics(lpString[i], glyph_metrics);
				if (!b_ret)
					return b_ret;
			}
		}

		FT_Int glyph_left = 0, glyph_width = 0;

		if (new_glyph != NULL)
		{
			const FT_BitmapGlyph bmp_glyph = reinterpret_cast<FT_BitmapGlyph>(new_glyph);
			glyph_left = bmp_glyph->left;
			glyph_width = get_glyph_bmp_width(bmp_glyph->bitmap);
		}

		RECT ctrl_box, black_box;
		ctrl_box.left = pen_pos.x;
		ctrl_box.top = pen_pos.y;
		black_box.left = ctrl_box.left + glyph_left;
		black_box.top = ctrl_box.top;

		pen_pos.x += glyph_metrics.gmCellIncX;
		pen_pos.y += glyph_metrics.gmCellIncY;

		ctrl_box.right = pen_pos.x;
		ctrl_box.bottom = pen_pos.y;
		black_box.right = black_box.left + glyph_width;
		black_box.bottom = ctrl_box.bottom;

		new_glyph_run->glyphs[i] = new_glyph;
		new_glyph_run->ctrl_boxes[i] = ctrl_box;
		new_glyph_run->black_boxes[i] = black_box;
	}

	return true;
}

void ggo_renderer::outline_ggo_to_ft(DWORD ggo_outline_buf_len, const BYTE *ggo_outline_buf, std::vector<FT_Vector> &curve_points, std::vector<char> &curve_tags, std::vector<short> &contour_indices)
{
	// parse outline coutours
	DWORD header_off = 0;
	do
	{
		const BYTE *header_ptr = ggo_outline_buf + header_off;
		const TTPOLYGONHEADER *header = reinterpret_cast<const TTPOLYGONHEADER *>(header_ptr);

		// FreeType uses 26.6 format, while Windows gives logical units
		const FT_Vector start_point = {fixed_to_26dot6(header->pfxStart.x), fixed_to_26dot6(header->pfxStart.y)};

		DWORD curve_off = sizeof(TTPOLYGONHEADER);
		while (curve_off < header->cb)
		{
			// the starting point of each curve is the last point of the previous curve or the starting point of the contour
			if (curve_off == sizeof(TTPOLYGONHEADER))
			{
				curve_points.push_back(start_point);
				// the first point is on the curve
				curve_tags.push_back(FT_CURVE_TAG_ON);
			}

			const TTPOLYCURVE *curve = reinterpret_cast<const TTPOLYCURVE *>(header_ptr + curve_off);
			char curve_tag;
			switch (curve->wType)
			{
			case TT_PRIM_LINE:
				curve_tag = FT_CURVE_TAG_ON;
				break;
			case TT_PRIM_QSPLINE:
				curve_tag = FT_CURVE_TAG_CONIC;
				break;
			case TT_PRIM_CSPLINE:
				curve_tag = FT_CURVE_TAG_CUBIC;
				break;
			}

			for (int j = 0; j < curve->cpfx; ++j)
			{
				const FT_Vector curr_point = {fixed_to_26dot6(curve->apfx[j].x), fixed_to_26dot6(curve->apfx[j].y)};
				curve_points.push_back(curr_point);
				curve_tags.push_back(curve_tag);
			}
			// the last point is on the curve
			curve_tags[curve_tags.size() - 1] = FT_CURVE_TAG_ON;

			curve_off += sizeof(TTPOLYCURVE) + (curve->cpfx - 1) * sizeof(POINTFX);
		}

		contour_indices.push_back(static_cast<short>(curve_points.size() - 1));
		header_off += header->cb;
	} while (header_off < ggo_outline_buf_len);

	assert(curve_points.size() <= FT_OUTLINE_POINTS_MAX);
}

bool ggo_renderer::get_glyph_metrics(wchar_t ch, GLYPHMETRICS &glyph_metrics) const
{
	DWORD outline_buf_len = GetGlyphOutline(_session->hdc, ch, (_ggo_format | GGO_METRICS), &glyph_metrics, 0, NULL, &_matrix);
	return (outline_buf_len != GDI_ERROR);
}

const FT_Glyph ggo_renderer::outline_to_bitmap(wchar_t ch, GLYPHMETRICS &glyph_metrics) const
{
	bool b_ret;
	FT_Error ft_error;

	FT_OutlineGlyphRec outline_glyph = {*empty_outline_glyph, {}};
	outline_glyph.root.format = FT_GLYPH_FORMAT_OUTLINE;

	DWORD outline_buf_len = GetGlyphOutline(_session->hdc, ch, _ggo_format, &glyph_metrics, 0, NULL, &_matrix);
	assert(outline_buf_len != GDI_ERROR);

	if (outline_buf_len == 0)
	{
		// the glyph outline of this character is empty (e.g. space)
		b_ret = get_glyph_metrics(ch, glyph_metrics);
		assert(b_ret);

		return NULL;
	}
	else
	{
		BYTE *outline_buf = new BYTE[outline_buf_len];
		outline_buf_len = GetGlyphOutline(_session->hdc, ch, _ggo_format, &glyph_metrics, outline_buf_len, outline_buf, &_matrix);
		assert(outline_buf_len != GDI_ERROR);

		std::vector<FT_Vector> curve_points;
		std::vector<char> curve_tags;
		std::vector<short> contour_indices;
		outline_ggo_to_ft(outline_buf_len, outline_buf, curve_points, curve_tags, contour_indices);

		delete[] outline_buf;

		outline_glyph.outline.n_contours = static_cast<short>(contour_indices.size());
		outline_glyph.outline.n_points = static_cast<short>(curve_points.size());
		outline_glyph.outline.points = &curve_points[0];
		outline_glyph.outline.tags = &curve_tags[0];
		outline_glyph.outline.contours = &contour_indices[0];
		outline_glyph.outline.flags = FT_OUTLINE_NONE;

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

		if (_session->render_config->embolden != 0)
		{
			ft_error = FT_Outline_Embolden(&outline_glyph.outline, _session->render_config->embolden);
			assert(ft_error == 0);
		}

		// convert outline to bitmap
		FT_Glyph generic_glyph = reinterpret_cast<FT_Glyph>(&outline_glyph);

		{
			// the FreeType function seems not thread-safe
			lock l(lock::SERVER_FREETYPE);
			ft_error = FT_Glyph_To_Bitmap(&generic_glyph, _session->render_mode, NULL, false);
			if (ft_error != 0)
				return NULL;
		}

		glyph_cache_instance.store_glyph(_session->render_trait, ch, !!(_ggo_format & GGO_GLYPH_INDEX), generic_glyph);

		return generic_glyph;
	}
}

}
