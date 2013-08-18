
#include "stdafx.h"

SettingsDlg::SettingsDlg(Storage& globalStorage) : globalStorage(globalStorage), currentPage(0)
{
  dockManager.addListener(*this);
}

SettingsDlg::~SettingsDlg()
{
  for(auto i = pages.begin(), end = pages.end(); i != end; ++i)
    delete *i;
}

UINT SettingsDlg::show(HWND hwndParent)
{
  return WinAPI::Dialog::showBox(IDD_SETTINGS, hwndParent);
}

void SettingsPage::enableApplyButton()
{
  WinAPI::Button applyButton;
  VERIFY(applyButton.attach(getParent(), IDAPPLY));
  applyButton.enable(true);
  //settingsChanged = true;
}

bool SettingsDlg::onInitDialog()
{
  VERIFY(pageImageList.create(16, 16, ILC_COLOR32, 3, 3));
  VERIFY(pageImageList.add(WinAPI::Icon(IDI_WRENCH_ORANGE)) == 0);
  VERIFY(pageImageList.add(WinAPI::Icon(IDI_TAB)) == 1);
  VERIFY(pageImageList.add(WinAPI::Icon(IDI_TABS)) == 2);
  VERIFY(pageImageList.add(WinAPI::Icon(IDI_PLUGIN)) == 3);
  VERIFY(pageImageList.add(WinAPI::Icon(IDI_ARROW_OUT)) == 4);
  VERIFY(pageImageList.add(WinAPI::Icon(IDI_TAB_ADD)) == 5);
  VERIFY(pageImageList.add(WinAPI::Icon(IDI_CROSS)) == 6);

  VERIFY(applyButton.attach(*this, IDAPPLY));
  
  VERIFY(pageTreeView.attach(*this, IDC_PAGE_TREE));
  VERIFY(pageTreeView.setTheme(_T("Explorer"), NULL));
  VERIFY(pageTreeView.setImageList(pageImageList, TVSIL_NORMAL));
  //pageTreeView.setStyle(pageTreeView.getStyle() | TVS_FULLROWSELECT | TVS_TRACKSELECT);

  //VERIFY(toolbar.create(*this, WS_CHILD | WS_VISIBLE | TBSTYLE_WRAPABLE | TBSTYLE_LIST /*| WS_TABSTOP*/ | CCS_NOPARENTALIGN | CCS_NORESIZE));
  //toolbar.move(0, 0, 100, 100, true);
  //VERIFY(toolbar.setImageList(0, pageImageList));

  // add toolbar buttons
  /*
  WinAPI::String addDock(IDS_SETTINGS_ADD_DOCK);
  WinAPI::String removeDock(IDS_SETTINGS_REMOVE_DOCK);
   TBBUTTON tbButtons[2] = 
    {
        { MAKELONG(4, 0), IDS_SETTINGS_ADD_DOCK, TBSTATE_ENABLED, BTNS_AUTOSIZE, {0}, 0, (INT_PTR) (LPCTSTR) addDock },
        { MAKELONG(5, 0), IDS_SETTINGS_REMOVE_DOCK, 0, BTNS_AUTOSIZE, {0}, 0, (INT_PTR) (LPCTSTR) removeDock },
    };
  TBBUTTON& but = tbButtons[0];
  toolbar.addButtons(tbButtons, 2);
  */
  //toolbar.autoSize();
  //toolbar.show(SW_SHOW);

  // fill page tree view
  WinAPI::String general(IDS_SETTINGS_PAGE_GENERAL);
  WinAPI::String docks(IDS_SETTINGS_PAGE_DOCKS);
  WinAPI::String newDockWin(IDS_SETTINGS_PAGE_NEW_DOCK);
  String newDock;
  newDock.attach(newDockWin, newDockWin.length());

  TVINSERTSTRUCT item;
  item.hParent = TVI_ROOT;
  item.hInsertAfter = TVI_LAST;
  item.itemex.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
  item.itemex.pszText = (LPTSTR) (LPCTSTR) general;
  item.itemex.iImage = 0;
  item.itemex.iSelectedImage = 0;
  item.itemex.lParam = IDD_SETTINGS_GENERAL;
  HTREEITEM hGeneralItem = pageTreeView.insertItem(item);

  item.itemex.pszText = (LPTSTR) (LPCTSTR) docks;
  item.itemex.iImage = 2;
  item.itemex.iSelectedImage = 2;
  item.itemex.mask |= TVIF_STATE;
  item.itemex.state = TVIS_EXPANDED;
  item.itemex.stateMask = TVIS_EXPANDED;
  item.itemex.lParam = IDD_SETTINGS_DOCKS;
  hDocksItem = pageTreeView.insertItem(item);

  // load docks
  for(int_t i = 0, count = globalStorage.getNumSectionCount(); i < count; ++i)
  {
    globalStorage.enterNumSection(i);
    String name = globalStorage.getStr(_T("name"), newDock);
    uint_t id = dockManager.addDock(name);
    (*dockInfoMap.find(id)).storageDockIndex = i;
    globalStorage.leave();
  }

  // pre-select general page
  pageTreeView.selectItem(hGeneralItem);

  return WinAPI::Dialog::onInitDialog();
}

