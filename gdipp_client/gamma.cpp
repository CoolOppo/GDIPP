#include "stdafx.h"
#include "gamma.h"
#include "gdipp_lib/scoped_rw_lock.h"

namespace gdipp
{

gamma::~gamma()
{
	for (std::map<double, BYTE *>::const_iterator iter = _gamma_ramps.begin(); iter != _gamma_ramps.end(); ++iter)
		delete[] iter->second;
}

const BYTE *gamma::get_ramp(double gamma)
{
	std::map<double, BYTE *>::const_iterator iter = _gamma_ramps.find(gamma);
	if (iter == _gamma_ramps.end())
	{
		// double-check lock
		const scoped_rw_lock lock_w(scoped_rw_lock::CLIENT_GAMMA, false);
		iter = _gamma_ramps.find(gamma);
		if (iter == _gamma_ramps.end())
			init_ramp(gamma);
	}

	return _gamma_ramps[gamma];
}

void gamma::init_ramp(double gamma)
{
	BYTE *new_ramp = new BYTE[256];
	const double gamma_inv = 1 / gamma;

	for (int i = 0; i < 256; ++i)
	{
		double a = pow(i / 255.0, gamma);
		new_ramp[i] = static_cast<BYTE>((pow(i / 255.0, gamma_inv) * 255));
	}

	_gamma_ramps[gamma] = new_ramp;
}

}
