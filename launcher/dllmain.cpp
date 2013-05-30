// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

HMODULE hmodule = 0;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{  
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
    hmodule = hModule;
    break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
    break;
	case DLL_PROCESS_DETACH:
    hmodule = 0;
		break;
	}
	return TRUE;
}

#define LAUNCHER_API __declspec(dllexport)

extern "C" LAUNCHER_API struct Plugin* create(void)
{
  return new Launcher;
}

extern "C" LAUNCHER_API int init(struct Plugin* plugin)
{
  if(!((Launcher*)plugin)->init())
    return -1;
  return 0;
}

extern "C" LAUNCHER_API void destroy(struct Plugin* plugin)
{
	delete (Launcher*)plugin;
}

