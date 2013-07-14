
#ifndef SettingsDlg_H
#define SettingsDlg_H

class Storage;

class SettingsDlg : public WinAPI::Dialog
{
public:
  SettingsDlg(Storage& globalStorage) : globalStorage(globalStorage) {}

  UINT show(HWND hwndParent);

private:
  Storage& globalStorage;

  WinAPI::ImageList pageImageList;
  WinAPI::TreeView pageTreeView;

  WinAPI::Button autostartButton;

  virtual bool onInitDialog();
  virtual bool onCommand(UINT command, HWND source);
};


#endif
