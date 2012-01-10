#include "stdafx.h"
#include "scoped_rw_lock.h"

namespace gdipp
{

std::vector<SRWLOCK> scoped_rw_lock::_srws;

void scoped_rw_lock::initialize()
{
	_srws.resize(LAST_MONITOR_LOCATION);
	for (int i = 0; i < LAST_MONITOR_LOCATION; ++i)
		InitializeSRWLock(&_srws[i]);
}

scoped_rw_lock::scoped_rw_lock(MONITOR_LOCATION cs_location, bool is_shared)
{
	_curr_srw = &_srws[cs_location];
	_is_shared = is_shared;

	if (is_shared)
		AcquireSRWLockShared(_curr_srw);
	else
		AcquireSRWLockExclusive(_curr_srw);
}

scoped_rw_lock::~scoped_rw_lock()
{
	if (_is_shared)
		ReleaseSRWLockShared(_curr_srw);
	else
		ReleaseSRWLockExclusive(_curr_srw);
}

}
