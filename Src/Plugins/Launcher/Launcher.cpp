// launcher.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

Launcher::Launcher(Dock& dock) : dock(dock), defaultIcon(IDI_APPLICATION), activeHwnd(0), hotIcon(0)
{
}

Launcher::~Launcher()
{
  if(hwnd)
    deregisterShellHookWindow();

  for(auto i = icons.begin(), end = icons.end(); i != end; ++i)
    delete *i;
}

bool Launcher::create()
{
  // load pinned icons
  for(int i = 0, count = dock.getStorageNumSectionCount(); i < count; ++i)
  {
    dock.enterStorageNumSection(i);
    BITMAP* header;
    char* data;
    unsigned int len;
    HBITMAP bitmap = 0;
    if(!dock.getStorageData("iconHeader", (char**)&header, &len, 0, 0) && len >= sizeof(BITMAP) - sizeof(LPVOID))
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
      icon->handleMoveEvent = handleMoveEvent;
      IconData* iconData = new IconData(*this, 0, icon, 0, path, parameters, i);
      icon->userData = iconData;
      iconData->pinned = true;
      icons.insert(iconData);
    }
  }

  // create shell hook message window
  if(!WinAPI::Window::create(_T("BDOCKLauncher"), 0, HWND_MESSAGE, NULL, NULL, NULL, _T("BDOCKLauncher"), 0, 0))
    return false;
  if(!registerShellHookWindow())
    return false;

  RegisterHotKey(hwnd, 123, MOD_ALT|MOD_WIN, VK_LEFT);
  RegisterHotKey(hwnd, 124, MOD_ALT|MOD_WIN, VK_RIGHT);

  // add icon for each opened window
  activeHwnd = GetForegroundWindow();
  struct EnumProc
  {
    static BOOL CALLBACK enumWindowsProc(HWND hwnd, LPARAM lParam)
    {  
      ((Launcher*)lParam)->addIcon(hwnd);
      return TRUE;
    }
  };
  if(!EnumWindows(EnumProc::enumWindowsProc, (LPARAM)this))
    return false;
  return true;
}

