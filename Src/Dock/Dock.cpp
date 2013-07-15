

#include "stdafx.h"

ATOM Dock::wndClass = 0;
HINSTANCE Dock::hinstance = 0;

Dock::Dock(Storage& globalStorage, Storage* dockStorage) : globalStorage(globalStorage), storage(dockStorage), settings(dockStorage), hwnd(0), skin(0), iconCount(0), firstIcon(0), lastIcon(0), lastHitIcon(0),
activeHwnd(0), activeHwndRudeFullscreen(false) {}

Dock::~Dock()
{
  if(hwnd)
  {
    DeregisterShellHookWindow(hwnd);
    DestroyWindow(hwnd);
    hwnd = 0;
  }
  if(skin)
    delete skin;
  while(plugins.begin() != plugins.end())
    deletePlugin(*plugins.begin());
  ASSERT(iconCount == 0);
}

bool Dock::init(HINSTANCE hinstance)
{
  ASSERT(!hwnd);
  ASSERT(!Dock::hinstance || Dock::hinstance == hinstance);
  Dock::hinstance = hinstance;

  // register window class
  if(!wndClass)
  {
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style      = CS_DBLCLKS; //CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc  = wndProc;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = 0;
    wcex.hInstance    = hinstance;
    wcex.hIcon      = LoadIcon(hinstance, MAKEINTRESOURCE(IDI_BDOCK));
    wcex.hCursor    = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = 0; //(HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName  = 0; //MAKEINTRESOURCE(IDC_BDOCK);
    wcex.lpszClassName  = _T("BDOCK");
    wcex.hIconSm    = LoadIcon(hinstance, MAKEINTRESOURCE(IDI_SMALL));

    wndClass = RegisterClassEx(&wcex);
    if(!wndClass)
      return false;
  }

  // load skin
  if(!loadSkin(L"Default"))
  {
    // TODO: error message
    return false;
  }

  // load plugins
  for(int i = 0, count = storage->getNumSectionCount(); i < count; ++i)
  {
    storage->enterNumSection(i);
    const wchar* name = storage->getStr("name");
    if(name)
      if(!loadPlugin(name, storage->getSection("config")))
      {
        // TODO: error message
      }
    storage->leave();
  }

  // create window
  hwnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_NOACTIVATE | WS_EX_TOPMOST, _T("BDOCK"), _T(""), WS_POPUP,
    0, 0, 0, 0, NULL, NULL, hinstance, this);
  if (!hwnd)
    return false;
  for(std::unordered_set<Timer*>::iterator i = timers.begin(), end = timers.end(); i != end; ++i)
    SetTimer(hwnd,(UINT_PTR)*i, (*i)->interval, 0);

  if(!RegisterShellHookWindow(hwnd))
    return false;

  // show window
  calcIconRects(0, firstIcon);
  ShowWindow(hwnd, SW_SHOW);
  update();
  //UpdateWindow(hwnd);

   return true;
}

bool Dock::loadSkin(const wchar* name)
{
  Skin* skin = new Skin;
  if(!skin->init(name))
  {
    delete skin;
    return false;
  }
  if(this->skin)
    delete this->skin;
  this->skin = skin;
  return true;
}

bool Dock::loadPlugin(const wchar* name, Storage* storage)
{
  Plugin* plugin = new Plugin(this, storage);
  if(!plugin->init(name))
  {
    delete plugin;
    return false;
  }
  addPlugin(plugin);
  return true;
}

void Dock::deletePlugin(Plugin* plugin)
{
  removePlugin(plugin);
  delete plugin;
}

void Dock::update()
{
  update((RECT*)0);
}

void Dock::updateIcon(Icon* icon)
{
  update(&icon->rect);
}

