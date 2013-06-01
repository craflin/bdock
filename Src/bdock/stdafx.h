// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include <shlobj.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <set> // TODO: repalce all sets with hash_sets
#include <map> // TODO: replace all maps with hash_maps
#include <vector>
#include <string>
#include <hash_map>
#include <hash_set>

namespace API
{
#include "bdock.h"
};

typedef unsigned int uint;
typedef wchar_t wchar;

#include "assert.h"
#include "storage.h"
#include "settings.h"
#include "skin.h"
#include "icon.h"
#include "timer.h"
#include "plugin.h"
#include "dock.h"



