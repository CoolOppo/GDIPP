#include "stdafx.h"
#include "render_config_static.h"
#include "gdipp_lib/helper.h"

namespace gdipp
{

render_config_static::render_mode_static::render_mode_static()
	: mono(0),
	gray(1),
	subpixel(1),
	aliased_text(0)
{
}

render_config_static::render_config_static()
	: auto_hinting(1),
	embedded_bitmap(false),
	embolden(0),
	hinting(1),
	kerning(false),
	renderer(render_config::RENDERER_FREETYPE)
{
}

void render_config_static::parse(const config &cfg)
{
	auto_hinting = cfg.get_number(L"auto_hinting/text()", static_cast<unsigned int>(auto_hinting));
	embedded_bitmap = (!!cfg.get_number(L"embedded_bitmap/text()", static_cast<int>(embedded_bitmap)));
	embolden = cfg.get_number(L"embolden/text()", embolden);
	hinting = cfg.get_number(L"hinting/text()", static_cast<unsigned int>(hinting));
	kerning = (!!cfg.get_number(L"kerning/text()", static_cast<int>(kerning)));
	render_mode.mono = cfg.get_number(L"render_mode/mono/text()", static_cast<unsigned int>(render_mode.mono));
	render_mode.gray = cfg.get_number(L"render_mode/gray/text()", static_cast<unsigned int>(render_mode.gray));
	render_mode.subpixel = cfg.get_number(L"render_mode/subpixel/text()", static_cast<unsigned int>(render_mode.subpixel));
	render_mode.aliased_text = (!!cfg.get_number(L"render_mode/aliased_text/text()", static_cast<int>(render_mode.aliased_text)));
	renderer = static_cast<render_config::RENDERER_TYPE>(cfg.get_number(L"renderer/text()", static_cast<int>(renderer)));
}

}