void Dock::draw(HDC dest, const RECT& update)
{
  skin->draw(dest, size, update);

  for(Icon* icon = firstIcon; icon; icon = icon->next)
  {
    const RECT& rect(icon->rect);
    if(rect.left >= update.left && rect.right <= update.right)
    {
      if(icon->flags & IF_ACTIVE && skin->activeBg.bmp)
      {
        const SIZE& size(skin->activeBg.size);
        skin->activeBg.draw(dest, rect.left + (settings.itemWidth - size.cx) / 2, settings.topMargin + (settings.iconHeight - size.cx) / 2);
      }
      else
      {
        if(!(icon->flags & (IF_HALFBG | IF_FULLBG)) && skin->defaultBg.bmp)
        {
          const SIZE& size(skin->activeBg.size);
          skin->defaultBg.draw(dest, rect.left + (settings.itemWidth - size.cx) / 2, settings.topMargin + (settings.iconHeight - size.cx) / 2);
        }
        if(icon->flags & IF_HALFBG && skin->halfBg.bmp)
        {
          const SIZE& size(skin->halfBg.size);
          skin->halfBg.draw(dest, rect.left + (settings.itemWidth - size.cx) / 2, settings.topMargin + settings.iconHeight / 2 + (settings.iconHeight - size.cx) / 2);
        }
        if(icon->flags & IF_FULLBG && skin->fullBg.bmp)
        {
          const SIZE& size(skin->fullBg.size);
          skin->fullBg.draw(dest, rect.left + (settings.itemWidth - size.cx) / 2, settings.topMargin + (settings.iconHeight - size.cx) / 2);
        }
      }
      icon->draw(dest, settings);
    }
  }
}

void Dock::update(RECT* update)
{
  if(!hwnd || !IsWindowVisible(hwnd))
    return;

  bool updateAll = false;
  SIZE size;
  {

    size.cx = (lastIcon ? lastIcon->rect.right : settings.leftMargin) + settings.rightMargin;
    if(skin->leftBg.bmp && skin->rightBg.bmp && skin->midBg.bmp)
      if(size.cx < skin->leftBg.size.cx + skin->rightBg.size.cx)
        size.cx = skin->leftBg.size.cx + skin->rightBg.size.cx;
    size.cy = settings.barHeight;
  }

  if(!bmp || size.cy != this->size.cy || size.cx != this->size.cx)
  {
    if(bmp)
      DeleteObject(bmp);
    updateAll = true;

    HDC screen = GetDC(NULL);
    bmp = CreateCompatibleBitmap(screen, size.cx, size.cy);
    this->size.cx = size.cx;
    this->size.cy = size.cy;
    DeleteDC(screen);
  }

  RECT updateRect;
  if(updateAll || !update)
  {
    updateRect.left = updateRect.top = 0;
    updateRect.right = size.cx;
    updateRect.bottom = size.cy;
  }
  else
  {
    updateRect = *update;
  }

  HDC dest = CreateCompatibleDC(NULL);
  HBITMAP oldBmp = (HBITMAP)SelectObject(dest, bmp);

  draw(dest, updateRect);
   
  POINT point = { 0, 0 };
  if(updateAll)
  {
    /*
    RECT rectWortArea;
    SystemParametersInfo(SPI_GETWORKAREA,sizeof(RECT),&rectWortArea,0);
    pos.x = rectWortArea.left + (rectWortArea.right - rectWortArea.left - size.cx) / 2;
    pos.y = rectWortArea.bottom - size.cy;
    */
    
    switch(settings.edge)
    {
    case Settings::top:
      pos.y = 0;
      break;
    default:
      pos.y = GetSystemMetrics(SM_CYSCREEN) - size.cy;
      break;
    }
    switch(settings.alignment)
    {
    case Settings::left:
      pos.x = 0;
      break;
    case Settings::right:
      pos.x = GetSystemMetrics(SM_CXSCREEN) - size.cx;
      break;
    default:
      pos.x = (GetSystemMetrics(SM_CXSCREEN) - size.cx) / 2;
      break;
    }
  }
  BLENDFUNCTION bf;
  bf.AlphaFormat = AC_SRC_ALPHA;
  bf.BlendFlags = 0;
  bf.BlendOp = AC_SRC_OVER;
  bf.SourceConstantAlpha = 255;
  UPDATELAYEREDWINDOWINFO ulwi;
  ulwi.cbSize = sizeof(UPDATELAYEREDWINDOWINFO);
  ulwi.hdcDst = NULL;
  ulwi.pptDst = &pos;
  ulwi.psize = &size;
  ulwi.hdcSrc = dest;
  ulwi.pptSrc = &point;
  ulwi.crKey = 0;
  ulwi.pblend = &bf;
  ulwi.dwFlags = ULW_ALPHA;
  ulwi.prcDirty = &updateRect;
  UpdateLayeredWindowIndirect(hwnd, &ulwi);
  //UpdateLayeredWindow(hwnd, NULL, &pos, &size, dest, &point, RGB(255, 255, 255), &bf, ULW_ALPHA);

  SelectObject(dest, oldBmp);
  DeleteDC(dest);
}

