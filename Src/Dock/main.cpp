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


//LRESULT CALLBACK  wndProc(HWND, UINT, WPARAM, LPARAM);
//INT_PTR CALLBACK  aboutDlgProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  WinAPI::Application application(hInstance, ICC_LINK_CLASS);

  // load storage
  std::wstring storageFile;
  Storage storage;
  {
    WCHAR path[MAX_PATH];
    if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, path))) 
    {
      storageFile = path;
      storageFile += L"/BDock";
      if(GetFileAttributes(storageFile.c_str()) == INVALID_FILE_ATTRIBUTES)
        CreateDirectory(storageFile.c_str(), 0);
      storageFile += L"/config.bd";
      storage.load(storageFile.c_str());
    }
  }
  if(storage.getNumSectionCount() == 0)
  {
    storage.setNumSectionCount(1);

    // launcher dock
    storage.enterNumSection(0);
    storage.setNumSectionCount(2);
    storage.enterNumSection(0);
    storage.setStr("name", L"launcher", 0);
    storage.leave();
    storage.enterNumSection(1);
    storage.setStr("name", L"hidetaskbar", 0);
    storage.leave();
    storage.leave();

    /*
    // system tray dock
    storage.enterNumSection(1);
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

  // autostart?
  //SHGetFolderPath(
  //if(storage.getInt("autostart", 1) 

  // create docks
  std::vector<Dock*> docks(storage.getNumSectionCount());
  for(int i = 0, count = storage.getNumSectionCount(); i < count; ++i)
  {
    Dock* dock = new Dock(storage, *storage.getNumSection(i));
    if(!dock->create())
      delete dock;
    else
      docks.push_back(dock);
  }

  // Main message loop:
  UINT exitCode = application.run();

  // delete docks
  for(std::vector<Dock*>::iterator i = docks.begin(), end = docks.end(); i != end; ++i)
    delete *i;
  docks.clear();

  // save storage
  if(!storageFile.empty())
    storage.save(storageFile.c_str());

  return (int) exitCode;
}
