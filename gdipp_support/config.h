#pragma once

class config
{
public:
	virtual void load(const pugi::xml_node &root) = 0;
};
