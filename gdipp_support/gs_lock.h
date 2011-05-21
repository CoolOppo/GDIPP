#pragma once

#include <windows.h>

class gdipp_lock
{
public:
	explicit gdipp_lock(char *lock_sig);
	~gdipp_lock();

private:
	CRITICAL_SECTION *_cs;
};
