#include "stdafx.h"
#include "render_config.h"
#include "gdipp_lib/helper.h"

namespace gdipp
{

render_config::render_mode_config::render_mode_config()
	: mono(0),
	gray(1),
	subpixel(1),
	pixel_geometry(PIXEL_GEOMETRY_RGB),
	aliased_text(0)
{
}

render_config::render_config()
	: auto_hinting(1),
	embedded_bitmap(false),
	embolden(0),
	hinting(1),
	kerning(false),
	renderer(RENDERER_FREETYPE)
{
}

void render_config::load(const config_file &file)
{
	assert(false);
}

void render_config::parse(const void *root)
{
	if (root == NULL)
		return;

	const pugi::xml_node *root_node = reinterpret_cast<const pugi::xml_node *>(root);
	if (root_node->empty())
		return;

	pugi::xml_node node;
	
	node = root_node->select_single_node(L"auto_hinting/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), reinterpret_cast<short *>(&auto_hinting));

	node = root_node->select_single_node(L"embedded_bitmap/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &embedded_bitmap);

	node = root_node->select_single_node(L"embolden/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &embolden);

	node = root_node->select_single_node(L"hinting/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), reinterpret_cast<short *>(&hinting));

	node = root_node->select_single_node(L"kerning/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &kerning);

	node = root_node->select_single_node(L"render_mode/mono/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), reinterpret_cast<short *>(&render_mode.mono));

	node = root_node->select_single_node(L"render_mode/gray/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), reinterpret_cast<short *>(&render_mode.gray));

	node = root_node->select_single_node(L"render_mode/subpixel/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), reinterpret_cast<short *>(&render_mode.subpixel));

	node = root_node->select_single_node(L"render_mode/pixel_geometry/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), reinterpret_cast<short *>(&render_mode.pixel_geometry));

	node = root_node->select_single_node(L"render_mode/aliased_text/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &render_mode.aliased_text);

	node = root_node->select_single_node(L"renderer/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), reinterpret_cast<short *>(&renderer));
}

}
