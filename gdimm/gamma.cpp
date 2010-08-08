#include "stdafx.h"
#include "gamma.h"
#include "lock.h"

gdimm_gamma::~gdimm_gamma()
{
	for (map<double, BYTE *>::const_iterator iter = _gamma_ramps.begin(); iter != _gamma_ramps.end(); iter++)
		delete[] iter->second;
}

void gdimm_gamma::init_ramp(double gamma)
{
	BYTE *new_ramp = new BYTE[256];

	for (int i = 0; i < 256; i++)
		new_ramp[i] = static_cast<BYTE>((pow(i / 255.0, gamma) * 255));

	_gamma_ramps[gamma] = new_ramp;
}

const BYTE *gdimm_gamma::get_ramp(double gamma)
{
	map<double, BYTE *>::const_iterator iter = _gamma_ramps.find(gamma);

	if (iter == _gamma_ramps.end())
	{
		// double-check lock
		gdimm_lock lock(LOCK_GAMMA);

		iter = _gamma_ramps.find(gamma);
		if (iter == _gamma_ramps.end())
			init_ramp(gamma);
	}

	return _gamma_ramps[gamma];
}