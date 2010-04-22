#pragma once

#include "sink.h"

class svc_mon
{
	IWbemServices *_svc;
	IWbemObjectSink *_sink;

public:
	svc_mon()
	: 
	_svc(NULL),
	_sink(NULL)
	{};

	bool start_monitor();
	void stop_monitor();
};