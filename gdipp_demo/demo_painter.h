#pragma once

#include <vector>

using std::vector;

#define GDIPP_DEMO_MAX_STR_LEN 100

class gdipp_demo_painter
{
public:
	gdipp_demo_painter();
	~gdipp_demo_painter();

	void paint_demo(CPaintDC &dc);
	void stop_painting();

private:
	int _total_count;

	// multi-thread related
	vector<HANDLE> _start_render_events;
	vector<HANDLE> _complete_render_events;
	vector<HANDLE> _render_threads;

	// result text related
	bool _result_prepared;
	HFONT _result_font;
	wchar_t _result_str[GDIPP_DEMO_MAX_STR_LEN];

	int _painted_count;
	DWORD _start_time;
};
