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
	gdimm_lock(LOCK_TYPE type);

	~gdimm_lock();

	static void initialize();

	static void release();
};