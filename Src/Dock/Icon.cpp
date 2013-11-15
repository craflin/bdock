
#include "stdafx.h"

Icon::Icon(HBITMAP icon, uint_t flags, Plugin* plugin) : 
  plugin(plugin)
{
  memset((API::Icon*)this, 0, sizeof(API::Icon));
  this->icon = icon;
  this->flags = flags;
}

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