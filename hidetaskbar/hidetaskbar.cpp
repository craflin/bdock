// HideTaskBar.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

int HideTaskBar::instances = 0;
bool HideTaskBar::hidden = false;
LPARAM HideTaskBar::originalState;

HideTaskBar::HideTaskBar()
{
  ++instances;
} 

HideTaskBar::~HideTaskBar()
{
  --instances;
  if(instances == 0 && hidden)
  { // unhide
    showTaskBar(true, originalState);
  }
}

bool HideTaskBar::init()
{
  if(!hidden)
  {
	  if(!showTaskBar(false, originalState))
      return false;
    hidden = true;
  }
  return true;
}

bool HideTaskBar::showTaskBar(bool show, LPARAM& state) const
{
  APPBARDATA	apd = {0};

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
