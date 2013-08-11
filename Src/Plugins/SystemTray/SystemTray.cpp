// SystemTray.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

extern HMODULE hmodule;

DllInjection::DllInjection() : process(0), injectedModule(0) {}

DllInjection::~DllInjection()
{
  if(process)
  {
    if(injectedModule)
    {
      HANDLE hthread;
      hthread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)
        GetProcAddress(GetModuleHandle(L"kernel32.dll"), "FreeLibrary")/*&FreeLibrary*/, (void*)injectedModule, 0, NULL);
      WaitForSingleObject(hthread, INFINITE);
      CloseHandle(hthread);
    }
    CloseHandle(process);
  }
}

bool DllInjection::init(DWORD pid, const wchar* dll)
{
  process = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, pid);
  if(!process)
    return false;

  uint injectedStringSize = (uint)(wcslen(dll) + 1) * sizeof(wchar);
  void* injectedString = VirtualAllocEx(process, NULL, injectedStringSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  WriteProcessMemory(process, injectedString, dll, injectedStringSize, NULL);
  HANDLE hthread  = CreateRemoteThread(process, NULL, NULL, (LPTHREAD_START_ROUTINE)
    GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryW")
    /*&LoadLibraryW*/, injectedString, NULL, NULL);
  if(!hthread)  
  {
    VirtualFreeEx(process, injectedString, injectedStringSize, MEM_RELEASE);
    CloseHandle(process);
    process = 0;
    return false;
  }
  WaitForSingleObject(hthread, INFINITE);
  GetExitCodeThread(hthread, (LPDWORD)&injectedModule);
  VirtualFreeEx(process, injectedString, injectedStringSize, MEM_RELEASE);
  CloseHandle(hthread);
  if(!injectedModule)
  {
    CloseHandle(process);
    process = 0;
    return false;  
  }
  return true;
}

#define WM_SETNIDHOOKWND (WM_USER+503)
HWND nidHookWnd = 0;
WNDPROC TrayWndProc = 0;
LRESULT CALLBACK TrayWndProcHook(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch(uMsg)
  {
  case WM_SETNIDHOOKWND:
    nidHookWnd = (HWND)lParam;
    if(nidHookWnd)
      PostMessage(HWND_BROADCAST, RegisterWindowMessage(L"TaskbarCreated"), 0, 0);
    else
      SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)TrayWndProc);
    return (LRESULT)hmodule;
    break;
  case WM_COPYDATA:
    {
      PCOPYDATASTRUCT cds = (PCOPYDATASTRUCT)lParam;
      if(cds && cds->dwData == 1 && nidHookWnd)
        SendMessage(nidHookWnd, uMsg, wParam, lParam);
    }
    break;
  }
  return TrayWndProc(hwnd, uMsg, wParam, lParam);
}


ATOM SystemTray::wndClass = 0;
int SystemTray::instances = 0;

SystemTray::SystemTray(Dock& dock) : dock(dock)
{
  ++instances;
} 

SystemTray::~SystemTray()
{
  if(hwnd)
  {
    SendMessage(FindWindow(L"Shell_TrayWnd", 0), WM_SETNIDHOOKWND, 0, 0);
    DestroyWindow(hwnd);
  }

  for(stdext::hash_map<std::string,Icon*>::iterator i = icons.begin(), end = icons.end(); i != end; ++i)
    delete (IconData*)i->second->userData;

  --instances;
  if(instances == 0)
  {
    if(wndClass)
    {
      UnregisterClass(L"BDOCKLauncher", hmodule);
      wndClass = 0;
    }
  }
}

