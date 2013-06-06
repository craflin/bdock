// HideTaskBar.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

extern HMODULE hmodule;

int HideTaskBar::instances = 0;
bool HideTaskBar::hidden = false;
LPARAM HideTaskBar::originalState;

ATOM HideTaskBar::wndClass = 0;
HWND HideTaskBar::hwnd = 0;

HideTaskBar::HideTaskBar()
{
  ++instances;
} 

bool HideTaskBar::init()
{
  if(!hidden)
  {
    if(!showTaskBar(false, originalState))
      return false;
    hidden = true;
  }
  
  if(!hwnd)
  {
    // register system menu monitor window class
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
      wcex.lpszClassName  = L"BDOCKSysMenuMon";
      wcex.hIconSm    = 0;
      wndClass = RegisterClassEx(&wcex);
      if(!wndClass)
        return false;
    }

    // create system menu monitor window
    hwnd = CreateWindowEx(NULL, L"BDOCKSysMenuMon", NULL, NULL, NULL, NULL, NULL, NULL, HWND_MESSAGE, NULL, hmodule, this);
    if(!hwnd)
      return false;
    if(!RegisterShellHookWindow(hwnd))
      return false;
  }
  return true;
}

HideTaskBar::~HideTaskBar()
{
  --instances;
  if(instances == 0)
  {
      // unhide
      if(hidden)
      {
        showTaskBar(true, originalState);
        hidden = false;
      }

      // destroy system menu monitor window
      if(hwnd)
      {
        DeregisterShellHookWindow(hwnd);
        DestroyWindow(hwnd);
        hwnd = 0;
      }
  }
}

bool HideTaskBar::showTaskBar(bool show, LPARAM& state)
{
  APPBARDATA  apd = {0};

  apd.cbSize = sizeof(apd);
  apd.hWnd = FindWindow(L"Shell_TrayWnd", NULL);
  if(!apd.hWnd)
    return false;

  if(!show)
    state = SHAppBarMessage(ABM_GETSTATE, &apd);
  apd.lParam = state;

  if(!show)
  {
    apd.lParam |= ABS_AUTOHIDE;
    apd.lParam &= ~ABS_ALWAYSONTOP;
  }

  if(show)
  { 
    ShowWindow(apd.hWnd, SW_SHOW);
    SetWindowRgn(apd.hWnd, 0, true);
    ShowWindow(FindWindow(L"Button", L"Start"), SW_SHOW);
    
  }

  SHAppBarMessage(ABM_SETSTATE, &apd);

  if(!show)
  {
    ShowWindow(apd.hWnd, SW_HIDE);
    HRGN rgn = CreateRectRgn(0, 0, 0, 0); 
    SetWindowRgn(apd.hWnd, rgn, true);
    ShowWindow(FindWindow(L"Button", L"Start"), SW_HIDE);
  }
  return true;
}

LRESULT CALLBACK HideTaskBar::wndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  static unsigned int wm_shellhook = RegisterWindowMessage(L"SHELLHOOK");
  if(message == wm_shellhook)
    switch(wParam)
    {
    case HSHELL_RUDEAPPACTIVATED:
    case HSHELL_WINDOWACTIVATED:
      {
        HWND activeWnd = (HWND) lParam;
        bool hideTaskBar = activeWnd != 0;
        if(!hideTaskBar)
        {
          HWND startMenu = FindWindowExA(GetDesktopWindow(), 0, 0, "Start menu");
          if(!startMenu || !IsWindowVisible(startMenu))
            hideTaskBar = true;
        }
        if(hideTaskBar != hidden)
        {
          if(!showTaskBar(!hideTaskBar, HideTaskBar::originalState))
            return false;
          hidden = hideTaskBar;
        }
      }
      break;
    }
  return DefWindowProc(hwnd, message, wParam, lParam);
}


