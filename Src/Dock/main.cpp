// bdock.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#if defined(_DEBUG) && defined(DEBUG_CONSOLE)

#pragma comment(linker, "/subsystem:console")

// Console main function, this code is from WinMainCRTStartup()
int _tmain( DWORD, TCHAR**, TCHAR** )
{
  // set the event handler
  struct Console
  {
    static BOOL WINAPI CtrlHandler(DWORD dwCtrlType)
    {
      switch(dwCtrlType)
      {
      case CTRL_C_EVENT:
      case CTRL_BREAK_EVENT:
      case CTRL_CLOSE_EVENT:
      case CTRL_LOGOFF_EVENT:
      case CTRL_SHUTDOWN_EVENT:
        ExitProcess(0);
        break;
      }
      return TRUE;
    }
  };
  SetConsoleCtrlHandler(Console::CtrlHandler, TRUE);

  // get command line
  LPTSTR lpszCommandLine = ::GetCommandLine();
  if(lpszCommandLine == NULL)
    return -1;
  if(*lpszCommandLine == _T('\"'))
  {
    do
    {
      lpszCommandLine = ::CharNext(lpszCommandLine);
    } while(*lpszCommandLine != _T('\"') && *lpszCommandLine);
    if(*lpszCommandLine == _T('\"'))
      lpszCommandLine = ::CharNext(lpszCommandLine);
  }
  else
  {
    while(*lpszCommandLine != _T(' ') && *lpszCommandLine)
      lpszCommandLine = ::CharNext(lpszCommandLine);
  }
  while(*lpszCommandLine && *lpszCommandLine <= _T(' '))
    lpszCommandLine = ::CharNext(lpszCommandLine);
 
  // call win32 main function
  STARTUPINFO StartupInfo;
  StartupInfo.dwFlags = 0;
  ::GetStartupInfo(&StartupInfo);
  return _tWinMain(::GetModuleHandle(NULL), NULL, lpszCommandLine, (StartupInfo.dwFlags & STARTF_USESHOWWINDOW) ? StartupInfo.wShowWindow : SW_SHOWDEFAULT);
}

#endif

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  // Unless there is a debugger attached, create a copy of the executabe and launch the copy.
  // That way debugging is a lot easier since we can recompile and restart the process without having to close it in advanced.
#ifdef _DEBUG
  {
    HWND hwndDock = FindWindow(_T("BDOCK"), NULL);
    if(hwndDock != NULL)
      SendMessage(hwndDock, WM_CLOSE, 0, 0);
    BOOL isDebuggerPresent = FALSE;
    if(!CheckRemoteDebuggerPresent(GetCurrentProcess(), &isDebuggerPresent) || !isDebuggerPresent)
    {
      TCHAR moduleFilenameBuf[256];
      DWORD moduleFilenameLen = GetModuleFileName(NULL, moduleFilenameBuf, sizeof(moduleFilenameBuf));
      String moduleFilename(moduleFilenameBuf, moduleFilenameLen);
      String filename(moduleFilename);
      const tchar_t* lastDelimiter = filename.findLastOf(L"\\/");
      if(lastDelimiter)
        filename = filename.substr(lastDelimiter + 1 - (const tchar_t*)filename);
      if(filename == L"BDock.exe")
      {
        String copyFilename = moduleFilename + L"-copy";
        DWORD firstTry = GetTickCount();
        for(;;)
        {
          if(CopyFile(moduleFilename, copyFilename, FALSE))
            break;
          Sleep(10); // it may take awhile for the running process to close
          if(GetTickCount() - firstTry > 1000)
            goto copyTimeout; // somethings is wrong, skip this debug code
        }
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        CreateProcess(copyFilename, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
        return EXIT_SUCCESS;
      copyTimeout: ;
      }
    }
  }
#endif

  HANDLE hMutex = CreateMutex(NULL, TRUE, _T("BDock"));
  if(!hMutex)
    return -1;
#ifndef _DEBUG
  if(GetLastError() == ERROR_ALREADY_EXISTS)
  {
    CloseHandle(hMutex);
    return -1;
  }
#endif

  WinAPI::Application application(hInstance, ICC_LINK_CLASS);

  // load storage
  Storage storage;
  {
    WCHAR path[MAX_PATH];
    if(WinAPI::Shell::getFolderPath(CSIDL_APPDATA | CSIDL_FLAG_CREATE, path, MAX_PATH)) 
    {
      String storageFile(path, String::length(path));
      storageFile += L"/BDock";
      if(GetFileAttributes(storageFile) == INVALID_FILE_ATTRIBUTES)
        CreateDirectory(storageFile, 0);
      storageFile += L"/config.bd";
      storage.load(storageFile);
    }
  }
  if(storage.getNumSectionCount() == 0)
  {
    storage.setNumSectionCount(1);

    // launcher dock
    storage.enterNumSection(0);
    WinAPI::String dockName(IDS_MAIN_DOCK);
    storage.setStr(_T("name"), String(dockName, String::length(dockName)));

    storage.setNumSectionCount(2);
    storage.enterNumSection(0);
    storage.setStr(_T("name"), _T("launcher"));
    storage.leave();
    storage.enterNumSection(1);
    storage.setStr(_T("name"), _T("hidetaskbar"));
    storage.leave();
    storage.leave();

    /*
    // system tray dock
    storage.enterNumSection(1);
    storage.setStr("name", L"SystemTrayDock", 0); // dock name, TODO: load name from string table
    storage.setInt("alignment", Settings::right);
    storage.setInt("rightMargin", 0);
    storage.setNumSectionCount(2);
    storage.enterNumSection(0);
    storage.setStr("name", L"systemtray", 0);
    storage.leave();
    storage.enterNumSection(1);
    storage.setStr("name", L"clock", 0);
    storage.leave();
    storage.leave();
    */
  }

  // create docks
  Array<Dock*> docks(storage.getNumSectionCount());
  for(int i = 0, count = storage.getNumSectionCount(); i < count; ++i)
  {
    Dock* dock = new Dock(storage, *storage.getNumSection(i));
    if(!dock->create())
      delete dock;
    else
      docks.append(dock);
  }

  // Main message loop:
  UINT exitCode = application.run();

  // delete docks
  for(Array<Dock*>::Iterator i = docks.begin(), end = docks.end(); i != end; ++i)
    delete *i;
  docks.clear();

  CloseHandle(hMutex);
  return (int) exitCode;
}
