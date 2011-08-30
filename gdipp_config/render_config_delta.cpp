#include "stdafx.h"
#include "render_config_delta.h"

namespace gdipp
{

void render_config_delta::load(const config_file &file)
{
	assert(false);
}

void render_config_delta::parse(const void *root)
{
	if (root == NULL)
		return;

	const pugi::xml_node *root_node = reinterpret_cast<const pugi::xml_node *>(root);
	if (root_node->empty())
		return;

	// TODO:
	for (pugi::xml_node::iterator child_iter = root_node->begin(); child_iter != root_node->end(); ++child_iter)
	{
		
	}
}

}
