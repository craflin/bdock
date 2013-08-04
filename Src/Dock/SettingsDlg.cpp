
#include "stdafx.h"

UINT SettingsDlg::show(HWND hwndParent)
{
  return WinAPI::Dialog::show(IDD_SETTINGS, hwndParent);
}

bool SettingsDlg::onInitDialog()
{
  /*
  VERIFY(pageImageList.create(16, 16, ILC_COLOR32, 3, 3));
  VERIFY(pageImageList.add(WinAPI::Icon(IDI_BDOCK)) == 0);

  VERIFY(pageTreeView.initialize(*this, IDC_PAGE_TREE));
  VERIFY(pageTreeView.setTheme(_T("Explorer"), NULL));
  VERIFY(pageTreeView.setImageList(pageImageList, TVSIL_NORMAL));
  */
  VERIFY(autostartButton.initialize(*this, IDC_AUTOSTART));
  
  VERIFY(autostartButton.setCheck(globalStorage.getUInt("autostart", 0) ? BST_CHECKED : BST_UNCHECKED));
  /*
  WinAPI::String text(IDS_SETTINGS_PAGE_GENERAL);

  TVINSERTSTRUCT item;
  item.hParent = TVI_ROOT;
  item.hInsertAfter = TVI_LAST;
  item.itemex.mask = TVIF_TEXT | TVIF_IMAGE;
  item.itemex.pszText = (LPTSTR) (LPCTSTR) text;
  item.itemex.iImage = 0;
  pageTreeView.insertItem(item);
  */
  return WinAPI::Dialog::onInitDialog();
}

bool SettingsDlg::onCommand(UINT command, HWND source)
{
  if(command == IDOK)
  {
    bool autostart = autostartButton.getCheck() == BST_CHECKED;
    globalStorage.setUInt("autostart", autostart);
  }
  return WinAPI::Dialog::onCommand(command, source);
}

