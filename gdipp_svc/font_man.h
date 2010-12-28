#pragma once

using namespace std;

class gdipp_font_man
{
	struct font_info
	{
		HFONT font_handle;
		vector<BYTE> metric_buf;
	};

	map<wstring, unsigned long> font_name_to_id;
	map<unsigned long, font_info> id_to_info;

	static OUTLINETEXTMETRICW *get_dc_metrics(HDC hdc, vector<BYTE> &metric_buf);
	static unsigned long stream_io(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count);
	static void stream_close(FT_Stream stream);
	
public:
	unsigned long register_font(const LOGFONTW *attr_buf, DWORD buf_size);
	DWORD get_font_raw_data(unsigned long font_id, DWORD table, DWORD offset, LPVOID data_buf, DWORD buf_size);
	DWORD get_font_metrics(unsigned long font_id, LPVOID metrics_buf, DWORD buf_size);
	DWORD get_glyph_indices(unsigned long font_id, const wchar_t *str, int count, unsigned short *gi);

	FT_Stream lookup_stream(long font_id);
	ULONG lookup_face_index(long font_id);
};