
#include "stdafx.h"


UINT AboutDlg::show(HWND hwndParent)
{
  return WinAPI::Dialog::show(IDD_ABOUTBOX, hwndParent);
}

bool AboutDlg::onDlgMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message)
  {
  case WM_NOTIFY:
    switch (((LPNMHDR)lParam)->code)
    {
      case NM_CLICK:
      case NM_RETURN:
        {
          PNMLINK pNMLink = (PNMLINK)lParam;
          LITEM   item    = pNMLink->item;
          WinAPI::Shell::execute(NULL, _T("open"), item.szUrl, NULL, NULL, SW_SHOW);
          return true;
        }
    }

  }
  return WinAPI::Dialog::onDlgMessage(message, wParam, lParam);
}
