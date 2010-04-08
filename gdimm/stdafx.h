// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <mmsystem.h>
#include <psapi.h>

// C standard header files
#include <assert.h>
#include <cmath>
#include <map>
#include <process.h>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>

// 3rd-party dependency libraries
#include "ft2build.h"
#include FT_CACHE_H
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_LCD_FILTER_H
#include FT_TRUETYPE_TABLES_H

#include <easyhook.h>