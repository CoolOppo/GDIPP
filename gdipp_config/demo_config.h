#pragma once

#include "gdipp_config/config.h"

namespace gdipp
{

class GDIPP_API demo_config : public config
{
public:
	demo_config();
	void load(const void *root);

	unsigned int cycles;
	unsigned char threads;
	bool random_text;
	std::vector<std::wstring> fonts;
};

}
