#include "stdafx.h"
#include "outline.h"
#include "helper_func.h"

void outline_ggo_to_ft(DWORD ggo_outline_buf_len, const BYTE *ggo_outline_buf, vector<FT_Vector> &curve_points, vector<char> &curve_tags, vector<short> &contour_indices)
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

			for (int j = 0; j < curve->cpfx; j++)
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

DWORD get_ggo_outline_size(const FT_Outline &ft_outline)
{
	short curve_count = 0;
	short contour_start = 0;

	for (short i = 0; i < ft_outline.n_contours; i++)
	{
		bool new_curve = true;
		char curve_tag;

		for (short j = contour_start; j <= ft_outline.contours[i]; j++)
		{
			// a curve consists of consecutive points with the same tag, except the starting and ending point (always on the curve)

			if (new_curve)
			{
				curve_count += 1;
				new_curve = false;
				curve_tag = -1;
			}
			else
			{
				if (curve_tag == -1)
					curve_tag = ft_outline.tags[j];
				else if (curve_tag != ft_outline.tags[j])
				{
					new_curve = true;
					j -= 1;
				}
			}
		}

		contour_start = ft_outline.contours[i] + 1;
	}
	
	return ft_outline.n_contours * sizeof(TTPOLYGONHEADER) + curve_count * sizeof(TTPOLYCURVE) + (ft_outline.n_points - ft_outline.n_contours - curve_count) * sizeof(POINTFX);
}

DWORD outline_ft_to_ggo(const FT_Outline &ft_outline, LPVOID ggo_outline_buf)
{
	BYTE *byte_outline_buf = reinterpret_cast<BYTE *>(ggo_outline_buf);
	DWORD buf_offset = 0;
	short contour_start = 0;

	for (short i = 0; i < ft_outline.n_contours; i++)
	{
		TTPOLYGONHEADER *curr_contour_header = reinterpret_cast<TTPOLYGONHEADER *>(byte_outline_buf + buf_offset);
		curr_contour_header->dwType = TT_POLYGON_TYPE;
		curr_contour_header->pfxStart.x = fixed_from_26dot6(ft_outline.points[contour_start].x);
		curr_contour_header->pfxStart.y = fixed_from_26dot6(ft_outline.points[contour_start].y);
		buf_offset += sizeof(TTPOLYGONHEADER);

		bool new_curve = true;
		TTPOLYCURVE *curr_curve;
		POINTFX *curr_point;
		char curve_tag;

		for (short j = contour_start; j <= ft_outline.contours[i]; j++)
		{
			if (new_curve)
			{
				curr_curve = reinterpret_cast<TTPOLYCURVE *>(byte_outline_buf + buf_offset);
				buf_offset += sizeof(curr_curve->wType) + sizeof(curr_curve->cpfx);
				curr_point = reinterpret_cast<POINTFX *>(byte_outline_buf + buf_offset);
				curr_curve->cpfx = 0;
				new_curve = false;
				curve_tag = -1;
			}
			else
			{
				curr_point->x = fixed_from_26dot6(ft_outline.points[j].x);
				curr_point->y = fixed_from_26dot6(ft_outline.points[j].y);

				if (curve_tag == -1)
				{
					switch (ft_outline.tags[j])
					{
					case FT_CURVE_TAG_ON:
						curr_curve->wType = TT_PRIM_LINE;
						break;
					case FT_CURVE_TAG_CONIC:
						curr_curve->wType = TT_PRIM_QSPLINE;
						break;
					case FT_CURVE_TAG_CUBIC:
						curr_curve->wType = TT_PRIM_CSPLINE;
						break;
					}

					curve_tag = ft_outline.tags[j];
				}
				else if (curve_tag != ft_outline.tags[j])
				{
					new_curve = true;
					j -= 1;
				}

				buf_offset += sizeof(POINTFX);
				curr_point += 1;
				curr_curve->cpfx += 1;
			}
		}

		assert(buf_offset == reinterpret_cast<BYTE *>(curr_point) - reinterpret_cast<BYTE *>(curr_contour_header));
		curr_contour_header->cb = buf_offset;
		contour_start = ft_outline.contours[i] + 1;
	}

	return buf_offset;
}