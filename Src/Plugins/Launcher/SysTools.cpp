
#include "stdafx.h"

HMENU CopyMenu(HMENU hmenu)
{
  MENUITEMINFO mii;
  mii.cbSize = sizeof(mii);
  mii.fMask = MIIM_STATE | MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_SUBMENU | MIIM_CHECKMARKS | MIIM_BITMAP;
  wchar* buf = 0;
  int buflen = 0;
  HMENU hmenuCopy = CreatePopupMenu();
  const int count = GetMenuItemCount(hmenu);
  for(int i = 0; i < count; ++i)
  {
    int len = GetMenuString(hmenu, i, NULL, 0, MF_BYPOSITION) + 1;
    if(buflen < len)
    {
      if(buf)
        free(buf);
      buf = (wchar*)malloc((len + 1) * sizeof(wchar));
      buflen = len;
    }
    buf[0] = 0;
    mii.dwTypeData = buf;
    mii.cch = len;

    GetMenuItemInfo(hmenu, i, TRUE, &mii);
    if(mii.hSubMenu)
      mii.hSubMenu = CopyMenu(mii.hSubMenu);
    InsertMenuItem(hmenuCopy, i, TRUE, &mii);
  }
  if(buf)
    free(buf);
  return hmenuCopy;
}

// TODO: optimize this function using DIB-Bitmap!
HBITMAP createBitmapFromIcon(HICON icon, SIZE* size)
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

