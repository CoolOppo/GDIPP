// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <rpc.h>
#include <wtsapi32.h>

// C RunTime Header Files
#include <cmath>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <fassert.h>

// 3rd-party dependency
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_CACHE_H
#include FT_LCD_FILTER_H
#include FT_OUTLINE_H

#include <sqlite3.h>