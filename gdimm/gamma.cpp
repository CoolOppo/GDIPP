#include "stdafx.h"
#include "gamma.h"
#include <cmath>

_gdimm_gamma::~_gdimm_gamma()
{
	for (map<double, BYTE*>::const_iterator iter = _gamma_ramps.begin(); iter != _gamma_ramps.end(); iter++)
		delete[] iter->second;
}

void _gdimm_gamma::init_ramp(double gamma)
{
	BYTE *new_ramp = new BYTE[256];

	for (int i = 0; i < 256; i++)
		new_ramp[i] = (BYTE)(pow((double) i / 255, gamma) * 255);

	_gamma_ramps[gamma] = new_ramp;
}

const BYTE *_gdimm_gamma::get_ramp(double gamma)
{
	map<double, BYTE*>::const_iterator iter = _gamma_ramps.find(gamma);

	if (iter == _gamma_ramps.end())
		init_ramp(gamma);

	return _gamma_ramps[gamma];
}