void Dock::handleContextMenu(int x, int y)
{
  POINT pt = { x, y };
  ScreenToClient(hwnd, &pt); 

  Icon* icon = hitTest(pt.x, pt.y);
  if(icon && icon->handleMouseEvent && icon->handleMouseEvent(icon, WM_CONTEXTMENU, x, y) == 0)
    return;

  /*
  HMENU menu = LoadMenu(hinstance, MAKEINTRESOURCE(IDC_BDOCK));
  SetForegroundWindow(hwnd);
  TrackPopupMenu(GetSubMenu(menu, 0), 
            TPM_LEFTALIGN | TPM_RIGHTBUTTON, 
            x, y, 0, hwnd, NULL); 
  DestroyMenu(menu);
  */
  showMenu(0, x, y);
}

Icon* Dock::hitTest(int x, int y)
{  
  Icon* hitIcon = 0;
  if(lastHitIcon)
  {
    const RECT& rect(lastHitIcon->rect);
    if(x >= rect.left && x < rect.right && y >= rect.top && y < rect.bottom)
      hitIcon = lastHitIcon;
  }
  if(!hitIcon)
    for(Icon* icon = firstIcon; icon; icon = icon->next)
      if(icon != lastHitIcon)
      {
        const RECT& rect(icon->rect);
        if(x >= rect.left && x < rect.right && y >= rect.top && y < rect.bottom)
          hitIcon = icon;
      }
  lastHitIcon = hitIcon;
  return hitIcon;
}

bool Dock::handleMouseEvent(UINT message, int x, int y)
{
  Icon* icon = hitTest(x, y);
  if(icon && icon->handleMouseEvent && icon->handleMouseEvent(icon, message, x + pos.x, y + pos.y) == 0)
    return true;
  return false;
}

