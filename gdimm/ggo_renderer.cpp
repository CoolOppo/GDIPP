#include "stdafx.h"
#include "ggo_renderer.h"
#include "text_helper.h"
#include "ft.h"
#include "lock.h"

const FT_Glyph_Class *ggo_renderer::_glyph_clazz = NULL;

ggo_renderer::ggo_renderer(HDC hdc)
:
gdimm_gdi_text(hdc)
{
}

ggo_renderer::~ggo_renderer()
{
	for (vector<const FT_BitmapGlyph>::const_iterator iter = _glyphs.begin(); iter != _glyphs.end(); iter++)
		FT_Done_Glyph((FT_Glyph) *iter);
}

const FT_BitmapGlyph ggo_renderer::outline_to_bitmap(wchar_t ch, GLYPHMETRICS &glyph_metrics) const
{
	FT_Error ft_error;

	DWORD outline_buf_len = GetGlyphOutline(_hdc_text, ch, _ggo_format, &glyph_metrics, 0, NULL, &_matrix);
	assert(outline_buf_len != GDI_ERROR);

	// some character's glyph outline is empty (e.g. space), skip
	if (outline_buf_len == 0)
		return NULL;
	
	vector<FT_Vector> curve_points;
	vector<char> curve_tags;
	vector<short> contour_indices;

	BYTE *outline_buf = new BYTE[outline_buf_len];
	outline_buf_len = GetGlyphOutline(_hdc_text, ch, _ggo_format, &glyph_metrics, outline_buf_len, outline_buf, &_matrix);
	assert(outline_buf_len != GDI_ERROR);

	// parse outline coutours
	DWORD header_off = 0;
	do
	{
		const BYTE *header_ptr = outline_buf + header_off;
		const TTPOLYGONHEADER *header = (TTPOLYGONHEADER*) header_ptr;

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

			const TTPOLYCURVE *curve = (TTPOLYCURVE*)(header_ptr + curve_off);
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

	FT_OutlineGlyphRec outline_glyph = 
	{
		{
			ft_lib,
			_glyph_clazz,
			FT_GLYPH_FORMAT_OUTLINE,
			0,
			0
		},
		{
			(short) contour_indices.size(),
			(short) curve_points.size(),
			&curve_points[0],
			&curve_tags[0],
			&contour_indices[0],
			FT_OUTLINE_NONE
		}
	};

	if (_setting_cache->embolden != 0)
	{
		ft_error = FT_Outline_Embolden(&outline_glyph.outline, _setting_cache->embolden);
		assert(ft_error == 0);
	}

	// convert outline to bitmap
	FT_Glyph generic_glyph = (FT_Glyph) &outline_glyph;
	
	// FT_Glyph_To_Bitmap is not thread-safe
	{
		gdimm_lock lock(LOCK_TEXT);
		ft_error = FT_Glyph_To_Bitmap(&generic_glyph, _render_mode, NULL, FALSE);
		assert(ft_error == 0);
	}

	return (const FT_BitmapGlyph) generic_glyph;
}

bool ggo_renderer::render(UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx, FT_Render_Mode render_mode)
{
	// is ETO_PDY is set, lpDx contains both x increment and y displacement
	const int advance_factor = ((options & ETO_PDY) ? 2 : 1);

	// identity matrix
	memset(&_matrix, 0, sizeof(MAT2));
	_matrix.eM11.value = 1;
	_matrix.eM22.value = 1;

	_render_mode = render_mode;

	/*
	GetGlyphOutline is capable of returning cubic B¨¦zier curves
	although it generally require less points to define a curve with cubic rather than quadratic B¨¦zier curves,
	TrueType fonts internally store curves with quadratic B¨¦zier curves
	GetGlyphOutline has to do conversion, which takes time, and generates more points
	therefore, quadratic B¨¦zier curves are more favored
	*/
	_ggo_format = GGO_NATIVE;
	if (options & ETO_GLYPH_INDEX)
		_ggo_format |= GGO_GLYPH_INDEX;

	if (!_setting_cache->hinting)
		_ggo_format |= GGO_UNHINTED;

	for (unsigned int i = 0; i < c; i++)
	{
		// we do not care about non-printable characters
		// solution for Windows Vista/7 Date
		if (!(options & ETO_GLYPH_INDEX) && iswcntrl(lpString[i]))
			continue;

		GLYPHMETRICS glyph_metrics;
		const FT_BitmapGlyph bmp_glyph = outline_to_bitmap(lpString[i], glyph_metrics);

		if (bmp_glyph != NULL)
		{
			POINT adjusted_pos = _cursor;
			adjusted_pos.x += bmp_glyph->left;

			_glyphs.push_back(bmp_glyph);
			_glyph_pos.push_back(adjusted_pos);
		}

		POINT glyph_advance = {0, glyph_metrics.gmCellIncY};
		const LONG char_advance = glyph_metrics.gmCellIncX + _char_extra;

		if (lpDx == NULL)
			glyph_advance.x = char_advance;
		else
		{
			glyph_advance.x = lpDx[i * advance_factor];

			// the last element in lpDx may be 0
			// in that case, we use the character advancement
			if (i == c - 1 && glyph_advance.x == 0)
				glyph_advance.x = char_advance;
		}

		_cursor.x += glyph_advance.x;
		_cursor.y += glyph_advance.y;
	}

	return true;
}

bool ggo_renderer::init()
{
	if (!gdimm_gdi_text::init())
		return false;

	/*
	glyph clazz is a private field, cannot be constructed through FreeType API
	instead, we load the glyph of the default character from the current font
	use the clazz for all subsequent FT_OutlineGlyph
	
	we only deal with fonts in outlines, the glyph clazz must be ft_outline_glyph_class
	glyph class is initialized only once
	*/

	FT_Error ft_error;

	if (_glyph_clazz == NULL)
	{
		const long font_id = font_man_instance.register_font(_hdc_text, metric_face_name(_outline_metrics));
		const FTC_FaceID ft_face_id = (FTC_FaceID) font_id;

		FT_Face font_face;
		ft_error = FTC_Manager_LookupFace(ft_cache_man, ft_face_id, &font_face);
		assert(ft_error == 0);

		FTC_ScalerRec cache_scale = {ft_face_id, 0, 0, 1, 0, 0};
		FT_Size font_size;
		ft_error = FTC_Manager_LookupSize(ft_cache_man, &cache_scale, &font_size);
		assert(ft_error == 0);

		ft_error = FT_Load_Char(font_face, _outline_metrics->otmTextMetrics.tmDefaultChar, FT_LOAD_NO_BITMAP);
		assert(ft_error == 0);

		FT_Glyph useless;
		FT_Get_Glyph(font_face->glyph, &useless);
		_glyph_clazz = useless->clazz;
		FT_Done_Glyph(useless);
	}

	return true;
}