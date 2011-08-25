// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C standard header files
#include <list>
#include <map>
#include <regex>
#include <sstream>

// FreeType
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_LCD_FILTER_H

// Murmurhash
#include <MurmurHash/MurmurHash3.h>

// pugixml
#include <pugixml/src/pugixml.hpp>
