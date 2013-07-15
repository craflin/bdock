// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include <Psapi.h>
#include <shellapi.h>

#include <unordered_map>
#include <set>
#include <vector>

#include "../../Dock2.h"

typedef unsigned int uint;
typedef wchar_t wchar;

#include "launcher.h"
#include "wmi.h"
#include "systools.h"
