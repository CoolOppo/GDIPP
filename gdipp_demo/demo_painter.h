#pragma once

namespace gdipp
{

#define GDIPP_DEMO_MAX_STR_LEN 100

class demo_painter
{
public:
	demo_painter();
	~demo_painter();

	void paint_demo(CPaintDC &dc);
	void stop_painting();

private:
	size_t _demo_font_count;
	const wchar_t **_demo_fonts;
	bool _random_text;
	int _total_cycles;

	// multi-thread related
	std::vector<HANDLE> _start_render_events;
	std::vector<HANDLE> _complete_render_events;
	std::vector<HANDLE> _render_threads;

	// result text related
	bool _result_prepared;
	HFONT _result_font;
	wchar_t _result_str[GDIPP_DEMO_MAX_STR_LEN];

	int _painted_cycles;
	DWORD _start_time;
};

}