LRESULT CALLBACK Dock::wndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
  case WM_CREATE:
    {
      LPCREATESTRUCT cs = (LPCREATESTRUCT)lParam;
      SetWindowLong(hwnd, GWLP_USERDATA, (LONG)cs->lpCreateParams);
    }
    break;
  case WM_TIMER:
    if(wParam == 0) // check full screen app timer
    {
        bool hideWindow = isFullscreen(GetForegroundWindow());
        if(!!IsWindowVisible(hwnd) == hideWindow)
        {
          if(!hideWindow)
          {
            Dock* dock = (Dock*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            LONG windowStyle = GetWindowLong(hwnd, GWL_STYLE);
            SetWindowLong(hwnd, GWL_STYLE, windowStyle | WS_VISIBLE);
            dock->update(0);
            SetWindowLong(hwnd, GWL_STYLE, windowStyle);
            ShowWindow(hwnd, SW_SHOW);
          }
          else
            ShowWindow(hwnd, SW_HIDE);
        }
    }
    if(wParam != 0)
    {
      Dock* dock = (Dock*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
      if(dock->timers.find((Timer*)wParam) != dock->timers.end())
      {
        Timer* timer = (Timer*)wParam;
        timer->handleTimerEvent(timer);
      }
    }
    break;
  case WM_COMMAND:
    {
      switch (LOWORD(wParam))
      {
      case IDM_ABOUT:
        AboutDlg().show(hwnd);
        break;
      case IDM_SETTINGS:
        {
          Dock* dock = (Dock*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
          dock->showSettingsDlg();
        }
        break;
      case IDM_EXIT:
        PostQuitMessage(0);
        break;
      default:
        return DefWindowProc(hwnd, message, wParam, lParam);
      }
    }
    break;
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
    if(!((Dock*)GetWindowLongPtr(hwnd, GWLP_USERDATA))->handleMouseEvent(message, LOWORD(lParam), HIWORD(lParam)))
      return DefWindowProc(hwnd, message, wParam, lParam);
    break;
  case WM_CONTEXTMENU:
    ((Dock*)GetWindowLongPtr(hwnd, GWLP_USERDATA))->handleContextMenu(LOWORD(lParam), HIWORD(lParam));
    break;
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  default:
    static unsigned int wm_shellhook = RegisterWindowMessage(L"SHELLHOOK");
    if(message == wm_shellhook)
    {
      switch(wParam)
      {
      case HSHELL_RUDEAPPACTIVATED:
      case HSHELL_WINDOWACTIVATED:
        {
          KillTimer(hwnd, 0);
          SetTimer(hwnd, 0, 300, 0);
          /*
          bool hideWindow = isFullscreen((HWND)lParam) && wParam == HSHELL_RUDEAPPACTIVATED;
          if(!!IsWindowVisible(hwnd) == hideWindow)
          {
            if(!hideWindow)
            {
              Dock* dock = (Dock*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
              LONG windowStyle = GetWindowLong(hwnd, GWL_STYLE);
              SetWindowLong(hwnd, GWL_STYLE, windowStyle | WS_VISIBLE);
              dock->update(0);
              SetWindowLong(hwnd, GWL_STYLE, windowStyle);
              ShowWindow(hwnd, SW_SHOW);
            }
            else
              ShowWindow(hwnd, SW_HIDE);
          }
          */
        }
        break;
      }
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
  }
  return 0;
}

DWORD Dock::showMenu(HMENU hmenu, int x, int y)
{
  bool deleteHMenu = false;
  if(!hmenu)
  {
    hmenu = CreatePopupMenu();
    deleteHMenu = true;
  }

  SetForegroundWindow(hwnd);

  HMENU menu = LoadMenu(hinstance, MAKEINTRESOURCE(IDC_BDOCK));
  if(!deleteHMenu)
    InsertMenu(hmenu, 0, MF_BYPOSITION | MF_SEPARATOR, NULL, NULL);
  wchar name[64];
  GetMenuString(menu, 0, name, sizeof(name), MF_BYPOSITION); 
  InsertMenu(hmenu, 0, MF_BYPOSITION | MF_POPUP, (UINT_PTR)GetSubMenu(menu, 0), name);

  DWORD cmd = (DWORD)TrackPopupMenu(hmenu, 
                TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, 
                x, y, 0, hwnd, NULL);
  RemoveMenu(hmenu, 1, MF_BYPOSITION);
  RemoveMenu(hmenu, 0, MF_BYPOSITION);

  if(cmd)
  {
    MENUITEMINFO mii;
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_FTYPE;
    if(GetMenuItemInfo(menu, cmd, FALSE, &mii))
    {      
      SendMessage(hwnd, WM_COMMAND, cmd, NULL);
      cmd = 0;
    }
  }

  DestroyMenu(menu);
  if(deleteHMenu)
    DestroyMenu(hmenu);
  return cmd;
}

void Dock::addIcon(Icon* icon)
{
  ++iconCount;
  if(!icon->previous)
    firstIcon = icon;
  if(!icon->next)
    lastIcon = icon;

  if(hwnd)
    calcIconRects(icon->previous, icon);
}

void Dock::removeIcon(Icon* icon)
{
  --iconCount;
  if(icon == firstIcon)
    firstIcon = icon->next;
  if(icon == lastIcon)
    lastIcon = icon->previous;
  if(icon == lastHitIcon)
    lastHitIcon = 0;

  if(hwnd)
    calcIconRects(icon->previous, icon->next);
}

void Dock::calcIconRects(Icon* previous, Icon* firstToUpdate)
{
  RECT rect = { settings.leftMargin, 0, 0, settings.barHeight };
  if(previous)
    rect.left = previous->rect.right;
  for(Icon* icon = firstToUpdate; icon; icon = icon->next)
  {
    rect.right = rect.left + (icon->flags & IF_SMALL ? settings.itemWidth / 2 : settings.itemWidth);
    icon->rect = rect;
    rect.left = rect.right;
  }
}

void Dock::addTimer(Timer* timer)
{
  timers.insert(timer);
  if(hwnd)
    SetTimer(hwnd, (UINT_PTR)timer, timer->interval, 0);
}

void Dock::removeTimer(Timer* timer)
{
  timers.erase(timer);
  if(hwnd)
    KillTimer(hwnd, (UINT_PTR)timer);
}


void Dock::updateTimer(Timer* timer)
{
  if(hwnd)
  {
    KillTimer(hwnd, (UINT_PTR)timer);
    SetTimer(hwnd, (UINT_PTR)timer, timer->interval, 0);
  }
}

bool Dock::isFullscreen(HWND hwnd)
{
  HMONITOR hMon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONULL);
  if(!hMon)
    return false;
  MONITORINFO mi;
  mi.cbSize = sizeof(mi);
  if(!GetMonitorInfo(hMon, &mi))
    return false;

  RECT clientRect;
  if(!GetClientRect(hwnd, &clientRect))
    return false;
  POINT pt = {0};
  if(!ClientToScreen(hwnd, &pt))
    return false;
  clientRect.left += pt.x;
  clientRect.right += pt.x;
  clientRect.top += pt.y;
  clientRect.bottom += pt.y;
  
  if(!(clientRect.left <= mi.rcWork.left && clientRect.top <= mi.rcWork.top && clientRect.right >= mi.rcWork.right && clientRect.bottom >= mi.rcWork.bottom))
    return false;

  DWORD exstyle = GetWindowLong(hwnd, GWL_EXSTYLE);
  HANDLE owner = GetWindow(hwnd, GW_OWNER);
  if((!(exstyle & WS_EX_TOOLWINDOW) && owner == 0) || (exstyle & WS_EX_APPWINDOW && owner != 0))
    return true;
  return false;
}

bool Dock::showSettingsDlg()
{
  TCHAR startupLinkFilePath[MAX_PATH];
  VERIFY(WinAPI::Shell::getFolderPath(CSIDL_STARTUP, startupLinkFilePath, MAX_PATH));
  _tcscat_s(startupLinkFilePath, _T("\\BDock.lnk"));
  
  bool startup = GetFileAttributes(startupLinkFilePath) != INVALID_FILE_ATTRIBUTES;
  globalStorage.setUInt("autostart", startup);

  if(SettingsDlg(globalStorage).show(hwnd) != IDOK)
    return false;

  if(globalStorage.getUInt("autostart", 0))
  {
    if(GetFileAttributes(startupLinkFilePath) == INVALID_FILE_ATTRIBUTES)
    {
      TCHAR moduleFileName[MAX_PATH];
      GetModuleFileName(WinAPI::Application::getInstance(), moduleFileName, MAX_PATH);
      WinAPI::Shell::createLink(moduleFileName, startupLinkFilePath, _T(""));
    }
  }
  else
  {
    if(GetFileAttributes(startupLinkFilePath) != INVALID_FILE_ATTRIBUTES)
      DeleteFile(startupLinkFilePath);
  }


  return true;
}
