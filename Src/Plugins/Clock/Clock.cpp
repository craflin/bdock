// HideTaskBar.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"


Clock::Clock(Dock& dock) : dock(dock), bitmap(0), font(0), bgBrush(0), icon(0), timer(0) {}

Clock::~Clock()
{
  if(bitmap)
    DeleteObject(bitmap);
  if(font)
    DeleteObject(font);
  if(bgBrush)
    DeleteObject(bgBrush);
}

bool Clock::init()
{
  // create dib bitmap
  BITMAPINFO bi;
  memset(&bi, 0, sizeof(BITMAPINFOHEADER));

  bi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
  bi.bmiHeader.biWidth       =  32;
  bi.bmiHeader.biHeight      = -32;  // negative --> top-down DIB
  bi.bmiHeader.biPlanes      = 1;
  bi.bmiHeader.biBitCount    = 32;
  bi.bmiHeader.biCompression = BI_RGB;

  HDC hdc = CreateCompatibleDC(NULL);
  bitmap = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, (void**)&bitmapData, NULL, 0);
  DeleteDC(hdc);
  if(!bitmap)
    return false;

  // create font
  NONCLIENTMETRICS ncm;
  ncm.cbSize = sizeof(NONCLIENTMETRICS);
  SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);
  ncm.lfMessageFont.lfQuality = ANTIALIASED_QUALITY;
  font = CreateFontIndirect(&ncm.lfMessageFont);  
  if(!font)
    return false;
  textColor = 0xffffff; //GetSysColor(COLOR_WINDOWTEXT);

  // create bg brush
  bgBrush = CreateSolidBrush(0);
  if(!bgBrush)
    return false;

  // draw clock
  update();

  // add clock icon to dock
  //createIcon(this, 0, IF_SMALL);
  icon = dock.createIcon(bitmap, IF_HALFBG);
  if(!icon)
    return false;

  // add update timer
  timer = dock.createTimer(60 * 1000 - st.wSecond * 1000 - st.wMilliseconds);
  if(!timer)
    return false;
  timer->handleTimerEvent = handleTimerEvent;
  timer->userData = this;

  return true;
}

int Clock::handleTimerEvent(Timer* timer)
{
  Clock* clock = (Clock*) timer->userData;
  clock->update();
  clock->dock.updateIcon(clock->icon);
  timer->interval = 60 * 1000 - clock->st.wSecond * 1000 - clock->st.wMilliseconds;
  clock->dock.updateTimer(timer);
  return 0;
}

void Clock::update()
{
  HDC dest = CreateCompatibleDC(NULL);
  HBITMAP oldBmp = (HBITMAP)SelectObject(dest, bitmap);
  HFONT oldFont = (HFONT)SelectObject(dest, font);
  SetBkMode(dest, OPAQUE);
  SetBkColor(dest, (~textColor) & 0xffffff);
  SetTextColor(dest, textColor);


  RECT rc = { 0, 0, 32, 32 };
  FillRect(dest, &rc, bgBrush);

  GetLocalTime(&st);
  wchar_t time[32];
  swprintf_s(time, L"%02u:%02u", st.wHour, st.wMinute);

  DrawText(dest, time, 5, &rc, DT_SINGLELINE | DT_CALCRECT);

  int height = rc.bottom - rc.top;
  rc.top = 16 + (16 - height) / 2;
  rc.bottom = rc.top + height;
  int width = rc.right - rc.left;
  rc.left = (32 - width) / 2;
  rc.right = rc.left + width;
  DrawText(dest, time, 5, &rc,  DT_SINGLELINE);

  SelectObject(dest, oldBmp);
  SelectObject(dest, oldFont);
  DeleteDC(dest);

  
  COLORREF bgColor = (~textColor) & 0xffffff;
  for(int i = rc.top, end = rc.bottom; i < end; ++i)
  {
    COLORREF* pos = bitmapData + i * 32 + rc.left;
    COLORREF* bmEnd = pos + width;
    for(; pos < bmEnd; ++pos)
    {      
      if((*pos & 0xffffff) == bgColor)
        *pos = 0x00000000;
      else
      {
        //tmp = *pos & 0xff | (*pos >> 8) & 0xff | (*pos >> 16) & 0xff;

        //*pos |=  ((*pos & 0xff) << 24) | 0xaaaaaa;
        *pos |= 0xff000000; 
        //*pos |= tmp << 24;
      }
    }
  }
}

