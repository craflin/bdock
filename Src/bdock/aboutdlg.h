
#pragma once

class AboutDlg : public WinAPI::Dialog
{
public:
  AboutDlg() {}

  UINT show(HWND hwndParent);

private:
  virtual bool onDlgMessage(UINT message, WPARAM wParam, LPARAM lParam);
};
