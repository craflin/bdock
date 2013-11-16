
#include "stdafx.h"

HMENU CopyMenu(HMENU hmenu)
{
  MENUITEMINFO mii;
  mii.cbSize = sizeof(mii);
  mii.fMask = MIIM_STATE | MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_SUBMENU | MIIM_CHECKMARKS | MIIM_BITMAP;
  WCHAR* buf = 0;
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
      buf = (WCHAR*)malloc((len + 1) * sizeof(WCHAR));
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
HBITMAP CreateBitmapFromIcon(HICON icon, SIZE* size)
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

HBITMAP CreateResizedBitmap(UINT destWidth, UINT destHeight, HBITMAP hsrc, UINT srcX, UINT srcY, UINT srcWidth, UINT srcHeight)
{
  // create source dc
  HDC src = CreateCompatibleDC(NULL);
  HBITMAP oldSrcBmp = (HBITMAP)SelectObject(src, hsrc);

  // create dest dc
  HBITMAP bmp = CreateBitmap(destWidth, destHeight, 1, 32, NULL);
  HDC dest = CreateCompatibleDC(NULL);
  HBITMAP oldDestBmp = (HBITMAP)SelectObject(dest, bmp);
  

  SetStretchBltMode(dest, COLORONCOLOR);
  StretchBlt(dest, 0, 0, destWidth, destHeight, src, srcX, srcY, srcWidth, srcHeight, SRCCOPY);

  // cleanup
  SelectObject(dest, oldDestBmp);
  SelectObject(src, oldSrcBmp);
  DeleteDC(dest);
  DeleteDC(src);

  return bmp;
}

HBITMAP CreateBitmapFromIcon(UINT destWidth, UINT destHeight, HICON icon)
{
  SIZE size;
  HBITMAP hbitmap = CreateBitmapFromIcon(icon, &size);
  if(size.cy == destWidth && size.cx == destHeight)
    return hbitmap;
  HBITMAP hresized = CreateResizedBitmap(destWidth, destHeight, hbitmap, 0, 0, size.cy, size.cx);
  DeleteObject(hbitmap);
  return hresized;
}


#include <Winternl.h>

struct NtDll
{
  typedef NTSTATUS (NTAPI *pfnNtQueryInformationProcess)(
      IN  HANDLE ProcessHandle,
      IN  PROCESSINFOCLASS ProcessInformationClass,
      OUT PVOID ProcessInformation,
      IN  ULONG ProcessInformationLength,
      OUT PULONG ReturnLength    OPTIONAL
      );

  pfnNtQueryInformationProcess NtQueryInformationProcess;

  NtDll() : NtQueryInformationProcess(NULL), hNtDll(NULL) {}

  ~NtDll()
  {
    if(hNtDll)
      FreeLibrary(hNtDll);
  }

  bool load()
  {
    if(!hNtDll)
    {
      hNtDll = LoadLibrary(_T("ntdll.dll"));
      if(hNtDll == NULL)
        return false;
    }

    if(!NtQueryInformationProcess)
    {
      NtQueryInformationProcess = (pfnNtQueryInformationProcess)GetProcAddress(hNtDll, "NtQueryInformationProcess");
      if(NtQueryInformationProcess == NULL)
        return false; 
    }

    return true;
  }

private:
  HMODULE hNtDll;
} ntDll;


BOOL GetCommandLine(DWORD pid, LPWSTR commandLine, UINT maxLen)
{
  // Earlier versions of BDock used a the Windows Management Instrumentation interface (WMI) to determine the 
  // command line that was used to launch a process. While this worked most of the time, it was a slow 
  // and bloated approach.

  // I came across this PEB-based technique after reading an article on getting process info with
  // NtQueryInformationProcess by Steven A. Moore
  // http://www.codeproject.com/Articles/19685/Get-Process-Info-with-NtQueryInformationProcess

  if(!ntDll.load())
    return FALSE;

  LPCWSTR result = 0;

  HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
  if(hProcess)
  {
    PROCESS_BASIC_INFORMATION pbi;
    pbi.PebBaseAddress = 0;
    if(NT_SUCCESS(ntDll.NtQueryInformationProcess(hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), NULL)))
      if(pbi.PebBaseAddress)
      {
        PEB peb;
        SIZE_T bytesRead;
        if(ReadProcessMemory(hProcess, pbi.PebBaseAddress, &peb, sizeof(peb), &bytesRead))
          if(bytesRead == sizeof(peb))
          {
            RTL_USER_PROCESS_PARAMETERS upp;
            if(ReadProcessMemory(hProcess, peb.ProcessParameters, &upp, sizeof(upp), &bytesRead))
              if(bytesRead == sizeof(upp))
              {
                UINT readLen = upp.CommandLine.Length / sizeof(WCHAR);
                if(readLen > maxLen - 1)
                  readLen = maxLen -1;
                if(ReadProcessMemory(hProcess, upp.CommandLine.Buffer, commandLine, readLen * sizeof(WCHAR), &bytesRead))
                  if(bytesRead == readLen * sizeof(WCHAR))
                  {
                    commandLine[readLen] = 0;
                    CloseHandle(hProcess);
                    return TRUE;
                  }
              }
          }
      }

    CloseHandle(hProcess);
  }
  return FALSE;
}