bool Launcher::hasTaskBarIcon(HWND hwnd)
{
  if(IsWindowVisible(hwnd) && GetParent(hwnd) == 0)
  {
    DWORD exstyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    HANDLE owner = GetWindow(hwnd, GW_OWNER);
    //if((!(exstyle & WS_EX_TOOLWINDOW) && owner == 0) || (exstyle & WS_EX_APPWINDOW && owner != 0))
    if((owner == 0 && !(exstyle & WS_EX_TOOLWINDOW)) || (exstyle & WS_EX_APPWINDOW))
    {
      TCHAR name[32];
      GetClassName(hwnd, name, 31);
      if(!_tcscmp(name, _T("Shell_TrayWnd")))
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

  if(iconsByHWND.find(hwnd) != iconsByHWND.end())
    return; // wtf

  // get command line
  std::wstring path, parameters;
  getCommandLine(hwnd, path, parameters);

  // there might be an unused pinned icon for this window. if so, use that icon
  for(auto i = icons.begin(), end = icons.end(); i != end; ++i)
  {
    IconData* iconData = *i;
    if(!iconData->hwnd && !iconData->path.compare(path))
    {
      iconData->hwnd = hwnd;
      iconData->icon->flags &= ~IF_GHOST;
      iconsByHWND[hwnd] = iconData;
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

  HBITMAP bitmap = CreateBitmapFromIcon(32, 32, hicon);
  Icon* icon = dock.createIcon(bitmap, hwnd == activeHwnd ? IF_ACTIVE : 0);
  if(!icon)
  {
    DeleteObject(bitmap);
    return;
  }
  icon->handleMouseEvent = handleMouseEvent;
  icon->handleMoveEvent = handleMoveEvent;
  IconData* iconData = new IconData(*this, hicon, icon, hwnd, path, parameters, -1);
  icon->userData = iconData;
  //GetWindowText(hwnd, newIcon->title, sizeof(newIcon->title));

  icons.insert(iconData);
  iconsByHWND[hwnd] = iconData;
}

void Launcher::activateIcon(HWND hwnd)
{
  if(hwnd == activeHwnd)
  {
    if(activeHwnd && !IsWindow(activeHwnd))
      removeIcon(activeHwnd);
    return;
  }

  Icon* updateIcons[2];
  uint updateIconsCount = 0;

  if(activeHwnd)
  {
    if(!IsWindow(activeHwnd))
      removeIcon(activeHwnd);
    else
    {
      auto i = iconsByHWND.find(activeHwnd);
      if(i != iconsByHWND.end())
      {
        Icon* icon = i->second->icon;
        if(icon->flags & IF_ACTIVE)
        {
          icon->flags &= ~IF_ACTIVE;
          updateIcons[updateIconsCount++] = icon;
        }
      }
    }
  }

  if(hwnd)
  {
    if(!IsWindow(hwnd))
    {
      removeIcon(hwnd);
      hwnd = 0;
    }
    else
    {
      auto i = iconsByHWND.find(hwnd);
      if(i != iconsByHWND.end())
      {
        Icon* icon = i->second->icon;
        if(!(icon->flags & IF_ACTIVE))
        {
          icon->flags |= IF_ACTIVE;
          updateIcons[updateIconsCount++] = icon;
        }
      }
    }
  }
  activeHwnd = hwnd;

  if(updateIconsCount > 0)
    dock.updateIcons(updateIcons, updateIconsCount);
}

void Launcher::updateIcon(HWND hwnd, bool forceUpdate)
{
  auto i = iconsByHWND.find(hwnd);
  if(i == iconsByHWND.end())
    return;

  if(!IsWindow(hwnd))
  {
    removeIcon(hwnd);
    return;
  }

  IconData* iconData = i->second;
  Icon* icon = iconData->icon;

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
    icon->icon = CreateBitmapFromIcon(32, 32, hicon);
    forceUpdate = true;
  }

  if(forceUpdate)
    dock.updateIcon(icon);
}

void Launcher::removeIcon(HWND hwnd)
{
  auto i = iconsByHWND.find(hwnd);
  if(i == iconsByHWND.end())
    return;

  IconData* iconData = i->second;
  iconsByHWND.erase(i);

  if(iconData->pinned)
  {
    // try to find a icon compatible with this launcher. adopt the hwnd and remove that icon.
    for(auto i = icons.begin(), end = icons.end(); i != end; ++i)
    {
      IconData* itIconData = *i;
      if(itIconData != iconData && itIconData->hwnd && 
        (!itIconData->pinned || itIconData->launcherIndex > iconData->launcherIndex) &&
        !itIconData->path.compare(iconData->path))
      {
        if(!itIconData->pinned)
        {
          iconData->hwnd = itIconData->hwnd;
          iconData->icon->flags = itIconData->icon->flags;
          iconsByHWND.erase(itIconData->hwnd);
          iconsByHWND[iconData->hwnd] = iconData;
          
          removeIcon(*itIconData);
          updateIcon(iconData->hwnd, true);
          return;
        }
        else
        {
          iconData->hwnd = itIconData->hwnd;
          iconData->icon->flags = itIconData->icon->flags;
          iconsByHWND.erase(itIconData->hwnd);
          iconsByHWND[iconData->hwnd] = iconData;

          updateIcon(iconData->hwnd, true);

          iconData = itIconData;
          break;
        }
      }
    }

    // restore the icon from storage
    if(dock.enterStorageNumSection(iconData->launcherIndex) == 0)
    {
      BITMAP* header;
      char* data;
      unsigned int len;
      HBITMAP bitmap = 0;
      if(!dock.getStorageData("iconHeader", (char**)&header, &len, 0, 0) && len >= sizeof(BITMAP) - sizeof(LPVOID))
        if(!dock.getStorageData("iconData", &data, &len, 0, 0) && len == header->bmWidthBytes * header->bmHeight)
          bitmap = CreateBitmap(header->bmWidth, header->bmHeight, header->bmPlanes, header->bmBitsPixel, data);
      dock.leaveStorageSection();
      if(bitmap)
      {
        if(iconData->icon->icon)
          DeleteObject(iconData->icon->icon);
        iconData->icon->icon = bitmap;
      }
    }

    iconData->hwnd = 0;
    iconData->icon->flags |= IF_GHOST;
    iconData->icon->flags &= ~IF_ACTIVE;
    dock.updateIcon(iconData->icon);
  }
  else
    removeIcon(*iconData);
}

void Launcher::removeIcon(IconData& iconData)
{
  icons.erase(&iconData);
  delete &iconData;

  if(&iconData == hotIcon)
    hotIcon = 0;
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
  HINSTANCE inst = WinAPI::Shell::execute(NULL, NULL, iconData->path.c_str(), iconData->parameters.c_str(),
    dir.empty() ? NULL : dir.c_str(), SW_SHOWNORMAL);
  if((DWORD)inst <= 32)
    return false;
  return true;
}

typedef struct
{
  HWND hwnd;
  short left;
  short top;
  short right;
  short bottom;
} SHELLHOOKINFO2, *LPSHELLHOOKINFO2;

LRESULT Launcher::onMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
  case WM_KEYUP:
    if(wParam == VK_MENU)
    {
      if(hotIcon)
      {
        IconData* icon = hotIcon;
        hotIcon = 0;
        icon->icon->flags &= ~IF_HOT;
        handleMouseEvent(icon->icon, WM_LBUTTONUP, 0, 0);
      }
    }
    break;
  case WM_KILLFOCUS:
    if(hotIcon)
    {
      IconData* icon = hotIcon;
      hotIcon = 0;
      icon->icon->flags &= ~IF_HOT;
      dock.updateIcon(icon->icon);
    }
    break;
  case WM_HOTKEY:
    {
      if(hotIcon && hotIcon->icon->flags & IF_HOT)
      {
        hotIcon->icon->flags &= ~IF_HOT;
        dock.updateIcon(hotIcon->icon);
      }
      if(!hotIcon)
      {
        auto i = iconsByHWND.find(activeHwnd);
        if(i != iconsByHWND.end())
          hotIcon = i->second;
      }
      IconData* iconToHighlight = 0;

      if(!icons.empty())
      {
        if(!hotIcon)
          iconToHighlight = (IconData*) dock.getFirstIcon()->userData;
        else
        {
          if(HIWORD(lParam) == VK_RIGHT)
          {
            Icon* icon = dock.getNextIcon(hotIcon->icon);
            if(!icon)
              icon = dock.getFirstIcon();
            iconToHighlight = (IconData*) icon->userData;
          }
          else if(HIWORD(lParam) == VK_LEFT)
          {
            Icon* icon = dock.getPreviousIcon(hotIcon->icon);
            if(!icon)
              icon = dock.getLastIcon();
            iconToHighlight = (IconData*) icon->userData;
          }
        }
      }
      if(iconToHighlight)
      {
        if(!(iconToHighlight->icon->flags & IF_HOT))
        {
          iconToHighlight->icon->flags |= IF_HOT;
          dock.updateIcon(iconToHighlight->icon);
        }
        hotIcon = iconToHighlight;
        SetForegroundWindow(hwnd);
      }
    }
    break;
  case WM_COMMAND:
  default:
    {
      static unsigned int wm_shellhook = RegisterWindowMessage(_T("SHELLHOOK"));
      if(message == wm_shellhook)
      {
        switch(wParam)
        {
        case HSHELL_WINDOWACTIVATED:
        case HSHELL_RUDEAPPACTIVATED:
          activateIcon((HWND)lParam);
          break;
        case HSHELL_WINDOWCREATED:
          addIcon((HWND)lParam);
          break;
        case HSHELL_WINDOWDESTROYED:
          removeIcon((HWND)lParam);
          break;
        case HSHELL_REDRAW:
          updateIcon((HWND)lParam, false);
          break;
        case HSHELL_GETMINRECT:
          {
            LPSHELLHOOKINFO2 shi = (LPSHELLHOOKINFO2)lParam;
            auto i = iconsByHWND.find(shi->hwnd);
            if(i != iconsByHWND.end())
            {
              RECT rect;
              dock.getIconRect(i->second->icon, &rect);
              shi->left = short(rect.left);
              shi->top = short(rect.top);
              shi->right = short(rect.right);
              shi->bottom = short(rect.bottom);
              return 1;
            }
          }
          break;
          /*
        case HSHELL_FLASH:
          printf("unhandled HSHELL_FLASH msg=%u\n", (uint) wParam);
          break;
        default:
          printf("unhandled HSELL msg=%u\n", (uint) wParam);
          break;
          */
        }
        break;
      }
      return WinAPI::Window::onMessage(message, wParam, lParam);
    }
  }
  return 0;
}


bool Launcher::getCommandLine(HWND hwnd, std::wstring& path, std::wstring& param)
{
  DWORD pid;
  if(!GetWindowThreadProcessId(hwnd, &pid))
    return false;

  WCHAR commandLine[32768]; // the default stack size is 1mb... so 64kb should fit on the stack
  if(!GetCommandLine(pid, commandLine, 32768))
    return false;

  LPCTSTR result = commandLine;
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
  LoadString(WinAPI::Application::getInstance(), IDS_LAUNCH, str, sizeof(str));
  InsertMenu(hmenu, 0, MF_BYPOSITION |  MF_STRING, 2, str);
  LoadString(WinAPI::Application::getInstance(), iconData->pinned ? IDS_UNPIN : IDS_PIN, str, sizeof(str));
  InsertMenu(hmenu, 1, MF_BYPOSITION | MF_STRING, 1, str);
  
  if(!iconData->hwnd)
    SetMenuDefaultItem(hmenu, 0, MF_BYPOSITION);

  HWND hwnd = iconData->hwnd; // copy hwnd since the window may close and iconData may be destroyed while showMenu()
  DWORD cmd = dock.showMenu(hmenu, x, y);
  DestroyMenu(hmenu);
  
  if(!cmd)
    return;

  if(hwnd)
  {
    auto i = iconsByHWND.find(hwnd);
    if(i == iconsByHWND.end() || i->second != iconData || iconData->icon != icon)
      return;
  }
  
  switch(cmd)
  {
  case 1:
    if(iconData->pinned)
    { // unpin icon
      int index = iconData->launcherIndex;
      if(dock.deleteStorageNumSection(index) == 0)
      {
        for(auto i = icons.begin(), end = icons.end(); i != end; ++i)
          if((*i)->launcherIndex > index)
            --((*i)->launcherIndex);

        iconData->pinned = false;
        iconData->launcherIndex = -1;

        // remove icon
        if(!iconData->hwnd)
          removeIcon(*iconData);
      }
    }
    else
    { // pin icon
      int index = -1;
      for(auto i = icons.begin(), end = icons.end(); i != end; ++i)
        if((*i)->pinned && (*i)->launcherIndex > index)
          index = (*i)->launcherIndex;
      ++index;

      if(dock.enterStorageNumSection(index) == 0)
      {
        dock.setStorageString("path", iconData->path.c_str(), (uint)iconData->path.length());
        dock.setStorageString("parameters", iconData->parameters.c_str(), (uint)iconData->parameters.length());
        BITMAP bm;
        if(GetObject(icon->icon, sizeof(BITMAP), &bm))
        {
          dock.setStorageData("iconHeader", (char*)&bm, sizeof(bm) - sizeof(LPVOID));
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

        iconData->launcherIndex = index;
        iconData->pinned = true;
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

int Launcher::handleMoveEvent(Icon* icon)
{
  IconData* iconData = (IconData*) icon->userData;
  Dock& dock = iconData->launcher.dock;
  int launcherIndex = -1;
  for(Icon* i = dock.getFirstIcon(); i; i = dock.getNextIcon(i))
  {
    IconData* iconData = (IconData*) i->userData;
    if(iconData->launcherIndex >= 0)
    {
      ++launcherIndex;
      if(iconData->launcherIndex != launcherIndex)
      {
        for(Icon* j = dock.getNextIcon(i); j; j = dock.getNextIcon(j))
          if(((IconData*) j->userData)->launcherIndex == launcherIndex)
          {
            dock.swapStorageNumSections(iconData->launcherIndex, launcherIndex);
            ((IconData*) j->userData)->launcherIndex = iconData->launcherIndex;
            iconData->launcherIndex = launcherIndex;
            break;
          }
        assert(iconData->launcherIndex == launcherIndex);
      }
    }
  }
  return 0;
}

IconData::IconData(Launcher& launcher, HICON hicon, Icon* icon, HWND hwnd, const std::wstring& path, const std::wstring& parameters, int launcherIndex) :
  launcher(launcher), hicon(hicon), icon(icon), hwnd(hwnd), path(path), parameters(parameters), pinned(false), launcherIndex(launcherIndex) {}

IconData::~IconData()
{
  if(icon)
  {
    if(icon->icon)
      DeleteObject(icon->icon);
    launcher.dock.destroyIcon(icon);
  }
}
