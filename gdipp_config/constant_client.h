#pragma once

namespace gdipp
{
namespace client_config
{

enum PIXEL_GEOMETRY_TYPE
{
	PIXEL_GEOMETRY_RGB,
	PIXEL_GEOMETRY_BGR
};

enum PAINTER_TYPE
{
	PAINTER_GDI = 10,
	PAINTER_D2D = 20
};

static const double GAMMA_RED = 1.0;
static const double GAMMA_GREEN = 1.0;
static const double GAMMA_BLUE = 1.0;
static const PAINTER_TYPE PAINTER = PAINTER_GDI;
static const PIXEL_GEOMETRY_TYPE PIXEL_GEOMETRY = PIXEL_GEOMETRY_RGB;
static const int SHADOW_OFFSET_X = 0;
static const int SHADOW_OFFSET_Y = 0;
static const unsigned char SHADOW_ALPHA = 0;

}
}
