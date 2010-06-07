#pragma once

class gdimm_os2_metrics
{
	FT_Short _xAvgCharWidth;
	FT_UShort _usWeightClass;
	FT_UShort _fsSelection;

public:
	bool init(HDC hdc);

	FT_Short get_xAvgCharWidth();
	unsigned char get_weight_class();
	bool is_italic();
};