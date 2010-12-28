#pragma once

class gdipp_lock
{
	CRITICAL_SECTION *_cs;

public:
	gdipp_lock(char *lock_sig);
	~gdipp_lock();
};