
#include "stdafx.h"

Skin::Bitmap::Bitmap() : bmp(0) {}

Skin::Bitmap::~Bitmap()
{
  if(bmp)
    DeleteObject(bmp);
}

bool Skin::Bitmap::load(const wchar* file)
{
  ASSERT(!bmp);
  bmp = (HBITMAP) LoadImage(NULL, file, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
  if(!bmp)
    return false;

  BITMAP bm;
  ::GetObject(bmp, sizeof( bm ), &bm);
  size.cx = bm.bmWidth;
  size.cy = bm.bmHeight;
  return true;
}


void Skin::Bitmap::draw(HDC dest, int x, int y) const
{
  HDC tmp = CreateCompatibleDC(NULL);
  HBITMAP oldBmp = (HBITMAP)SelectObject(tmp, bmp);

  BLENDFUNCTION bf;
  bf.AlphaFormat = AC_SRC_ALPHA;
  bf.BlendFlags = 0;
  bf.BlendOp = AC_SRC_OVER;
  bf.SourceConstantAlpha = 160;
  AlphaBlend(dest, x, y, size.cx, size.cy, tmp, 0, 0, size.cx, size.cy, bf);

  SelectObject(tmp, oldBmp);
  DeleteDC(tmp);
}

Skin::Skin() : bgBrush(CreateSolidBrush(0)), bg(0) {}

Skin::~Skin()
{
  if(bgBrush)
    DeleteObject(bgBrush);
  if(bg)
    DeleteObject(bg);
}

bool Skin::init(const String& name)
{
  String file(_T("Skins/"));
  file += name;
  file += _T('/');
  uint_t flen = file.length();
 
  file.resize(flen);
  file += _T("leftBg.bmp");
  leftBg.load(file);

  file.resize(flen);
  file += _T("rightBg.bmp");
  rightBg.load(file);

  file.resize(flen);
  file += _T("midBg.bmp");
  midBg.load(file);

  file.resize(flen);
  file += _T("activeBg.bmp");
  activeBg.load(file);

  file.resize(flen);
  file += _T("defaultBg.bmp");
  defaultBg.load(file);

  file.resize(flen);
  file += _T("hotBg.bmp");
  hotBg.load(file);

  file.resize(flen);
  file += _T("fullBg.bmp");
  fullBg.load(file);

  file.resize(flen);
  file += _T("halfBg.bmp");
  halfBg.load(file);

  return true;
}

void Skin::draw(HDC dest, const SIZE& size, const RECT& update)
{
  FillRect(dest, &update, bgBrush);

  if(!leftBg.bmp || !rightBg.bmp || !midBg.bmp)
    return;

  if(!bg || size.cx != bgSize.cx || size.cy != bgSize.cy)
  {
    if(bg)
      DeleteObject(bg);

    HDC screen = GetDC(NULL);
    bg = CreateCompatibleBitmap(screen, size.cx, size.cy);
    bgSize.cx = size.cx;
    bgSize.cy = size.cy;
    HDC dest = CreateCompatibleDC(NULL);
    HBITMAP oldDestBmp = (HBITMAP)SelectObject(dest, bg);
    HDC src = CreateCompatibleDC(NULL);
    HBRUSH trans = CreateSolidBrush(RGB(0, 0, 0));

    HBITMAP oldSrcBmp = (HBITMAP)SelectObject(src, leftBg.bmp);
    BitBlt(dest, 0, size.cy - leftBg.size.cy, leftBg.size.cx, leftBg.size.cy, src, 0, 0, SRCCOPY);
    if(size.cy > leftBg.size.cy)
    {
      RECT rect = { 0, 0, leftBg.size.cx, size.cy - leftBg.size.cy};
      FillRect(dest, &rect, trans);
    }

    SelectObject(src, midBg.bmp);
    StretchBlt(dest, leftBg.size.cx, size.cy - midBg.size.cy, size.cx - leftBg.size.cx - rightBg.size.cx, midBg.size.cy, 
      src, 0, 0, midBg.size.cx, midBg.size.cy, SRCCOPY);
    if(size.cy > midBg.size.cy)
    {
      RECT rect = { leftBg.size.cx, 0, size.cx - rightBg.size.cx, size.cy - midBg.size.cy};
      FillRect(dest, &rect, trans);
    }

    SelectObject(src, rightBg.bmp);
    BitBlt(dest, size.cx - rightBg.size.cx, size.cy - rightBg.size.cy, rightBg.size.cx, rightBg.size.cy, src, 0, 0, SRCCOPY);
    if(size.cy > rightBg.size.cy)
    {
      RECT rect = { 0, 0, size.cx - rightBg.size.cx, size.cy - rightBg.size.cy};
      FillRect(dest, &rect, trans);
    }

    SelectObject(dest, oldDestBmp);
    DeleteDC(dest);
    DeleteDC(screen);
    SelectObject(src, oldSrcBmp);
    DeleteDC(src);
    DeleteObject(trans);
  }

  HDC src = CreateCompatibleDC(NULL);
  HBITMAP oldBmp = (HBITMAP)SelectObject(src, bg);

  BitBlt(dest, update.left, update.top, update.right - update.left, update.bottom - update.top, src, 
    update.left, update.top, SRCCOPY);
  
  SelectObject(src, oldBmp);
  DeleteDC(src);
}
