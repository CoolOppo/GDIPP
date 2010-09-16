#pragma once

using namespace std;

void outline_ggo_to_ft(DWORD ggo_outline_buf_len, const BYTE *ggo_outline_buf, vector<FT_Vector> &curve_points, vector<char> &curve_tags, vector<short> &contour_indices);
DWORD get_ggo_outline_size(const FT_Outline &ft_outline);
DWORD outline_ft_to_ggo(const FT_Outline &ft_outline, LPVOID ggo_outline_buf);