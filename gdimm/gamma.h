#pragma once

using namespace std;

class gdimm_gamma
{
	map<double, BYTE*> _gamma_ramps;

	void init_ramp(double gamma);

public:
	~gdimm_gamma();
	const BYTE *get_ramp(double gamma);
};