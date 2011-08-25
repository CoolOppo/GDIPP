#pragma once

namespace gdipp
{

enum GLYPH_CACHE_LEVEL
{
	/*
	every renderer supports one of the following glyph cache levels

	if NONE is supported, the renderer does not generate intermediate glyph or glyph run bitmap, and nothing can be cached
	all render process must be repeated disregarding request

	if GLYPH_RUN is supported, the renderer groups multiple glyphs into single cacheable bitmap
	such "glyph" does not correspond to any character

	if SINGLE_GLYPH is supported, single glyphs of each character are generated accordingly by the renderer
	each glyph can be shared across glyph runs
	*/

	NONE,
	GLYPH_RUN,
	SINGLE_GLYPH
};

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

enum RENDERER_TYPE
{
	RENDERER_CLEARTYPE = 0,
	RENDERER_FREETYPE = 10,
	RENDERER_GETGLYPHOUTLINE = 20,
	RENDERER_DIRECTWRITE = 30,
	RENDERER_WIC = 31
};

}
