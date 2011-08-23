#pragma once

namespace gdipp
{

class lock
{
public:
	explicit lock(char *lock_sig);
	~lock();

private:
	CRITICAL_SECTION *_cs;
};

}
