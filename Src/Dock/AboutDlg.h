
#pragma once

class AboutDlg : private WinAPI::Dialog
{
public:
  AboutDlg() {}

  UINT show(HWND hwndParent);

private:
  virtual bool onDlgMessage(UINT message, WPARAM wParam, LPARAM lParam);
};
