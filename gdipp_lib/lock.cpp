#include "stdafx.h"
#include "lock.h"

namespace gdipp
{

std::vector<CRITICAL_SECTION> lock::_locks;

void lock::initialize_locks()
{
	_locks.resize(LAST_LOCK_TYPE);
	for (int i = 0; i < LAST_LOCK_TYPE; ++i)
		InitializeCriticalSection(&_locks[i]);
}

void lock::destory_locks()
{
	const size_t lock_count = _locks.size();
	for (size_t i = 0; i < lock_count; ++i)
		DeleteCriticalSection(&_locks[i]);
}

lock::lock(LOCK_TYPE lock_type)
{
	_cs = &_locks[lock_type];
	EnterCriticalSection(_cs);
}

lock::~lock()
{
	LeaveCriticalSection(_cs);
}

}