void SettingsDlg::showPage(uint_t dockId, uint_t dialogId)
{
  RECT pageRect;
  GetClientRect(hwnd, &pageRect);
  pageRect.left += 122;
  pageRect.bottom -= 46;

  SettingsPage* newPage = 0;
  auto it = pages.find(dockId << 16 | dialogId);
  if(it != pages.end())
  {
    newPage = *it;
    if(newPage == currentPage)
      return;
  }

  if(newPage == 0)
  {
    switch(dialogId)
    {
    case IDD_SETTINGS_GENERAL:
      newPage = new SettingsPageGeneral;
      newPage->storage = &globalStorage;
      newPage->create(IDD_SETTINGS_GENERAL, *this);
      break;
    case IDD_SETTINGS_DOCKS:
      newPage = new SettingsPageDocks(dockManager);
      newPage->create(IDD_SETTINGS_DOCKS, *this);
      break;
    case IDD_SETTINGS_DOCK:
      {
        newPage = new SettingsPageDock;
        DockInfo& dockInfo = *dockInfoMap.find(dockId);
        if(dockInfo.storageDockIndex >= 0)
          newPage->storage = globalStorage.getNumSection(dockInfo.storageDockIndex);
        newPage->create(IDD_SETTINGS_DOCK, *this);
        dockInfo.pages.append(newPage);
      }
      break;
    case IDD_SETTINGS_PLUGINS:
      {
        newPage = new SettingsPagePlugins;
        DockInfo& dockInfo = *dockInfoMap.find(dockId);
        if(dockInfo.storageDockIndex >= 0)
          newPage->storage = globalStorage.getNumSection(dockInfo.storageDockIndex);
        newPage->create(IDD_SETTINGS_PLUGINS, *this);
        dockInfo.pages.append(newPage);
      }
      break;
    }
    if(newPage)
    {
      newPage->setStyle(newPage->getStyle() | WS_TABSTOP);
      newPage->move(pageRect.left, pageRect.top, pageRect.right - pageRect.left, pageRect.bottom - pageRect.top, false);
      pages.append((dockId << 16) | dialogId, newPage);
    }
  }

  if(currentPage)
      currentPage->show(SW_HIDE);

  currentPage = newPage;
  if(newPage)
    newPage->show(SW_SHOW);
}

