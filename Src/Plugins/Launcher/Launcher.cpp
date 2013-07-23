// launcher.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "resource.h"

extern HMODULE hmodule;

ATOM Launcher::wndClass = 0;
int Launcher::instances = 0;

Launcher::Launcher(Dock& dock) : dock(dock), defaultIcon(LoadIcon(NULL, IDI_APPLICATION)), hwnd(0), activeHwnd(0)
{
  if(instances == 0)
    wmiInit();
  ++instances;
} 

Launcher::~Launcher()
{
  if(hwnd)
  {
    DeregisterShellHookWindow(hwnd);
    DestroyWindow(hwnd);
  }

  for(std::vector<Icon*>::iterator i = launchers.begin(), end = launchers.end(); i != end; ++i)
  {
    IconData* iconData = (IconData*)(*i)->userData;
    if(iconData->hwnd == 0)
      delete iconData;
  }
  for(std::unordered_map<HWND, Icon*>::iterator i = icons.begin(), end = icons.end(); i != end; ++i)
    delete (IconData*)i->second->userData;

  --instances;
  if(instances == 0)
  {
    if(wndClass)
    {
      UnregisterClass(L"BDOCKLauncher", hmodule);
      wndClass = 0;
    }
    wmiCleanup();
  }
}

bool Launcher::init()
{
  // load launcher
  for(int i = 0, count = dock.getStorageNumSectionCount(); i < count; ++i)
  {
    dock.enterStorageNumSection(i);
    BITMAP* header;
    char* data;
    unsigned int len;
    HBITMAP bitmap = 0;
    if(!dock.getStorageData("iconHeader", (char**)&header, &len, 0, 0) && len == sizeof(BITMAP))
      if(!dock.getStorageData("iconData", &data, &len, 0, 0) && len == header->bmWidthBytes * header->bmHeight)
        bitmap = CreateBitmap(header->bmWidth, header->bmHeight, header->bmPlanes, header->bmBitsPixel, data);
    const wchar* path = dock.getStorageString("path", 0, L"", 0);
    const wchar* parameters = dock.getStorageString("parameters", 0, L"", 0);
    dock.leaveStorageSection();

    Icon* icon = dock.createIcon(bitmap, IF_GHOST);
    if(!icon)
      DeleteObject(bitmap);
    else
    {
      icon->handleMouseEvent = handleMouseEvent;
      IconData* iconData = new IconData(*this, 0, icon, 0, path, parameters);;
      icon->userData = iconData;
      iconData->pinned = true;
      launchers.push_back(icon);
    }
  }

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
    wcex.lpszClassName  = L"BDOCKLauncher";
    wcex.hIconSm    = 0;
    wndClass = RegisterClassEx(&wcex);
    if(!wndClass)
      return false;
  }

  hwnd = CreateWindowEx(NULL, L"BDOCKLauncher", NULL, NULL, NULL, NULL, NULL, NULL, HWND_MESSAGE, NULL, hmodule, this);
  if(!hwnd)
    return false;
  if(!RegisterShellHookWindow(hwnd))
    return false;

  activeHwnd = GetForegroundWindow();
  if(!EnumWindows(EnumWindowsProc, (LPARAM)this))
    return false;
  return true;
}

bool Launcher::hasTaskBarIcon(HWND hwnd)
{
  if(IsWindowVisible(hwnd) && GetParent(hwnd) == 0)
  {
    DWORD exstyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    HANDLE owner = GetWindow(hwnd, GW_OWNER);
    if((!(exstyle & WS_EX_TOOLWINDOW) && owner == 0) || (exstyle & WS_EX_APPWINDOW && owner != 0))
    {
      wchar name[32];
      GetClassName(hwnd, name, 31);
      if(!wcscmp(name, L"Shell_TrayWnd"))
        return false;
      return true;
    }
  }
  return false;
}

