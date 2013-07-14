
#ifndef AboutDlg_H
#define AboutDlg_H

class AboutDlg : public WinAPI::Dialog
{
public:
  AboutDlg() {}

  UINT show(HWND hwndParent);

private:
  virtual bool onDlgMessage(UINT message, WPARAM wParam, LPARAM lParam);
};


#endif
