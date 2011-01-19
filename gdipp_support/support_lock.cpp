#include <map>
#include "support_lock.h"
#include "MurmurHash2.h"

using namespace std;

map<unsigned int, CRITICAL_SECTION *> locks;

gdipp_lock::gdipp_lock(char *lock_sig)
{
	const unsigned int lock_key = MurmurHash2(lock_sig, strlen(lock_sig), 0);
	
	map<unsigned int, CRITICAL_SECTION *>::const_iterator lock_iter = locks.find(lock_key);

	if (lock_iter == locks.end())
	{
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