void Launcher::addIcon(HWND hwnd)
{  
  if(!hasTaskBarIcon(hwnd))
    return;

  if(icons.find(hwnd) != icons.end())
    return; // wtf

  // get command line
  std::wstring path, parameters;
  getCommandLine(hwnd, path, parameters);

  // there might be a launcher for this window. if so, use launcher icon
  for(std::vector<Icon*>::iterator i = launchers.begin(), end = launchers.end(); i != end; ++i)
  {
    IconData* iconData = (IconData*)(*i)->userData;
    if(!iconData->hwnd && !iconData->path.compare(path))
    {
      iconData->hwnd = hwnd;
      (*i)->flags &= ~IF_GHOST;
      icons[hwnd] = iconData->icon;
      updateIcon(hwnd, true);
      return;
    }
  }

  // add icon
  HICON hicon;
  hicon = (HICON)SendMessage(hwnd, WM_GETICON, ICON_BIG, 0); 
  if(!hicon)
    hicon = (HICON)GetClassLongPtr(hwnd, GCLP_HICON);
  if(!hicon)
    hicon = defaultIcon;

  HBITMAP bitmap = createBitmapFromIcon(hicon, 0);
  Icon* icon = dock.createIcon(bitmap, hwnd == activeHwnd ? IF_ACTIVE : 0);
  if(!icon)
  {
    DeleteObject(bitmap);
    return;
  }
  icon->handleMouseEvent = handleMouseEvent;
  icon->userData = new IconData(*this, hicon, icon, hwnd, path, parameters);
  //GetWindowText(hwnd, newIcon->title, sizeof(newIcon->title));

  icons[hwnd] = icon;
}

void Launcher::activateIcon(HWND hwnd)
{
  if(hwnd == activeHwnd)
    return;

  Icon* updateIcons[2];
  uint updateIconsCount = 0;

  std::unordered_map<HWND, Icon*>::iterator i = icons.find(activeHwnd);
  if(i != icons.end() && i->second->flags & IF_ACTIVE)
  {
    i->second->flags &= ~IF_ACTIVE;
    updateIcons[updateIconsCount++] = i->second;
  }

  i = icons.find(hwnd);
  if(i != icons.end() && !(i->second->flags & IF_ACTIVE))
  {
    i->second->flags |= IF_ACTIVE;
    updateIcons[updateIconsCount++] = i->second;
  }
  activeHwnd = hwnd;

  dock.updateIcons(updateIcons, updateIconsCount);
}

void Launcher::updateIcon(HWND hwnd, bool forceUpdate)
{
  if(icons.find(hwnd) == icons.end())
    return;

  Icon* icon = icons[hwnd];
  IconData* iconData = (IconData*)icon->userData;

  HICON hicon;
  hicon = (HICON)SendMessage(hwnd, WM_GETICON, ICON_BIG, 0); 
  if(!hicon)
    hicon = (HICON)GetClassLongPtr(hwnd, GCLP_HICON);
  if(!hicon)
    hicon = hicon;

  if(iconData->hicon != hicon)
  {
    iconData->hicon = hicon;
    if(icon->icon)
      DeleteObject(icon->icon);
    icon->icon = createBitmapFromIcon(hicon, 0);
    forceUpdate = true;
  }
  if(forceUpdate)
    dock.updateIcon(icon);
}

void Launcher::removeIcon(HWND hwnd)
{
  if(icons.find(hwnd) == icons.end())
    return;

  Icon* icon = icons[hwnd];
  IconData* iconData = (IconData*)icon->userData;
  if(iconData->pinned)
  {
    // maybe TODO: try to find a not-pinned icon compatible with this launcher. adopt the hwnd and remove that icon.

    // find launcher index
    uint pos = 0;
    for(std::vector<Icon*>::iterator i = launchers.begin(), end = launchers.end(); i != end; ++i, ++pos)
      if(*i == icon)
        break;

    // try to restore the icon from storage
    if(pos < (uint)launchers.size())
    {
      dock.enterStorageNumSection(pos);
      BITMAP* header;
      char* data;
      unsigned int len;
      HBITMAP bitmap = 0;
      if(!dock.getStorageData("iconHeader", (char**)&header, &len, 0, 0) && len == sizeof(BITMAP))
        if(!dock.getStorageData("iconData", &data, &len, 0, 0) && len == header->bmWidthBytes * header->bmHeight)
          bitmap = CreateBitmap(header->bmWidth, header->bmHeight, header->bmPlanes, header->bmBitsPixel, data);
      dock.leaveStorageSection();
      if(bitmap)
      {
        if(icon->icon)
          DeleteObject(icon->icon);
        icon->icon = bitmap;
      }
    }

    iconData->hwnd = 0;
    icon->flags |= IF_GHOST;
    icon->flags &= ~IF_ACTIVE;
    dock.updateIcon(icon);
  }
  else
  {
    delete iconData;
    dock.destroyIcon(icon);
  }
  icons.erase(hwnd);
}

