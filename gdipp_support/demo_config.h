#pragma once

#include "gdipp_support/config.h"

namespace gdipp
{

class demo_config : public config
{
public:
	demo_config();
	void load(const pugi::xml_node &root);

	unsigned int cycles;
	unsigned char threads;
	bool random_text;
	std::vector<std::wstring> fonts;
};

}
