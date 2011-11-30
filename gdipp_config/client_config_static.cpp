#include "stdafx.h"
#include "client_config_static.h"

namespace gdipp
{

client_config_static::gamma_static::gamma_static()
	: red(client_config::GAMMA_RED),
	green(client_config::GAMMA_GREEN),
	blue(client_config::GAMMA_BLUE)
{
}

client_config_static::shadow_static::shadow_static()
	: offset_x(client_config::SHADOW_OFFSET_X),
	offset_y(client_config::SHADOW_OFFSET_Y),
	alpha(client_config::SHADOW_ALPHA)
{
}

client_config_static::client_config_static()
	: painter(client_config::PAINTER),
	pixel_geometry(client_config::PIXEL_GEOMETRY)
{
}

void client_config_static::parse(const config &cfg)
{
	gamma.red = cfg.get_number(L"/gdipp/paint/gamma/red/text()", gamma.red);
	gamma.green = cfg.get_number(L"/gdipp/paint/gamma/green/text()", gamma.green);
	gamma.blue = cfg.get_number(L"/gdipp/paint/gamma/blue/text()", gamma.blue);
	painter = static_cast<client_config::PAINTER_TYPE>(cfg.get_number(L"/gdipp/paint/painter/text()", static_cast<int>(painter)));
	pixel_geometry = static_cast<client_config::PIXEL_GEOMETRY_TYPE>(cfg.get_number(L"/gdipp/paint/pixel_geometry/text()", static_cast<int>(pixel_geometry)));
	shadow.offset_x = cfg.get_number(L"/gdipp/paint/shadow/offset_x/text()", shadow.offset_x);
	shadow.offset_y = cfg.get_number(L"/gdipp/paint/shadow/offset_y/text()", shadow.offset_y);
	shadow.alpha = cfg.get_number(L"/gdipp/paint/shadow/alpha/text()", static_cast<int>(shadow.alpha));
}

}
