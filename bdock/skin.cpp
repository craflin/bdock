
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

Skin::Skin() : bgBrush(CreateSolidBrush(0)) {}

Skin::~Skin()
{
  if(bgBrush)
    DeleteObject(bgBrush);
}

bool Skin::init(const wchar* name)
{
  std::wstring file(L"skins/");
  file += name;
  file += L'/';
  size_t flen = file.length();
 
  file.resize(flen);
  file += L"leftBg.bmp";
  if(!leftBg.load(file.c_str()))
    return false;

  file.resize(flen);
  file += L"rightBg.bmp";
  if(!rightBg.load(file.c_str()))
    return false;

  file.resize(flen);
  file += L"midBg.bmp";
  if(!midBg.load(file.c_str()))
    return false;

  file.resize(flen);
  file += L"activeBg.bmp";
  if(!activeBg.load(file.c_str()))
    return false;

  file.resize(flen);
  file += L"fullBg.bmp";
  if(!fullBg.load(file.c_str()))
    return false;

  file.resize(flen);
  file += L"halfBg.bmp";
  if(!halfBg.load(file.c_str()))
    return false;

  return true;
}


void Skin::draw(HDC dest, const SIZE& size, const RECT& update)
{
  FillRect(dest, &update, bgBrush);

  /*

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

    HBITMAP oldSrcBmp = (HBITMAP)SelectObject(src, leftBg);
    BitBlt(dest, 0, size.cy - leftSize.cy, leftSize.cx, leftSize.cy, src, 0, 0, SRCCOPY);
    if(size.cy > leftSize.cy)
    {
      RECT rect = { 0, 0, leftSize.cx, size.cy - leftSize.cy};
      FillRect(dest, &rect, trans);
    }

    SelectObject(src, midBg);
    StretchBlt(dest, leftSize.cx, size.cy - midSize.cy, size.cx - leftSize.cx - rightSize.cx, midSize.cy, 
      src, 0, 0, midSize.cx, midSize.cy, SRCCOPY);
    if(size.cy > midSize.cy)
    {
      RECT rect = { leftSize.cx, 0, size.cx - rightSize.cx, size.cy - midSize.cy};
      FillRect(dest, &rect, trans);
    }

    SelectObject(src, rightBg);
    BitBlt(dest, size.cx - rightSize.cx, size.cy - rightSize.cy, rightSize.cx, rightSize.cy, src, 0, 0, SRCCOPY);
    if(size.cy > rightSize.cy)
    {
      RECT rect = { 0, 0, size.cx - rightSize.cx, size.cy - rightSize.cy};
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
  */
}