bool SettingsDlg::onCommand(UINT command, UINT notificationCode, HWND source)
{
  switch(command)
  {
  case IDAPPLY:
  case IDOK:
    {
      // update dock order in storage, generate storage section for new docks, pass storage to settings pages of newly added docks
      List<DockManager::Dock> docks;
      dockManager.getDocks(docks);
      int_t index = 0;
      for(auto i = docks.begin(), end = docks.end(); i != end; ++i, ++index)
      {
        DockManager::Dock& dock = *i;
        DockInfo& dockInfo = *dockInfoMap.find(dock.id);
        Storage* storage;
        if(dockInfo.storageDockIndex >= 0)
          storage = globalStorage.getNumSection(dockInfo.storageDockIndex);
        else
        {
          dockInfo.storageDockIndex = globalStorage.getNumSectionCount();
          globalStorage.setNumSectionCount(dockInfo.storageDockIndex + 1);
         storage = globalStorage.getNumSection(dockInfo.storageDockIndex);
          for(auto i = dockInfo.pages.begin(), end = dockInfo.pages.end(); i != end; ++i)
            (*i)->storage = storage;
        }
        if(dockInfo.storageDockIndex != index)
        {
          int_t oldIndex = dockInfo.storageDockIndex;
          globalStorage.swapNumSections(oldIndex, index);
          for(auto i = dockInfoMap.begin(), end = dockInfoMap.end(); i != end; ++i)
          {
            DockInfo& dockInfo = *i;
            if(dockInfo.storageDockIndex == index)
              dockInfo.storageDockIndex = oldIndex;
          }
          dockInfo.storageDockIndex = index;
        }
        storage->setStr(_T("name"), dock.name);
      }
      globalStorage.setNumSectionCount(index);

      // let each page write its settings into its storage
      for(auto i = pages.begin(), end = pages.end(); i != end; ++i)
      {
        SettingsPage* page = *i;
        page->sendMessage(WM_COMMAND, IDAPPLY, (LPARAM) (HWND) *this);
      }

      // disable apply button
      if(command == IDAPPLY)
        applyButton.enable(false);
    }
    break;
  }
  return WinAPI::Dialog::onCommand(command, notificationCode, source);
}

bool SettingsDlg::onDlgMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message)
  {
  case WM_NOTIFY:
    switch (((LPNMHDR)lParam)->code)
    {
      /*
    case TVN_BEGINLABELEDIT:
      {
        LPNMTVDISPINFO nmtvdi = (LPNMTVDISPINFO) lParam;
        HTREEITEM hparent = pageTreeView.getItemParent(nmtvdi->item.hItem);
        if((hparent != NULL && hparent != TVI_ROOT) || (int) nmtvdi->item.lParam < 0)
        {
          SetWindowLongPtr(hwnd, DWLP_MSGRESULT, TRUE);
          return TRUE;
        }

        break;
      }
    case TVN_ENDLABELEDIT:
      {
        LPNMTVDISPINFO ptvdi = (LPNMTVDISPINFO) lParam;
        if(ptvdi->item.pszText)
        {
          pageTreeView.setItemText(ptvdi->item.hItem, ptvdi->item.pszText);
        }
      }
      break;
      */
    case TVN_SELCHANGED:
      {
        LPNMTREEVIEW nmtv = (LPNMTREEVIEW) lParam;
        {
          // create title from selected tree item
          String path, itemText;
          WinAPI::String itemTextWin;
          HTREEITEM hItem = nmtv->itemNew.hItem;
          do
          {
            pageTreeView.getItemText(hItem, itemTextWin);
            itemText.attach(itemTextWin, itemTextWin.length());
            if(!path.isEmpty())
              path.prepend(_T(" / "));
            path.prepend(itemText);
            hItem = pageTreeView.getParent(hItem);
          } while(hItem);

          WinAPI::String captionWin;
          getText(captionWin);
          String title(captionWin, captionWin.length());
          const tchar_t* separator = title.find(_T(" - "));
          if(separator)
            title.resize(separator - (const tchar_t*)title);
          title += _T(" - ");
          title += path;
          setText(title);
        }
        uint_t data = (uint_t) nmtv->itemNew.lParam;
        uint_t dockId = data >> 16;
        uint_t dialogId = data & 0xffff;
        showPage(dockId, dialogId);
        break;
      }
    }
    break;
  }
  return WinAPI::Dialog::onDlgMessage(message, wParam, lParam);
}

