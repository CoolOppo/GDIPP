#pragma once

enum LOCK_TYPE
{
	LOCK_GAMMA,
	LOCK_GLYPH_CACHE,
	LOCK_HOOK,
	LOCK_LINKED_FONT,
	LOCK_REG_FONT,
	LOCK_SETTING_CACHE,
	LOCK_TEXT,
	LOCK_DEBUG,

	// count of lock types
	// never use this type
	_LOCK_TYPE_COUNT_
};

class gdimm_lock
{
	static CRITICAL_SECTION _cs[_LOCK_TYPE_COUNT_];
	LOCK_TYPE _type;

public:
	gdimm_lock(LOCK_TYPE type)
	: _type(type)
	{
		EnterCriticalSection(&_cs[type]);
	}

	~gdimm_lock()
	{
		LeaveCriticalSection(&_cs[_type]);
	}

	static void initialize()
	{
		for (int i = 0; i < _LOCK_TYPE_COUNT_; i++)
			InitializeCriticalSection(&_cs[i]);
	}

	static void release()
	{
		for (int i = 0; i < _LOCK_TYPE_COUNT_; i++)
			DeleteCriticalSection(&_cs[i]);
	}
};