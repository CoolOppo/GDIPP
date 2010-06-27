#pragma once

#include "gdipp_common.h"

class gdipp_injector
{
	wchar_t _gdimm_path[MAX_PATH];

public:
	gdipp_injector();
	NTSTATUS inject_process(ULONG process_id, ULONG thread_id);
};