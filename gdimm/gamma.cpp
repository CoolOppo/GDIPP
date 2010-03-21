#include "stdafx.h"
#include "gamma.h"
#include "gdimm.h"

gdimm_gamma::~gdimm_gamma()
{
	for (map<double, BYTE*>::const_iterator iter = _gamma_ramps.begin(); iter != _gamma_ramps.end(); iter++)
		delete[] iter->second;
}

void gdimm_gamma::init_ramp(double gamma)
{
	BYTE *new_ramp = new BYTE[256];

	for (int i = 0; i < 256; i++)
		new_ramp[i] = (BYTE)(pow((double) i / 255, gamma) * 255);

	_gamma_ramps[gamma] = new_ramp;
}

const BYTE *gdimm_gamma::get_ramp(double gamma)
{
	map<double, BYTE*>::const_iterator iter = _gamma_ramps.find(gamma);

	// double-check interlock
	if (iter == _gamma_ramps.end())
	{
		critical_section interlock(CS_GAMMA);

		iter = _gamma_ramps.find(gamma);
		if (iter == _gamma_ramps.end())
			init_ramp(gamma);
	}

	return _gamma_ramps[gamma];
}