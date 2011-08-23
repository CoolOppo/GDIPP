#pragma once

#include "gdipp_support/config.h"
#include "gdipp_support/type_enum.h"

namespace gdipp
{

class client_config : public config
{
public:
	client_config();
	void load(const pugi::xml_node &root);

	PAINTER_TYPE painter;
};

}
