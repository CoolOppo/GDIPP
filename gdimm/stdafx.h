// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <assert.h>
#include <shlwapi.h>
#include <mmsystem.h>

// C standard header files
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <fstream>
#include <cstdio>
#include <cmath>
#include <regex>

// TODO: reference additional headers your program requires here
#include "global.h"
#include "setting.h"

#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_CACHE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_LCD_FILTER_H
#include FT_TRUETYPE_TABLES_H

#include <easyhook.h>