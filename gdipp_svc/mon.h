#pragma once

#include "global.h"
#include <wbemidl.h>

class _svc_mon
{
	IWbemLocator *_loc;
	IWbemServices *_svc;
	IUnsecuredApartment* _unsec_app;
	IWbemObjectSink *_sink;
	IUnknown* _stub_unk;
	IWbemObjectSink* _stub_sink;

	void release_all();

public:
	bool start_monitor();
	void stop_monitor();
};

typedef singleton<_svc_mon> svc_mon;