#pragma once

#include "api.h"

namespace gdipp
{

class GDIPP_API lock
{
public:
	explicit lock(char *lock_sig);
	~lock();

private:
	CRITICAL_SECTION *_cs;
};

}
