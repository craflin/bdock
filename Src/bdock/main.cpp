// bdock.cpp : Defines the entry point for the application.
//

#include "stdafx.h"


#ifdef _DEBUG

#pragma comment ( linker, "/subsystem:console" )

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


LRESULT CALLBACK  wndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK  aboutDlgProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  // load storage
  std::wstring storageFile;
  Storage storage;
  {
    wchar path[MAX_PATH];
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

  // create docks
  std::set<Dock*> docks;
  for(int i = 0, count = storage.getNumSectionCount(); i < count; ++i)
  {
    Dock* dock = new Dock(storage.getNumSection(i));
    if(!dock->init(hInstance))
      delete dock;
    else
      docks.insert(dock);
  }

  // Main message loop:
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0))
  {
    //if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  // delete docks
  for(std::set<Dock*>::iterator i = docks.begin(), end = docks.end(); i != end; ++i)
    delete *i;
  docks.clear();

  // save storage
  if(!storageFile.empty())
    storage.save(storageFile.c_str());

  return (int) msg.wParam;
}


extern "C" BDOCK_API struct API::Icon* createIcon(struct API::Plugin* key, HBITMAP icon, unsigned int flags)
{
  Plugin* plugin = Plugin::lookup(key);
  if(plugin)
    return plugin->createIcon(icon, flags);
  return 0;
}

extern "C" BDOCK_API int destroyIcon(struct API::Plugin* key, struct API::Icon* icon)
{
  Plugin* plugin = Plugin::lookup(key);
  if(plugin && plugin->destroyIcon(icon))
    return 0;
  return -1;
}

extern "C" BDOCK_API int updateIcon(struct API::Plugin* key, struct API::Icon* icon)
{
  Plugin* plugin = Plugin::lookup(key);
  if(plugin && plugin->updateIcon(icon))
    return 0;
  return -1;
}

extern "C" BDOCK_API int updateIcons(struct API::Plugin* key, struct API::Icon** icons, uint count)
{
  Plugin* plugin = Plugin::lookup(key);
  if(plugin && plugin->updateIcons(icons, count))
    return 0;
  return -1;
}

extern "C" BDOCK_API int getIconRect(struct API::Plugin* key, struct API::Icon* icon, RECT* rect)
{
  Plugin* plugin = Plugin::lookup(key);
  if(plugin && plugin->getIconRect(icon, rect))
    return 0;
  return -1;
}

extern "C" BDOCK_API DWORD showMenu(struct API::Plugin* key, HMENU hmenu, int x, int y)
{
  Plugin* plugin = Plugin::lookup(key);
  if(plugin)
    return plugin->dock->showMenu(hmenu, x, y);
  return 0;
}

extern "C" BDOCK_API struct API::Timer* createTimer(struct API::Plugin* key, unsigned int interval)
{
  Plugin* plugin = Plugin::lookup(key);
  if(plugin)
    return plugin->createTimer(interval);
  return 0;
}

extern "C" BDOCK_API int updateTimer(struct API::Plugin* key, struct API::Timer* timer)
{
  Plugin* plugin = Plugin::lookup(key);
  if(plugin && plugin->updateTimer(timer))
    return 0;
  return -1;
}

extern "C" BDOCK_API int destroyTimer(struct API::Plugin* key, struct API::Timer* timer)
{
  Plugin* plugin = Plugin::lookup(key);
  if(plugin && plugin->destroyTimer(timer))
    return 0;
  return -1;
}

extern "C" BDOCK_API int enterStorageSection(struct API::Plugin* key, const char* name)
{
  Plugin* plugin = Plugin::lookup(key);
  if(plugin && plugin->storage->enterSection(name))
    return 0;
  return -1;
}

extern "C" BDOCK_API int enterStorageNumSection(struct API::Plugin* key, unsigned int pos)
{
  Plugin* plugin = Plugin::lookup(key);
  if(plugin && plugin->storage->enterNumSection(pos))
    return 0;
  return -1;
}

