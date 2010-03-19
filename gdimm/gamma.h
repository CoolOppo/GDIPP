#pragma once

#include <global.h>

class _gdimm_gamma
{
	map<double, BYTE*> _gamma_ramps;

	void init_ramp(double gamma);

public:
	~_gdimm_gamma();
	const BYTE *get_ramp(double gamma);
};

typedef singleton<_gdimm_gamma> gdimm_gamma;