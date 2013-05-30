
#include "stdafx.h"

Icon::Icon(HBITMAP icon, uint flags, Plugin* plugin, Icon* insertAfter, Icon* first) : 
  plugin(plugin), previous(insertAfter), next(insertAfter ? insertAfter->next : first)
{
  memset((API::Icon*)this, 0, sizeof(API::Icon));
  this->icon = icon;
  this->flags = flags;

  if(previous)
    previous->next = this;
  if(next)
    next->previous = this;
}

Icon::~Icon()
{
  if(previous)
    previous->next = next;
  if(next)
    next->previous = previous;
}

// TODO: optimize this function using DIB-Bitmap!
extern "C" BDOCK_API HBITMAP createBitmapFromIcon(HICON icon, SIZE* size)
{
  HBITMAP bmp = 0;
  ICONINFO ii;
  if(!GetIconInfo(icon, &ii))
    return 0;

  BITMAP bm;
  if(!GetObject(ii.hbmColor, sizeof(BITMAP), &bm))
    goto abruption;
  if(bm.bmWidth > 128 || bm.bmHeight > 128)
    goto abruption;

  bmp = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 32, NULL);
  if(size)
  {
    size->cx = bm.bmWidth;
    size->cy = bm.bmHeight;
  }
  HDC dest = CreateCompatibleDC(NULL);
  HBITMAP oldBmp = (HBITMAP)SelectObject(dest, bmp);

  // clear bitmap
  HBRUSH trans = CreateSolidBrush(0);
  { RECT rect = { 0, 0, bm.bmWidth, bm.bmHeight };
  FillRect(dest, &rect, trans); }
  DeleteObject(trans);
  
  // draw icon
  //DrawIcon(dest, 0, 0, icon);
  DrawIconEx(dest, 0, 0, icon, bm.bmWidth, bm.bmHeight, 0, NULL, DI_NORMAL);
  SelectObject(dest, oldBmp);
  DeleteDC(dest);

  // read bitmap
  if(!GetObject(bmp, sizeof(BITMAP), &bm))
    goto abruption;
  char buffer[128 * 128* 4];
  if(!GetBitmapBits(bmp, bm.bmWidthBytes * bm.bmHeight, &buffer))
    goto abruption;

  // try to find alpha information
  DWORD* pos, * end;
  for(int y = bm.bmHeight - 1; y >= 0; --y)
  {
    pos = (DWORD*)&buffer[y * bm.bmWidthBytes];
    end = pos + bm.bmWidth;

    for(; pos < end; ++pos)
      if(*pos & 0xff000000)
      { // found 
        DeleteObject(ii.hbmColor);
        DeleteObject(ii.hbmMask);
        return bmp;
      }
  }

  // there was no alpha information
  // try to apply the icon mask

  BITMAP bmMask;
  if(!GetObject(ii.hbmMask, sizeof(BITMAP), &bmMask))
    goto abruption;

  unsigned char mask[128 * 128 / 8];
  if(bmMask.bmBitsPixel != 1 || bmMask.bmWidthBytes * bmMask.bmHeight > 128 * 128 / 8)
    goto abruption;

  if(!GetBitmapBits(ii.hbmMask, bmMask.bmWidthBytes * bmMask.bmHeight, mask))
    goto abruption;

  unsigned char* maskPos;
  int maskSubPos;
  for(int y = bm.bmHeight - 1; y >= 0; --y)
  {
    pos = (DWORD*)&buffer[y * bm.bmWidthBytes];
    end = pos + bm.bmWidth;
    maskPos = &mask[y * bmMask.bmWidthBytes];
    maskSubPos = 7;
    for(; pos < end; ++pos)
    {
      if(*maskPos & (1 << maskSubPos)) // transparent
      {
        *pos = 0x0;
      }
      else // not transparent
      {        
        *pos |= 0xff000000;
      }
      if(--maskSubPos < 0)
      {
        maskSubPos = 7;
        ++maskPos;
      }
    }
  }

  if(!SetBitmapBits(bmp, bm.bmWidthBytes * bm.bmHeight, buffer))
    goto abruption;

  DeleteObject(ii.hbmColor);
  DeleteObject(ii.hbmMask);
  return bmp;

abruption:
  DeleteObject(ii.hbmColor);
  DeleteObject(ii.hbmMask);
  if(bmp)
    DeleteObject(bmp);
  return 0;
}

