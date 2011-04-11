#pragma once

#include "MurmurHash3.h"

class uint128_t
{
	uint64_t data[2];

public:
	bool operator <(const uint128_t &i) const
	{
		return ((this->data[0] < i.data[0]) ? true : (this->data[1] < i.data[1]));
	}
};