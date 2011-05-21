#pragma once

#include "MurmurHash3.h"

class uint128_t
{
public:
	bool operator <(const uint128_t &i) const
	{
		return (this->data[0] < i.data[0]) ? true : this->data[1] < i.data[1];
	}

private:
	uint64_t data[2];
};
