#pragma once

#include "gdipp_lib/api.h"

class GDIPP_API config
{
public:
	virtual void load(const void *root) = 0;
};