#if 0
HBITMAP CreateBitmapFromIcon(HICON icon)
{
  ICONINFO ii;
  if(!GetIconInfo(icon, &ii))
    return 0;

  BITMAP bmMask;
  if(!GetObject(ii.hbmMask, sizeof(BITMAP), &bmMask))
    goto abruption;

  unsigned char mask[128 * 128 / 8];
  if(bmMask.bmBitsPixel != 1 || bmMask.bmWidthBytes * bmMask.bmHeight > 128 * 128 / 8)
    goto abruption;

  BITMAP bmColor;
  if(!GetObject(ii.hbmColor, sizeof(BITMAP), &bmColor))
    goto abruption;

  char color[128 * 128 * 4];
  if(bmColor.bmBitsPixel != 32 || bmColor.bmWidthBytes * bmColor.bmHeight > 128 * 128 * 4)
    goto abruption;

  if(!GetBitmapBits(ii.hbmMask, bmMask.bmWidthBytes * bmMask.bmHeight, mask) ||
     !GetBitmapBits(ii.hbmColor, bmColor.bmWidthBytes * bmColor.bmHeight, color))
    goto abruption;

  DWORD* pos, * end;
  unsigned char* maskPos;
  int maskSubPos;
  for(int y = bmColor.bmHeight - 1; y >= 0; --y)
  {
    pos = (DWORD*)&color[y * bmColor.bmWidthBytes];
    end = pos + bmColor.bmWidth;
    maskPos = &mask[y * bmMask.bmWidthBytes];
    maskSubPos = 7;
    for(; pos < end; ++pos)
    {
      if(*maskPos & (1 << maskSubPos)) // transparent
      {
        *pos = 0x0;
      }
      else // not transparent
      {
        if(*pos & 0xff000000) // alpha channel has the same opinion => ii.hbmColor is ok
        {
          //DeleteObject(ii.hbmMask);
          //return ii.hbmColor;
        }
        else // apply mask
        {
          *pos |= 0xff000000;
        }
      }
      if(--maskSubPos < 0)
      {
        maskSubPos = 7;
        ++maskPos;
      }
    }
  }

  if(!SetBitmapBits(ii.hbmColor, bmColor.bmWidthBytes * bmColor.bmHeight, color))
    goto abruption;

  DeleteObject(ii.hbmMask);
  return ii.hbmColor;

abruption:
    DeleteObject(ii.hbmColor);
    DeleteObject(ii.hbmMask);
    return 0;
}
#endif 


void Icon::draw(HDC dest, const Settings& settings)
{
  if(!icon)
    return;

  BITMAP bm;
  if(!GetObject(icon, sizeof(BITMAP), &bm))
    return;

  HDC tmp = CreateCompatibleDC(NULL);
  HBITMAP oldBmp = (HBITMAP)SelectObject(tmp, icon);

  RECT rect = { this->rect.left + (settings.itemWidth - settings.iconWidth) / 2, settings.topMargin };
  rect.right = rect.left + settings.iconWidth;
  rect.bottom = rect.top + settings.iconHeight;
  BLENDFUNCTION bf;
  bf.AlphaFormat = AC_SRC_ALPHA;
  bf.BlendFlags = 0;
  bf.BlendOp = AC_SRC_OVER;
  bf.SourceConstantAlpha = flags & IF_GHOST ? 160 : 255;
  if(flags & IF_SMALL)
    AlphaBlend(dest, rect.left, rect.top + settings.iconHeight / 2, settings.iconWidth / 2, settings.iconHeight / 2, tmp, 0, 0, bm.bmWidth, bm.bmHeight, bf);
  else
    AlphaBlend(dest, rect.left, rect.top, settings.iconWidth, settings.iconHeight, tmp, 0, 0, bm.bmWidth, bm.bmHeight, bf);

  bf.SourceConstantAlpha = 120;
  int iconHeight = flags & IF_SMALL ? (settings.iconHeight / 2) : settings.iconHeight;
  for(int y = rect.top + iconHeight; y < settings.barHeight - settings.bottomMargin; ++y)
  {
    int yLine = iconHeight - (y - (rect.top + iconHeight)) - 1;
    if(yLine < 0)
      break;
    if(iconHeight != bm.bmHeight)
      yLine = int(float(yLine) * float(bm.bmHeight) / float(iconHeight));
    AlphaBlend(dest, rect.left, y + (flags & IF_SMALL ? (settings.iconHeight / 2) : 0), flags & IF_SMALL ? (settings.iconWidth / 2) : settings.iconWidth, 1, tmp, 0, yLine, bm.bmWidth, 1, bf);
    bf.SourceConstantAlpha -= 8;
    if(bf.SourceConstantAlpha == 0)
      break;
  }

  SelectObject(tmp, oldBmp);
  DeleteDC(tmp);
}