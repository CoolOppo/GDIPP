#pragma once

#include "sink.h"

class svc_mon
{
	IWbemServices *_svc;
	IWbemObjectSink *_sink;

public:
	svc_mon();

	bool start_monitor();
	void stop_monitor();
};