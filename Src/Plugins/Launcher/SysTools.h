
#pragma once

HMENU CopyMenu(HMENU hmenu);
HBITMAP createBitmapFromIcon(HICON icon, SIZE* size);

BOOL GetCommandLine(DWORD pid, LPWSTR commandLine, UINT maxLen);