void_t SettingsDlg::addedDock(uint_t id, const String& name)
{
  DockInfo& dockInfo = dockInfoMap.append(id, DockInfo());
  dockInfo.storageDockIndex = -1;

  //WinAPI::String appearance(IDS_SETTINGS_PAGE_APPEARANCE);
  WinAPI::String plugins(IDS_SETTINGS_PAGE_PLUGINS);

  TVINSERTSTRUCT item;
  item.hParent = hDocksItem;
  item.hInsertAfter = TVI_LAST;

  item.itemex.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE | TVIF_PARAM;
  item.itemex.pszText = (LPTSTR)(const tchar_t*) name;
  item.itemex.iImage = 1;
  item.itemex.iSelectedImage = 1;
  item.itemex.state = TVIS_EXPANDED;
  item.itemex.stateMask = TVIS_EXPANDED;
  item.itemex.lParam = IDD_SETTINGS_DOCK | (id << 16);
  dockInfo.hItem = pageTreeView.insertItem(item);

  item.hParent = dockInfo.hItem;
  item.itemex.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
  /*
  item.itemex.pszText = (LPTSTR) (LPCTSTR) appearance;
  item.itemex.iImage = 3;
  item.itemex.iSelectedImage = 3;
  item.itemex.lParam = 1; // apperance page
  pageTreeView.insertItem(item);
  */
  item.hParent = dockInfo.hItem;
  item.itemex.pszText = (LPTSTR) (LPCTSTR) plugins;
  item.itemex.iImage = 3;
  item.itemex.iSelectedImage = 3;
  item.itemex.lParam = IDD_SETTINGS_PLUGINS | (id << 16); // plugins page
  pageTreeView.insertItem(item);
}

void_t SettingsDlg::renamedDock(uint_t id, const String& name)
{
  DockInfo& dockInfo = *dockInfoMap.find(id);
  pageTreeView.setItemText(dockInfo.hItem, name);
}

void_t SettingsDlg::removedDock(uint_t id)
{
  DockInfo& dockInfo = *dockInfoMap.find(id);
  pageTreeView.deleteItem(dockInfo.hItem);
  dockInfoMap.remove(id);
}

bool SettingsPageGeneral::onInitDialog()
{
  VERIFY(autostartButton.attach(*this, IDC_AUTOSTART));
  VERIFY(autostartButton.setCheck(storage->getUInt(_T("autostart"), 0) ? BST_CHECKED : BST_UNCHECKED));

  return SettingsPage::onInitDialog();
}

bool SettingsPageGeneral::onCommand(UINT command, UINT notificationCode, HWND source)
{
  if(notificationCode == BN_CLICKED)
    enableApplyButton();

  switch(command)
  {
  case IDOK:
  case IDAPPLY:
    {
      bool autostart = autostartButton.getCheck() == BST_CHECKED;
      storage->setUInt(_T("autostart"), autostart);
      break;
    }
  }

  return SettingsPage::onCommand(command, notificationCode, source);
}

bool SettingsPageGeneral::onDlgMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message)
  {
  case WM_NOTIFY:
    break;
  }
  return SettingsPage::onDlgMessage(message, wParam, lParam);
}

