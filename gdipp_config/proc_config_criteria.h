#pragma once

namespace gdipp
{

class proc_config_criteria
{
public:
	proc_config_criteria(const void *node);
	bool is_satisfied(const wchar_t *proc_name) const;

private:
	std::wstring _proc_name;
};

}
