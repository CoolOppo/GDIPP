#include "stdafx.h"
#include "render_config_static.h"
#include "gdipp_lib/helper.h"

namespace gdipp
{

render_config_static::render_mode_static::render_mode_static()
	: mono(server_config::RENDER_MODE_MONO),
	gray(server_config::RENDER_MODE_GRAY),
	subpixel(server_config::RENDER_MODE_SUBPIXEL),
	aliased(server_config::RENDER_MODE_ALIASED)
{
}

render_config_static::render_config_static()
	: auto_hinting(server_config::AUTO_HINTING),
	embedded_bitmap(server_config::EMBEDDED_BITMAP),
	embolden(server_config::EMBOLDEN),
	hinting(server_config::HINTING),
	kerning(server_config::KERNING),
	renderer(server_config::RENDERER)
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
	render_mode.aliased = (!!cfg.get_number(L"render_mode/aliased_text/text()", static_cast<int>(render_mode.aliased)));
	renderer = static_cast<server_config::RENDERER_TYPE>(cfg.get_number(L"renderer/text()", static_cast<int>(renderer)));
}

}
