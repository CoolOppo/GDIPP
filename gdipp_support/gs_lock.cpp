#include "gs_lock.h"
#include <map>
#include <MurmurHash3.h>

using std::map;

map<unsigned int, CRITICAL_SECTION *> locks;

gdipp_lock::gdipp_lock(char *lock_sig)
{
	uint32_t lock_key;
	MurmurHash3_x86_32(lock_sig, static_cast<int>(strlen(lock_sig)), 0, &lock_key);

	map<unsigned int, CRITICAL_SECTION *>::const_iterator lock_iter = locks.find(lock_key);

	if (lock_iter == locks.end())
	{
		_cs = new CRITICAL_SECTION;
		InitializeCriticalSection(_cs);
		locks[lock_key] = _cs;
	}
	else
		_cs = lock_iter->second;

	EnterCriticalSection(_cs);
}

gdipp_lock::~gdipp_lock()
{
	LeaveCriticalSection(_cs);
}
