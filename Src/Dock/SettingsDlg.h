
#pragma once

class Storage;

class SettingsDlg : private WinAPI::Dialog
{
public:
  SettingsDlg(Storage& globalStorage) : globalStorage(globalStorage) {}

  UINT show(HWND hwndParent);

private:
  Storage& globalStorage;

  WinAPI::ImageList pageImageList;
  WinAPI::TreeView pageTreeView;

  WinAPI::Dialog generalPage;

  WinAPI::Button autostartButton;

  virtual bool onInitDialog();
  virtual bool onCommand(UINT command, HWND source);
};
