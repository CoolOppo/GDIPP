#pragma once

using namespace std;

class gdipp_font_store
{
	struct font_info
	{
		HFONT font_handle;
		vector<BYTE> metric_buf;
	};

	map<wstring, unsigned long> font_name_to_id;
	map<unsigned long, font_info> id_to_info;

	static OUTLINETEXTMETRICW *get_dc_metrics(HDC hdc, vector<BYTE> &metric_buf);
	
public:
	unsigned long register_font(const LOGFONTW *attr_buf, DWORD buf_size);
	DWORD get_font_raw_data(unsigned long font_id, DWORD table, DWORD offset, LPVOID data_buf, DWORD buf_size);
	DWORD get_font_metrics(unsigned long font_id, LPVOID metrics_buf, DWORD buf_size);
	DWORD get_glyph_indices(unsigned long font_id, const wchar_t *str, int count, unsigned short *gi);
};