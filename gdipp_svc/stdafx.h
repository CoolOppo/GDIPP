// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <comutil.h>
#include <objbase.h>
#include <shlwapi.h>
#include <tlhelp32.h>
#include <wbemidl.h>

// C RunTime Header Files
#include <assert.h>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <tchar.h>

// TODO: reference additional headers your program requires here
#include <easyhook.h>
#include <pugixml.hpp>