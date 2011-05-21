#pragma once

class gdimm_os2_metrics
{
public:
	bool init(HDC hdc);
	bool init(long font_id);

	FT_Short get_xAvgCharWidth() const;
	char get_weight_class() const;
	FT_UShort get_usWidthClass() const;
	bool is_italic() const;

private:
	FT_Short _xAvgCharWidth;
	FT_UShort _usWeightClass;
	FT_UShort _usWidthClass;
	FT_UShort _fsSelection;
};
