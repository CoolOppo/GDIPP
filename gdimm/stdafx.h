// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <atlbase.h>
#include <mmsystem.h>
#include <psapi.h>

// C standard header files
#include <cassert>
#include <cmath>
#include <ctime>
#include <list>
#include <map>
#include <process.h>
#include <set>
#include <sstream>
#include <string>
#include <vector>

// 3rd-party dependency
#include "ft2build.h"
#include FT_BBOX_H
#include FT_CACHE_H
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_LCD_FILTER_H
#include FT_OUTLINE_H

#pragma pack(1)
#include FT_TRUETYPE_TABLES_H
#pragma pack()

#include <easyhook.h>

#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>