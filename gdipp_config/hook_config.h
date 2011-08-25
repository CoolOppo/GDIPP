#pragma once

#include "gdipp_config/config.h"

namespace gdipp
{

class GDIPP_API hook_config : public config
{
public:
	hook_config();
	void load(const void *root);

	bool proc_32_bit;
	bool proc_64_bit;
	std::vector<std::wstring> excludes;
};

}