bool SystemTray::init()
{
  HWND hshell = FindWindow(L"Shell_TrayWnd", 0);
  if(!hshell)
    return false;

  DWORD pid;
  if(!GetWindowThreadProcessId(hshell, &pid))
    return false;

  wchar thisDll[MAX_PATH];
  GetModuleFileName(GetModuleHandle(L"systemtray.dll"), thisDll, MAX_PATH);
  if(!dllInjection.init(pid, thisDll))
    return false;

  // register window class
  if(!wndClass)
  {
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style      = 0; 
    wcex.lpfnWndProc  = wndProc;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = 0;
    wcex.hInstance    = hmodule;
    wcex.hIcon      = 0;
    wcex.hCursor    = 0;
    wcex.hbrBackground  = 0; //(HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName  = 0; //MAKEINTRESOURCE(IDC_BDOCK);
    wcex.lpszClassName  = L"BDOCKSystemTray";
    wcex.hIconSm    = 0;
    wndClass = RegisterClassEx(&wcex);
    if(!wndClass)
      return false;
  }

  hwnd = CreateWindowEx(NULL, L"BDOCKSystemTray", NULL, NULL, NULL, NULL, NULL, NULL, HWND_MESSAGE, NULL, hmodule, this);
  if(!hwnd)
    return false;

  dllInjection.injectedModule = (HMODULE)SendMessage(FindWindow(L"Shell_TrayWnd", 0), WM_SETNIDHOOKWND, 0, (LPARAM)hwnd);

  return true;
}

const char* keyOfIcon(PNOTIFYICONDATA32 nid)
{
  static char key[sizeof(GUID) + 1];
  char* keypos = key;
  if(nid->uFlags & NIF_GUID)
  {
    char* src = (char*)&nid->guidItem;
    char* srcEnd = src + sizeof(GUID);
    for(; src < srcEnd; ++src)
      *(keypos++) = *src ? *src : 1;
  }
  else
  {
    char* src = (char*)&nid->hWnd;
    char* srcEnd = src + sizeof(HWND);
    for(; src < srcEnd; ++src)
      *(keypos++) = *src ? *src : 1;
    src = (char*)&nid->uID;
    srcEnd = src + sizeof(UINT);
    for(; src < srcEnd; ++src)
      *(keypos++) = *src ? *src : 1;
  }
  *keypos = 0;
  return key;
}

void SystemTray::addIcon(PNOTIFYICONDATA32 nid)
{
  std::string key(keyOfIcon(nid));
  if(icons.find(key) != icons.end())
  {
    updateIcon2(nid);
    return;
  }

  if(nid->uFlags & NIF_STATE && (nid->dwState & nid->dwStateMask) & NIS_HIDDEN)
    return;
  
  HBITMAP bmp = createBitmapFromIcon((HICON)nid->hIcon, 0);
  Icon* icon = dock.createIcon(bmp, IF_SMALL);
  if(!icon)
  {
    DeleteObject(bmp);
    return;
  }
  IconData* iconData;
  icon->userData = iconData = new IconData(*this, (HICON)nid->hIcon, icon, nid);
  icon->handleMouseEvent = handleMouseEvent;
//  if(nid->uFlags & NIF_GUID)
//    memcpy(&iconData->guidItem, &nid->guidItem, sizeof(GUID));
  icons[key] = icon;
}

void SystemTray::updateIcon2(PNOTIFYICONDATA32 nid)
{
  std::string key(keyOfIcon(nid));
  stdext::hash_map<std::string,Icon*>::iterator i = icons.find(key);
  if(i == icons.end())
  {
    //addIcon(nid);
    return;
  }
  
  Icon* icon = i->second;
  IconData* iconData = (IconData*)icon->userData;

  if(nid->uFlags & NIF_ICON && (HICON)nid->hIcon != iconData->hicon)
  {
    iconData->hicon = (HICON)nid->hIcon;
    if(icon->icon)
      DeleteObject(icon->icon);
    icon->icon = createBitmapFromIcon((HICON)nid->hIcon, 0);
    dock.updateIcon(iconData->icon);
  }
  if(nid->hWnd)
    iconData->hwnd = (HWND)nid->hWnd;
  if(nid->uFlags & NIF_MESSAGE)
    iconData->callbackMessage = nid->uCallbackMessage;
}