bool SettingsPageDocks::onInitDialog()
{
  // create image list
  VERIFY(imageList.create(16, 16, ILC_COLOR32, 3, 3));
  VERIFY(imageList.add(WinAPI::Icon(IDI_TAB_ADD)) == 0);
  VERIFY(imageList.add(WinAPI::Icon(IDI_CROSS)) == 1);
  VERIFY(imageList.add(WinAPI::Icon(IDI_TAB)) == 2);

  // create toolbar
  VERIFY(toolbar.create(*this, WS_CHILD | WS_VISIBLE | TBSTYLE_WRAPABLE | TBSTYLE_LIST /*| WS_TABSTOP*/ | CCS_NOPARENTALIGN | CCS_NORESIZE | CCS_NODIVIDER));
  VERIFY(toolbar.setImageList(0, imageList));

  // add toolbar buttons
  WinAPI::String addDock(IDS_SETTINGS_ADD_DOCK);
  WinAPI::String removeDock(IDS_SETTINGS_REMOVE_DOCK);
   TBBUTTON tbButtons[2] = 
    {
        { MAKELONG(0, 0), IDS_SETTINGS_ADD_DOCK, TBSTATE_ENABLED, BTNS_AUTOSIZE, {0}, 0, (INT_PTR) (LPCTSTR) addDock },
        { MAKELONG(1, 0), IDS_SETTINGS_REMOVE_DOCK, 0, BTNS_AUTOSIZE, {0}, 0, (INT_PTR) (LPCTSTR) removeDock },
    };
  TBBUTTON& but = tbButtons[0];
  toolbar.addButtons(tbButtons, 2);

  RECT toolbarRect;
  GetClientRect(hwnd, &toolbarRect);
  SIZE idealSize;
  toolbar.sendMessage(TB_GETIDEALSIZE, FALSE, (LPARAM)&idealSize);
  LRESULT buttonSize = toolbar.sendMessage(TB_GETBUTTONSIZE, NULL, NULL);
  toolbarRect.top = 9;
  toolbarRect.bottom = toolbarRect.top + HIWORD(buttonSize);
  toolbarRect.right -= 11;
  toolbarRect.left = toolbarRect.right - idealSize.cx;
  toolbar.move(toolbarRect, true);

  // initialize list view
  VERIFY(listView.attach(*this, IDC_DOCK_LIST));
  //VERIFY(listView.setView(LV_VIEW_DETAILS));
  VERIFY(listView.setTheme(_T("Explorer"), NULL));
  VERIFY(listView.setExtendedStyle(LVS_EX_FULLROWSELECT));
  VERIFY(listView.setImageList(imageList, LVSIL_NORMAL));
  
  // enable tile mode
  VERIFY(listView.setView(LV_VIEW_TILE));
  RECT rcClient;
  VERIFY(listView.getClientRect(rcClient));

  SIZE size = { rcClient.right - rcClient.left - GetSystemMetrics(SM_CXVSCROLL), 30 };
  LVTILEVIEWINFO tileViewInfo = {0};

  tileViewInfo.cbSize   = sizeof(tileViewInfo);
  tileViewInfo.dwFlags  = LVTVIF_FIXEDSIZE;
  tileViewInfo.dwMask   = LVTVIM_COLUMNS | LVTVIM_TILESIZE;
  tileViewInfo.cLines   = 0;
  tileViewInfo.sizeTile = size;

  ListView_SetTileViewInfo(listView, &tileViewInfo);

  // add columns
  {
    //RECT rcClient;
    //VERIFY(listView.getClientRect(rcClient));

    LVCOLUMN lvc;
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvc.fmt = LVCFMT_LEFT;
    lvc.iSubItem = 0;
    lvc.pszText = _T("Name");
    lvc.cx = 100;//rcClient.right - rcClient.left - GetSystemMetrics(SM_CXVSCROLL);
    ListView_InsertColumn(listView, lvc.iSubItem, &lvc);
  }

  // add dock list
  //WinAPI::String newDockWin(IDS_SETTINGS_PAGE_NEW_DOCK);
  //String newDock;
  //newDock.attach(newDockWin, newDockWin.length());
  List<DockManager::Dock> docks;
  dockManager.getDocks(docks);
  for(auto i = docks.begin(), end = docks.end(); i != end; ++i)
  {
    DockManager::Dock& dock = *i;
    LVITEM lvi;
    lvi.mask = LVIF_TEXT | LVIF_STATE | LVIF_IMAGE | LVIF_PARAM;
    lvi.pszText = (LPTSTR)(const tchar_t*)dock.name;
    lvi.state = 0;
    lvi.stateMask = 0;
    lvi.iSubItem = 0;
    lvi.iImage = 2;
    lvi.lParam = dock.id;
    INT item = listView.addItem(lvi);
  }

  return SettingsPage::onInitDialog();
}

