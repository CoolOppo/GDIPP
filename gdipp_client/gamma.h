#pragma once

namespace gdipp
{

class gamma
{
public:
	~gamma();

	const BYTE *get_ramp(double gamma);

private:
	void init_ramp(double gamma);

	std::map<double, BYTE *> _gamma_ramps;
};

}
