// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

HMODULE hmodule = 0;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
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

extern "C" __declspec(dllexport) struct Plugin* create(struct Dock* dock)
{
  return (Plugin*) new Clock(*dock);
}

extern "C" __declspec(dllexport) int init(struct Plugin* plugin)
{
  if(!((Clock*)plugin)->init())
    return -1;
  return 0;
}

extern "C" __declspec(dllexport) void destroy(struct Plugin* plugin)
{
  delete (Clock*)plugin;
}

