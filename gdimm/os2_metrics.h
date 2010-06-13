#pragma once

class gdimm_os2_metrics
{
	FT_Short _xAvgCharWidth;
	FT_UShort _usWeightClass;
	FT_UShort _usWidthClass;
	FT_UShort _fsSelection;

public:
	bool init(HDC hdc);

	FT_Short get_xAvgCharWidth() const;
	unsigned char get_weight_class() const;
	FT_UShort get_usWidthClass() const;
	bool is_italic() const;
};