void SystemTray::setIconVersion(PNOTIFYICONDATA32 nid)
{
  std::string key(keyOfIcon(nid));
  stdext::hash_map<std::string,Icon*>::iterator i = icons.find(key);
  if(i == icons.end())
    return;
  
  Icon* icon = i->second;
  IconData* iconData = (IconData*)icon->userData;
  iconData->version = nid->uVersion;
}

void SystemTray::removeIcon(PNOTIFYICONDATA32 nid)
{
  std::string key(keyOfIcon(nid));

  stdext::hash_map<std::string,Icon*>::iterator i = icons.find(key);
  if(i == icons.end())
    return;

  Icon* icon = i->second;
  delete (IconData*)icon->userData;
  icons.erase(i);
}

LRESULT CALLBACK SystemTray::wndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
  case WM_CREATE:
    {
      LPCREATESTRUCT cs = (LPCREATESTRUCT)lParam;
      SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
    }
    break;
  case WM_COPYDATA:
    {
      PCOPYDATASTRUCT cds = (PCOPYDATASTRUCT)lParam;
      if(cds->dwData == 1)
      {
        uint cmd = *((uint*)((char*)cds->lpData + 4));
        PNOTIFYICONDATA32 nid = (PNOTIFYICONDATA32)((char*)cds->lpData + 8);
        switch(cmd)
        {
        case NIM_ADD:
          ((SystemTray*)GetWindowLongPtr(hwnd, GWLP_USERDATA))->addIcon(nid);
          break;
        case NIM_MODIFY:
          ((SystemTray*)GetWindowLongPtr(hwnd, GWLP_USERDATA))->updateIcon2(nid);
          break;
        case NIM_DELETE:
          ((SystemTray*)GetWindowLongPtr(hwnd, GWLP_USERDATA))->removeIcon(nid);
          break;
        case NIM_SETFOCUS:
          break;
        case NIM_SETVERSION:
          ((SystemTray*)GetWindowLongPtr(hwnd, GWLP_USERDATA))->setIconVersion(nid);
          break;
        }
      }
    }
    break;
  default:
    return DefWindowProc(hwnd, message, wParam, lParam);
  }
  return 0;
}

int SystemTray::handleMouseEvent(Icon* icon, unsigned int message, int x, int y)
{
  switch(message)
  {
  case WM_LBUTTONDOWN:
  case WM_LBUTTONUP:
  case WM_LBUTTONDBLCLK:
  case WM_MBUTTONDOWN:
  case WM_MBUTTONUP:
  case WM_MBUTTONDBLCLK:
  case WM_RBUTTONDOWN:
  case WM_RBUTTONUP:
  case WM_RBUTTONDBLCLK:
  case WM_MOUSEMOVE:
    {
      IconData* iconData = (IconData*)icon->userData;
      if(iconData->callbackMessage)
      {
        if(message == WM_LBUTTONUP || message == WM_MBUTTONUP || message == WM_RBUTTONUP)
          SetForegroundWindow(iconData->hwnd);

        if(iconData->version == NOTIFYICON_VERSION_4)
          SendMessage(iconData->hwnd, iconData->callbackMessage, MAKELONG(x, y), MAKELONG(message, iconData->id));
        else
          SendMessage(iconData->hwnd, iconData->callbackMessage, iconData->id, message);


      }
    }
    break;
  //case WM_CONTEXTMENU:
    //break;
  default:
    return -1;
  }
  return 0;
}

IconData::IconData(SystemTray& systemTray, HICON hicon, Icon* icon, PNOTIFYICONDATA32 nid) : 
  systemTray(systemTray), hicon(hicon), icon(icon), hwnd((HWND)nid->hWnd), 
  callbackMessage(nid->uFlags & NIF_MESSAGE ? nid->uCallbackMessage : 0),
  version(0), id(nid->uID)
{
}

IconData::~IconData()
{
  if(icon)
  {
    if(icon->icon)
      DeleteObject(icon->icon);
    systemTray.dock.destroyIcon(icon);
  }
}