bool SettingsPageDocks::onCommand(UINT command, UINT notificationCode, HWND source)
{
  switch(command)
  {
  case IDS_SETTINGS_ADD_DOCK:
    {
      WinAPI::String newDockWin(IDS_SETTINGS_PAGE_NEW_DOCK);
      String newDock;
      newDock.attach(newDockWin, newDockWin.length());
      
      LVITEM lvi;
      lvi.mask = LVIF_TEXT | LVIF_STATE | LVIF_IMAGE | LVIF_PARAM;
      lvi.pszText = (LPTSTR)newDockWin;
      lvi.state = 0;
      lvi.stateMask = 0;
      lvi.iSubItem = 0;
      lvi.iImage = 2;
      lvi.lParam  = dockManager.addDock(newDock);
      INT item = listView.addItem(lvi);

      listView.setFocus();
      listView.editLabel(item);
      enableApplyButton();
      break;
    }
  case IDS_SETTINGS_REMOVE_DOCK:
    {
      for(;;)
      {
        INT iSel = listView.getFirstItem(LVNI_SELECTED);
        if(iSel == -1)
          break;

        void_t* itemData;
        VERIFY(listView.getItemData(iSel, itemData));
        dockManager.removeDock((uint_t)itemData);
        listView.deleteItem(iSel);

        enableApplyButton();
      }
      break;
    }
  case IDAPPLY:
    {
      //MessageBox(0, NULL, NULL, 0);
      break;
    }
  case IDOK:
    {
      WinAPI::Edit edit;
      if(listView.getEditControl(edit))
      {
        if(source == edit)
          return false;
        
        ListView_CancelEditLabel(listView);
        return false;
      }
      break;
    }
  }
  return SettingsPage::onCommand(command, notificationCode, source);
}

bool SettingsPageDocks::onDlgMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message)
  {
  case WM_NOTIFY:
    switch (((LPNMHDR)lParam)->code)
    {
    case LVN_ENDLABELEDIT:
      {
        NMLVDISPINFO* pdi = (NMLVDISPINFO*)lParam; 
        if(pdi->item.pszText)
        {
          size_t textLen = _tcslen(pdi->item.pszText);
          if(textLen == 0)
            return false;
          listView.setItemText(pdi->item.iItem, 0, pdi->item.pszText);

          void* itemData;
          VERIFY(listView.getItemData(pdi->item.iItem, itemData));
          uint_t dockId = (uint_t)itemData;
          String name;
          name.attach(pdi->item.pszText, textLen);
          dockManager.setDockName(dockId, name);
          enableApplyButton();
          return true;
        }
        break;
      }
    case LVN_BEGINDRAG:
      {
        // TODO: allow dock list to be sorted via drag'n'drop
        break;
      }
    case LVN_ITEMCHANGED:
      {
        bool enableRemoveButton = ListView_GetSelectedCount(listView) > 0 && ListView_GetItemCount(listView) > 1;
        toolbar.setButtonState(IDS_SETTINGS_REMOVE_DOCK, enableRemoveButton ? TBSTATE_ENABLED : 0);
        break;
      }
    }
    break;
  }
  return WinAPI::Dialog::onDlgMessage(message, wParam, lParam);
}