extern "C" BDOCK_API int leaveStorageSection(struct API::Plugin* key)
{
  Plugin* plugin = Plugin::lookup(key);
  if(plugin)
  {
    Storage* storage = plugin->storage;
    if(storage->getCurrentStorage() != storage && storage->leave())
      return 0;
  }
  return -1;
}

extern "C" BDOCK_API int deleteStorageSection(struct API::Plugin* key, const char* name)
{
  Plugin* plugin = Plugin::lookup(key);
  if(plugin && plugin->storage->deleteSection(name))
    return 0;
  return -1;
}

extern "C" BDOCK_API int deleteStorageNumSection(struct API::Plugin* key, uint pos)
{
  Plugin* plugin = Plugin::lookup(key);
  if(plugin && plugin->storage->deleteNumSection(pos))
    return 0;
  return -1;
}

extern "C" BDOCK_API unsigned int getStorageNumSectionCount(struct API::Plugin* key)
{
  Plugin* plugin = Plugin::lookup(key);
  if(plugin)
    return plugin->storage->getNumSectionCount();
  return 0;
}

extern "C" BDOCK_API int setStorageNumSectionCount(struct API::Plugin* key, unsigned int count)
{
  Plugin* plugin = Plugin::lookup(key);
  if(plugin && plugin->storage->setNumSectionCount(count))
    return 0;
  return -1;
}

extern "C" BDOCK_API const wchar* getStorageString(struct API::Plugin* key, const char* name, unsigned int* length, const wchar* default, unsigned int defaultLength)
{
  Plugin* plugin = Plugin::lookup(key);
  if(plugin)
    return plugin->storage->getStr(name, length, default, defaultLength);
  if(length)
    *length = defaultLength;
  return default;
}

extern "C" BDOCK_API int getStorageInt(struct API::Plugin* key, const char* name, int default)
{
  Plugin* plugin = Plugin::lookup(key);
  if(plugin)
    return plugin->storage->getInt(name, default);
  return default;
}

extern "C" BDOCK_API unsigned int getStorageUInt(struct API::Plugin* key, const char* name, unsigned int default)
{
  Plugin* plugin = Plugin::lookup(key);
  if(plugin)
    return plugin->storage->getUInt(name, default);
  return default;
}

extern "C" BDOCK_API int getStorageData(struct API::Plugin* key, const char* name, char** data, unsigned int* length, const char* defaultData, unsigned int defaultLength)
{
  Plugin* plugin = Plugin::lookup(key);
  if(plugin)
    return plugin->storage->getData(name, data, length, defaultData, defaultLength) ? 0 : -1;
  *data = (char*)defaultData;
  if(length)
    *length = defaultLength;
  return -1;
}

extern "C" BDOCK_API int setStorageString(struct API::Plugin* key, const char* name, const wchar_t* value, unsigned int length)
{
  Plugin* plugin = Plugin::lookup(key);
  if(plugin && plugin->storage->setStr(name, value, length))
    return 0;
  return -1;
}

extern "C" BDOCK_API int setStorageInt(struct API::Plugin* key, const char* name, int value)
{
  Plugin* plugin = Plugin::lookup(key);
  if(plugin && plugin->storage->setInt(name, value))
    return 0;
  return -1;
}

extern "C" BDOCK_API int setStorageUInt(struct API::Plugin* key, const char* name, unsigned int value)
{
  Plugin* plugin = Plugin::lookup(key);
  if(plugin && plugin->storage->setUInt(name, value))
    return 0;
  return -1;
}

extern "C" BDOCK_API int setStorageData(struct API::Plugin* key, const char* name, char* data, unsigned int length)
{
  Plugin* plugin = Plugin::lookup(key);
  if(plugin && plugin->storage->setData(name, data, length))
    return 0;
  return -1;
}

extern "C" BDOCK_API int deleteStorageEntry(struct API::Plugin* key, const char* name)
{
  Plugin* plugin = Plugin::lookup(key);
  if(plugin && plugin->storage->deleteEntry(name))
    return 0;
  return -1;
}