bool Launcher::launch(Icon& icon)
{
  IconData* iconData = (IconData*)icon.userData;
  std::wstring dir(iconData->path);
  size_t pos = dir.find_last_of(L'\\');
  size_t pos2 = dir.find_last_of(L'/');
  if(pos == std::wstring::npos || (pos != std::wstring::npos && pos2 != std::wstring::npos && pos2 > pos))
    pos = pos2;
  if(pos != std::wstring::npos)
    dir.resize(pos);
  else
    dir.empty();
  HINSTANCE inst = ShellExecuteW(NULL, NULL, iconData->path.c_str(), iconData->parameters.c_str(),
    dir.empty() ? NULL : dir.c_str(), SW_SHOWNORMAL);
  if((DWORD)inst <= 32)
    return false;
  CloseHandle(inst);
  return true;
}

BOOL CALLBACK Launcher::EnumWindowsProc(HWND hwnd, LPARAM lParam)
{  
  ((Launcher*)lParam)->addIcon(hwnd);
  return TRUE;
}

typedef struct
{
  HWND hwnd;
  short left;
  short top;
  short right;
  short bottom;
} SHELLHOOKINFO2, *LPSHELLHOOKINFO2;

LRESULT CALLBACK Launcher::wndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
  case WM_CREATE:
    {
      LPCREATESTRUCT cs = (LPCREATESTRUCT)lParam;
      SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
    }
    break;
  default:
    {
      static unsigned int wm_shellhook = RegisterWindowMessage(L"SHELLHOOK");
      if(message == wm_shellhook)
      {
        switch(wParam)
        {
        case HSHELL_WINDOWACTIVATED:
        case HSHELL_RUDEAPPACTIVATED:
          ((Launcher*)GetWindowLongPtr(hwnd, GWLP_USERDATA))->activateIcon((HWND)lParam);
          break;
        case HSHELL_WINDOWCREATED:
          ((Launcher*)GetWindowLongPtr(hwnd, GWLP_USERDATA))->addIcon((HWND)lParam);
          break;
        case HSHELL_WINDOWDESTROYED:
          ((Launcher*)GetWindowLongPtr(hwnd, GWLP_USERDATA))->removeIcon((HWND)lParam);
          break;
        case HSHELL_REDRAW:
          ((Launcher*)GetWindowLongPtr(hwnd, GWLP_USERDATA))->updateIcon((HWND)lParam, false);
          break;
        case HSHELL_GETMINRECT:
          {
            LPSHELLHOOKINFO2 shi = (LPSHELLHOOKINFO2)lParam;
            Launcher* launcher = (Launcher*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            std::unordered_map<HWND,Icon*>::iterator i = launcher->icons.find(shi->hwnd);
            if(i != launcher->icons.end())
            {
              RECT rect;
              launcher->dock.getIconRect(i->second, &rect);
              shi->left = short(rect.left);
              shi->top = short(rect.top);
              shi->right = short(rect.right);
              shi->bottom = short(rect.bottom);
              return 1;
            }
          }
          break;
        }
        break;
      }
      return DefWindowProc(hwnd, message, wParam, lParam);
    }
  }
  return 0;
}

bool Launcher::getCommandLine(HWND hwnd, std::wstring& path, std::wstring& param)
{
  DWORD pid;
  if(!GetWindowThreadProcessId(hwnd, &pid))
    return false;
  
  if(!wmiRequestFormat(L"SELECT CommandLine FROM Win32_Process WHERE (ProcessId=%u)", pid))
    return false;
  if(!wmiGetNextResult())
    return false;

  LPCWSTR result;
  if(!wmiGetStrValue(L"CommandLine", &result))
    return false;

  bool quoted = *result == L'"';
  if(quoted)
    ++result;
  LPCWSTR parameters = wcschr(result, quoted ? L'"' : L' ');
  if(!parameters)
  {
    path = result;
    param = std::wstring();
  }
  else
  { 
    path = std::wstring(result, parameters - result);
    if(quoted)
      ++parameters;
    if(*parameters == ' ')
      ++parameters;
    param = parameters;
  }
  return true;
}

