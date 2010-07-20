#pragma once

#include "type_enum.h"

class gdimm_lock
{
	static CRITICAL_SECTION _cs[_LOCK_TYPE_COUNT_];
	LOCK_TYPE _type;

public:
	gdimm_lock(LOCK_TYPE type);

	~gdimm_lock();

	static void initialize();

	static void release();
};