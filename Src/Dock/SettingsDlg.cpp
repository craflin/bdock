
#include "stdafx.h"

UINT SettingsDlg::show(HWND hwndParent)
{
  return WinAPI::Dialog::show(IDD_SETTINGS, hwndParent);
}

bool SettingsDlg::onInitDialog()
{
  VERIFY(pageImageList.create(16, 16, ILC_COLOR32, 3, 3));
  VERIFY(pageImageList.add(WinAPI::Icon(IDI_WRENCH_ORANGE)) == 0);
  VERIFY(pageImageList.add(WinAPI::Icon(IDI_TAB)) == 1);
  VERIFY(pageImageList.add(WinAPI::Icon(IDI_PLUGIN)) == 2);
  VERIFY(pageImageList.add(WinAPI::Icon(IDI_ARROW_OUT)) == 3);

  VERIFY(pageTreeView.initialize(*this, IDC_PAGE_TREE));
  VERIFY(pageTreeView.setTheme(_T("Explorer"), NULL));
  VERIFY(pageTreeView.setImageList(pageImageList, TVSIL_NORMAL));


  generalPage.create(IDD_SETTINGS_GENERAL, *this);
  generalPage.move(122, 0, 334, 240, false);
  //generalPage.show(SW_SHOW);
  //generalPage.setStyle(generalPage.getStyle() | WS_VISIBLE);
  showPage(-1, 0);


  VERIFY(autostartButton.initialize(*this, IDC_AUTOSTART));

  VERIFY(autostartButton.setCheck(globalStorage.getUInt(_T("autostart"), 0) ? BST_CHECKED : BST_UNCHECKED));

  WinAPI::String general(IDS_SETTINGS_PAGE_GENERAL);
  WinAPI::String appearance(IDS_SETTINGS_PAGE_APPEARANCE);
  WinAPI::String plugins(IDS_SETTINGS_PAGE_PLUGINS);

  TVINSERTSTRUCT item;
  item.hParent = TVI_ROOT;
  item.hInsertAfter = TVI_LAST;
  item.itemex.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
  item.itemex.pszText = (LPTSTR) (LPCTSTR) general;
  item.itemex.iImage = 0;
  item.itemex.iSelectedImage = 0;
  pageTreeView.insertItem(item);

  item.itemex.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE | TVIF_PARAM;
  item.itemex.pszText = (LPTSTR) (LPCTSTR) _T("MainDock");
  item.itemex.iImage = 1;
  item.itemex.iSelectedImage = 1;
  item.itemex.state = TVIS_EXPANDED;
  item.itemex.stateMask = TVIS_EXPANDED;
  item.itemex.lParam = 0; // dock index
  HTREEITEM hdockitem = pageTreeView.insertItem(item);

  item.hParent = hdockitem;
  item.itemex.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
  item.itemex.pszText = (LPTSTR) (LPCTSTR) appearance;
  item.itemex.iImage = 3;
  item.itemex.iSelectedImage = 3;
  item.itemex.lParam = 0; // apperance page
  pageTreeView.insertItem(item);

  item.hParent = hdockitem;
  item.itemex.pszText = (LPTSTR) (LPCTSTR) plugins;
  item.itemex.iImage = 2;
  item.itemex.iSelectedImage = 2;
  item.itemex.lParam = 0; // plugins page
  pageTreeView.insertItem(item);


  return WinAPI::Dialog::onInitDialog();
}

void SettingsDlg::showPage(int dockIndex, int page)
{
  WinAPI::Dialog* newPage;
  //doc

  if(currentPage)
      currentPage->setStyle(generalPage.getStyle() & ~WS_VISIBLE);

  //current ?
}

bool SettingsDlg::onCommand(UINT command, HWND source)
{
  if(command == IDOK)
  {
    bool autostart = autostartButton.getCheck() == BST_CHECKED;
    globalStorage.setUInt(_T("autostart"), autostart);
  }
  return WinAPI::Dialog::onCommand(command, source);
}

bool SettingsDlg::onDlgMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message)
  {
  case WM_NOTIFY:
    if(false) // ??
    {
      int dockIndex = -1;
      HTREEITEM hParent; // ??
      if(hParent != TVI_ROOT)
      {
        // get parent
        // get dock index from parent
      }
      int page; // get page from current item
      showPage(dockIndex, page);

    }
    break;
  }
  return WinAPI::Dialog::onDlgMessage(message, wParam, lParam);
}
