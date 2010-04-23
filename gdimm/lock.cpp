#include "stdafx.h"
#include "lock.h"

CRITICAL_SECTION gdimm_lock::_cs[_LOCK_TYPE_COUNT_];

gdimm_lock::gdimm_lock(LOCK_TYPE type)
:
_type(type)
{
	EnterCriticalSection(&_cs[type]);
}

gdimm_lock::~gdimm_lock()
{
	LeaveCriticalSection(&_cs[_type]);
}

void gdimm_lock::initialize()
{
	for (int i = 0; i < _LOCK_TYPE_COUNT_; i++)
		InitializeCriticalSection(&_cs[i]);
}

void gdimm_lock::release()
{
	for (int i = 0; i < _LOCK_TYPE_COUNT_; i++)
		DeleteCriticalSection(&_cs[i]);
}