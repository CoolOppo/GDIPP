#pragma once

#include "gdipp_support/config.h"

namespace gdipp
{

class hook_config : public config
{
public:
	hook_config();
	void load(const pugi::xml_node &root);

	bool proc_32_bit;
	bool proc_64_bit;
	std::vector<std::wstring> excludes;
};

}
