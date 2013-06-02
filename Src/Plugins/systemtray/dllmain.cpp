// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

HMODULE hmodule = 0;

HWND hookedWnd = 0;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{  
  switch (ul_reason_for_call)
  {
  case DLL_PROCESS_ATTACH:
    hmodule = hModule;
    {
      wchar filename[MAX_PATH];
      GetModuleFileName(NULL, filename, MAX_PATH);
      wchar* name = wcsrchr(filename, L'\\');
      if(name && !_wcsicmp(name + 1, L"explorer.exe"))
      { // thats it! we, are running in the address space of explorer.exe        
        hookedWnd = FindWindow(L"Shell_TrayWnd", 0);
        if(hookedWnd)
        {          
          TrayWndProc = (WNDPROC)GetWindowLongPtr(hookedWnd, GWLP_WNDPROC);
          SetWindowLongPtr(hookedWnd, GWLP_WNDPROC, (LONG_PTR)TrayWndProcHook);
        }
      }
    }
    break;
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
    break;
  case DLL_PROCESS_DETACH:
    hmodule = 0;
    if(hookedWnd)
    {
      if(TrayWndProc)
      {
        SetWindowLongPtr(hookedWnd, GWLP_WNDPROC, (LONG_PTR)TrayWndProc);
        TrayWndProc = 0;
      }
      hookedWnd = 0;
    }
    break;
  }
  return TRUE;
}

extern "C" __declspec(dllexport) struct Plugin* create(struct Dock* dock)
{
  return (Plugin*) new SystemTray(*dock);
}

extern "C" __declspec(dllexport) int init(struct Plugin* plugin)
{
  if(!((SystemTray*)plugin)->init())
    return -1;
  return 0;
}

extern "C" __declspec(dllexport) void destroy(struct Plugin* plugin)
{
  delete (SystemTray*)plugin;
}

