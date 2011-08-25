#pragma once

namespace gdipp
{

class font_config_criteria
{
public:
	font_config_criteria(const void *node);
	bool is_satisfied(bool bold, bool italic, LONG height, const wchar_t *font_name) const;

private:
	char _bold;
	char _italic;
	LONG _max_height;
	std::wstring _font_name;
};

}
