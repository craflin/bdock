
#include "stdafx.h"
/*
typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

LPFN_ISWOW64PROCESS fnIsWow64Process;

BOOL IsWow64()
{
    static BOOL bDetermined = FALSE;
    static BOOL bIsWow64 = FALSE;

    if(!bDetermined)
    {
      fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
          GetModuleHandle(TEXT("kernel32")),"IsWow64Process");
    
      if (NULL != fnIsWow64Process)
      {
          if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
          {
              // handle error
          }
      }
      bDetermined = TRUE;
    }
    return bIsWow64;
}
*/
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
