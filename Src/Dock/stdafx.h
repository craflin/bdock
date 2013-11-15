// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include "WinAPI.h"

// C RunTime Header Files
#include <cstdlib>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

//#include <vector>
//#include <string>
//#include <unordered_map>
//#include <unordered_set>
//#include "list_set.h"

#include "resource.h"

namespace API
{
#include "../Dock2.h"
};

//typedef unsigned int uint;
//typedef wchar_t wchar;

#include <nstd/Debug.h>
#include <nstd/Memory.h>
#include <nstd/Array.h>
#include <nstd/HashMap.h>
#include <nstd/HashSet.h>
#include <nstd/String.h>
#include <nstd/File.h>

#include "Storage.h"
#include "Settings.h"
#include "Skin.h"
#include "Icon.h"
#include "Timer.h"
#include "Plugin.h"
#include "Dock.h"
#include "AboutDlg.h"
#include "SettingsDlg.h"


