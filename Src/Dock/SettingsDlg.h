
#pragma once

class Storage;

class SettingsDlg : private WinAPI::Dialog
{
public:
  SettingsDlg(Storage& globalStorage) : globalStorage(globalStorage), currentPage(0) {}

  UINT show(HWND hwndParent);

private:
  Storage& globalStorage;

  WinAPI::ImageList pageImageList;
  WinAPI::TreeView pageTreeView;

  WinAPI::Dialog generalPage;
  WinAPI::Dialog* currentPage;

  WinAPI::Button autostartButton;

  void showPage(int dockIndex, int page);

  virtual bool onInitDialog();
  virtual bool onDlgMessage(UINT message, WPARAM wParam, LPARAM lParam);
  virtual bool onCommand(UINT command, HWND source);
};
