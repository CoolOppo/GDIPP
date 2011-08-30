#pragma once

#include "gdipp_config/config.h"

namespace gdipp
{

class GDIPP_API exclude_config : public config
{
public:
	exclude_config();
	void load(const config_file &file);
	bool is_process_excluded(const wchar_t *proc_name) const;

	std::vector<std::wstring> processes;

private:
	void parse(const void *root);
};

}
