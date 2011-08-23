#include "stdafx.h"
#include "render_config.h"
#include "gdipp_support/helper.h"

namespace gdipp
{

render_config::gamma_config::gamma_config()
	: red(1.0),
	green(1.0),
	blue(1.0)
{
}

render_config::render_mode_config::render_mode_config()
	: mono(0),
	gray(1),
	subpixel(1),
	pixel_geometry(PIXEL_GEOMETRY_RGB),
	aliased_text(0)
{
}

render_config::shadow_config::shadow_config()
	: offset_x(0),
	offset_y(0),
	alpha(0)
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

void render_config::load(const pugi::xml_node &root)
{
	if (root.empty())
		return;

	pugi::xml_node node;
	
	node = root.select_single_node(L"auto_hinting/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), reinterpret_cast<short *>(&auto_hinting));

	node = root.select_single_node(L"embedded_bitmap/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &embedded_bitmap);

	node = root.select_single_node(L"embolden/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &embolden);

	node = root.select_single_node(L"gamma/red/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &gamma.red);

	node = root.select_single_node(L"gamma/green/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &gamma.green);

	node = root.select_single_node(L"gamma/blue/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &gamma.blue);

	node = root.select_single_node(L"hinting/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), reinterpret_cast<short *>(&hinting));

	node = root.select_single_node(L"kerning/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &kerning);

	node = root.select_single_node(L"render_mode/mono/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), reinterpret_cast<short *>(&render_mode.mono));

	node = root.select_single_node(L"render_mode/gray/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), reinterpret_cast<short *>(&render_mode.gray));

	node = root.select_single_node(L"render_mode/subpixel/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), reinterpret_cast<short *>(&render_mode.subpixel));

	node = root.select_single_node(L"render_mode/pixel_geometry/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), reinterpret_cast<short *>(&render_mode.pixel_geometry));

	node = root.select_single_node(L"render_mode/aliased_text/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &render_mode.aliased_text);

	node = root.select_single_node(L"renderer/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), reinterpret_cast<short *>(&renderer));

	node = root.select_single_node(L"shadow/offset_x/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &shadow.offset_x);

	node = root.select_single_node(L"shadow/offset_y/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), &shadow.offset_y);

	node = root.select_single_node(L"shadow/alpha/text()").node();
	if (!node.empty())
		wcs_convert(node.value(), reinterpret_cast<short *>(&shadow.alpha));
}

}