bool SettingsPagePlugins::onInitDialog()
{
  VERIFY(listView.attach(*this, IDC_PLUGIN_LIST));
  //VERIFY(listView.setView(LV_VIEW_DETAILS));
  VERIFY(listView.setTheme(_T("Explorer"), NULL));
  VERIFY(listView.setExtendedStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT));

  // enable tile mode
  VERIFY(listView.setView(LV_VIEW_TILE));
  RECT rcClient;
  VERIFY(listView.getClientRect(rcClient));

  SIZE size = { rcClient.right - rcClient.left - GetSystemMetrics(SM_CXVSCROLL), 50 };
  LVTILEVIEWINFO tileViewInfo = {0};

  tileViewInfo.cbSize   = sizeof(tileViewInfo);
  tileViewInfo.dwFlags  = LVTVIF_FIXEDSIZE;
  tileViewInfo.dwMask   = LVTVIM_COLUMNS | LVTVIM_TILESIZE;
  tileViewInfo.cLines   = 2;
  tileViewInfo.sizeTile = size;

  ListView_SetTileViewInfo(listView, &tileViewInfo);

  // load enabled plugins set
  HashSet<String> enabledPlugins;
  if(storage)
    for(int i = 0, count = storage->getNumSectionCount(); i < count; ++i)
    {
      storage->enterNumSection(i);
      String name = storage->getStr(_T("name"));
      if(!name.isEmpty())
      {
        name.toLowerCase();
        enabledPlugins.append(name);
      }
      storage->leave();
    }

  // add columns
  {
    LVCOLUMN lvc;
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvc.fmt = LVCFMT_LEFT;
    lvc.iSubItem = 0;
    lvc.pszText = _T("Name");
    lvc.cx = 100;
    ListView_InsertColumn(listView, lvc.iSubItem, &lvc);
    lvc.pszText = _T("Description");
    lvc.cx = 100;
    lvc.iSubItem = 1;
    ListView_InsertColumn(listView, lvc.iSubItem, &lvc);
    lvc.pszText = _T("Author");
    lvc.cx = 100;
    lvc.iSubItem = 2;
    ListView_InsertColumn(listView, lvc.iSubItem, &lvc);
  }

  // load avaible plugins
  WIN32_FIND_DATA ffd;
  HANDLE hFind = FindFirstFile(_T("Plugins/*.dll"), &ffd);
  if(hFind == INVALID_HANDLE_VALUE)
  {
    TCHAR searchStr[MAX_PATH];
    GetModuleFileName(WinAPI::Application::getInstance(), searchStr, MAX_PATH);
    TCHAR* lastSlash = searchStr;
    for(;;)
    {
      TCHAR* s = _tcspbrk(lastSlash + 1, _T("\\/"));
      if(s)
        lastSlash = s;
      else
        break;
    }
    if(lastSlash != searchStr)
    {
      _tcscpy_s(lastSlash, sizeof(searchStr) / sizeof(*searchStr) - (lastSlash - searchStr), _T("\\*.dll"));
      hFind = FindFirstFile(searchStr, &ffd);
    }
  }
  if(hFind != INVALID_HANDLE_VALUE)
  {
    String fileName;
    String displayName, name, author, description;
    WinAPI::String by(IDS_SETTINGS_PLUGIN_BY);
    uint32_t version;
    do
    {
      fileName.attach(ffd.cFileName, String::length(ffd.cFileName));
      if(fileName.endsWith(_T("-copy")))
        continue;
      if(getPluginInfo(fileName, name, displayName, version, author, description))
      {
        TCHAR nameVersion[50];
        _stprintf_s(nameVersion, _T("%s (%u.%u.%u)"), (LPCTSTR)displayName, (UINT)(version >> 24), (UINT)((version >> 16) & 0xff), (UINT)((version >> 8) & 0xff));

        LVITEM lvi;
        lvi.mask = LVIF_TEXT | LVIF_STATE | LVIF_COLUMNS;
        lvi.pszText = (LPTSTR)(LPCTSTR) nameVersion;
        lvi.state = 0;
        lvi.stateMask = 0;
        lvi.iSubItem = 0;
        UINT colums[] = { 1, 2 };
        lvi.cColumns = 2;
        lvi.puColumns = colums;
        lvi.piColFmt = 0;
        INT item = listView.addItem(lvi);
        listView.setItemText(item, 1, (LPCTSTR)description);
        TCHAR authorInfo[256];
        _stprintf_s(authorInfo, _T("%s %s"), (LPCTSTR)by, (LPCTSTR)author);
        listView.setItemText(item, 2, authorInfo);
        name.toLowerCase();
        if(enabledPlugins.find(name) != enabledPlugins.end())
          listView.setCheckState(item, TRUE);
      }
    } while(FindNextFile(hFind, &ffd));
    FindClose(hFind);
  }

  loaded = true;
  return SettingsPage::onInitDialog();
}

