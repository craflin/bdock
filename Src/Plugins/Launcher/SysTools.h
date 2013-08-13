
#pragma once

HMENU CopyMenu(HMENU hmenu);
HBITMAP CreateBitmapFromIcon(UINT destWidth, UINT destHeight, HICON icon);

BOOL GetCommandLine(DWORD pid, LPWSTR commandLine, UINT maxLen);