void Launcher::showContextMenu(Icon* icon, int x, int y)
{
  IconData* iconData = (IconData*)icon->userData;

  HMENU hmenu;

  if(iconData->hwnd)
  {
    HMENU hsysmenu = GetSystemMenu(iconData->hwnd, FALSE);
    if(!IsMenu(hsysmenu))
    {
      GetSystemMenu(iconData->hwnd, TRUE);
      hsysmenu = GetSystemMenu(iconData->hwnd, FALSE);
    }

    hmenu = CopyMenu(hsysmenu);
    InsertMenu(hmenu, 0, MF_BYPOSITION | MF_SEPARATOR, NULL, NULL);
  }
  else
    hmenu = CreatePopupMenu();

  wchar str[32];
  LoadString(hmodule, IDS_LAUNCH, str, sizeof(str));
  InsertMenu(hmenu, 0, MF_BYPOSITION |  MF_STRING, 2, str);
  LoadString(hmodule, iconData->pinned ? IDS_UNPIN : IDS_PIN, str, sizeof(str));
  InsertMenu(hmenu, 1, MF_BYPOSITION | MF_STRING, 1, str);
  
  if(!iconData->hwnd)
    SetMenuDefaultItem(hmenu, 0, MF_BYPOSITION);

  HWND hwnd = iconData->hwnd; // copy hwnd since the window may close and iconData may be destroyed while showMenu()
  DWORD cmd = dock.showMenu(hmenu, x, y);
  DestroyMenu(hmenu);
  
  if(!cmd)
    return;

  if(hwnd && (icons.find(hwnd) == icons.end() || icons[hwnd] != icon))
    return;
  
  switch(cmd)
  {
  case 1:
    if(iconData->pinned)
    {
      iconData->pinned = false;
      size_t pos = launchers.end() - launchers.begin();
      for(std::vector<Icon*>::iterator i = launchers.begin(), end = launchers.end(); i != end; ++i)
        if(*i == icon)
        {
          pos = i - launchers.begin();
          break;
        }
      if(pos != launchers.end() - launchers.begin())
      {
        launchers.erase(launchers.begin() + pos);
        dock.deleteStorageNumSection((uint)pos);
      }

      if(!iconData->hwnd)
      {
        delete iconData;
        dock.destroyIcon(icon);
      }
    }
    else
    {
      iconData->pinned = true;

      size_t pos = launchers.end() - launchers.begin();
      for(std::vector<Icon*>::iterator i = launchers.begin(), end = launchers.end(); i != end; ++i)
        if(*i == icon)
        {
          pos = i - launchers.begin();
          break;
        }
      if(pos == launchers.end() - launchers.begin())
      {
        uint pos = (uint)launchers.size();
        launchers.push_back(icon);
        dock.enterStorageNumSection(pos);
        dock.setStorageString("path", iconData->path.c_str(), (uint)iconData->path.length());
        dock.setStorageString("parameters", iconData->parameters.c_str(), (uint)iconData->parameters.length());
        BITMAP bm;
        if(GetObject(icon->icon, sizeof(BITMAP), &bm))
        {
          dock.setStorageData("iconHeader", (char*)&bm, sizeof(bm));
          uint size = bm.bmWidthBytes * bm.bmHeight;
          char* buffer = (char*)malloc(size);
          if(buffer)
          {
            if(GetBitmapBits(icon->icon, size, buffer))
              dock.setStorageData("iconData", buffer, size);
            free(buffer);
          }
        }
        dock.leaveStorageSection();
      }
    }
    break;
  case 2:
    launch(*icon);
    break;
  default:
    SetForegroundWindow(hwnd);
    SendMessage(hwnd, WM_SYSCOMMAND, cmd, MAKELONG(x, y));
    break;
  }
}

int Launcher::handleMouseEvent(Icon* icon, unsigned int message, int x, int y)
{
  IconData* iconData = (IconData*) icon->userData;
  switch(message)
  {
  case WM_LBUTTONUP:
    {
      HWND hwnd = iconData->hwnd;
      if(hwnd)
      {
        if(!IsWindow(hwnd))
        {
          iconData->launcher.removeIcon(hwnd);
          break;
        }

        bool iconic = IsIconic(hwnd) ? true : false;
        if(hwnd == GetForegroundWindow())
        {
          if(IsIconic(hwnd))
            PostMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, MAKELONG(x, y));
          else
            PostMessage(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, MAKELONG(x, y));
        }
        else 
        {
          if(IsIconic(hwnd))
            PostMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, MAKELONG(x, y));
          SetForegroundWindow(hwnd);
        }
      }
      else
      {        
        iconData->launcher.launch(*icon);
      }
    }
    break;
  case WM_MBUTTONUP:
    iconData->launcher.launch(*icon);
    break;
  case WM_CONTEXTMENU:
    iconData->launcher.showContextMenu(icon, x, y);
    break;
  default:
    return -1;
  }
  return 0;
}

IconData::IconData(Launcher& launcher, HICON hicon, Icon* icon, HWND hwnd, const std::wstring& path, const std::wstring& parameters) :
  launcher(launcher), hicon(hicon), icon(icon), hwnd(hwnd), path(path), parameters(parameters), pinned(false) {}

IconData::~IconData()
{
  if(icon && icon->icon)
    DeleteObject(icon->icon);
}