bool_t SettingsPagePlugins::getPluginInfo(const String& fileName, String& name, String& displayName, uint32_t& version, String& author, String& description)
{
  DWORD handle = 0;
  DWORD size = GetFileVersionInfoSize(fileName, &handle);
  ASSERT(handle == 0);
  BYTE* versionInfo = (BYTE*)Memory::alloc(size);
  if (!GetFileVersionInfo(fileName, handle, size, versionInfo))
  {
    Memory::free(versionInfo);
    return false;
  }

  UINT len = 0;
  VS_FIXEDFILEINFO* vsfi = 0;
  if(VerQueryValue(versionInfo, _T("\\"), (LPVOID*)&vsfi, &len))
  {
    version = HIWORD(vsfi->dwFileVersionMS) << 24;
    version |= LOWORD(vsfi->dwFileVersionMS) << 16;
    version |= HIWORD(vsfi->dwFileVersionLS) << 8;
    version |= LOWORD(vsfi->dwFileVersionLS);
  }
  else
    version = 0;

  WORD* translationTable;
  if(VerQueryValue(versionInfo,  _T("\\VarFileInfo\\Translation"), (LPVOID*)&translationTable, &len) && len >= sizeof(WORD) * 2)
  {
    TCHAR* value;
    TCHAR subBlock[50];

    _stprintf_s(subBlock, _T("\\StringFileInfo\\%04x%04x\\FileDescription"), translationTable[0], translationTable[1]);
    if(VerQueryValue(versionInfo, subBlock, (LPVOID*) &value, &len))
      description = String(value, len - 1);
    _stprintf_s(subBlock, _T("\\StringFileInfo\\%04x%04x\\CompanyName"), translationTable[0], translationTable[1]);
    if(VerQueryValue(versionInfo, subBlock, (LPVOID*) &value, &len))
      author = String(value, len - 1);
    _stprintf_s(subBlock, _T("\\StringFileInfo\\%04x%04x\\ProductName"), translationTable[0], translationTable[1]);
    if(VerQueryValue(versionInfo, subBlock, (LPVOID*) &value, &len))
      displayName = String(value, len - 1);
    else
      displayName = fileName;
  }
  else
    displayName = fileName;

  name = File::basename(fileName, File::extension(fileName));

  Memory::free(versionInfo);
  return true;
}

bool SettingsPagePlugins::onCommand(UINT command, UINT notificationCode, HWND source)
{
  switch(command)
  {
  case IDOK:
  case IDAPPLY:
    {
      // ...
      break;
    }
  }

  return SettingsPage::onCommand(command, notificationCode, source);
}

bool SettingsPagePlugins::onDlgMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message)
  {
  case WM_NOTIFY:
    switch (((LPNMHDR)lParam)->code)
    {
    case LVN_ITEMCHANGED:
      {
        LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam; 
        if(loaded && pnmv->uChanged & LVIF_STATE && (pnmv->uNewState ^ pnmv->uOldState) & (INDEXTOSTATEIMAGEMASK(2) | INDEXTOSTATEIMAGEMASK(1)))
          enableApplyButton();
      }
      break;
    }
    break;
  }
  return SettingsPage::onDlgMessage(message, wParam, lParam);
}
