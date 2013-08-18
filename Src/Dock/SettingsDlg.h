
#pragma once

class Storage;

class SettingsPage : public WinAPI::Dialog
{
public:
  Storage* storage;

  SettingsPage() : storage(0) {}

protected:
  void enableApplyButton();
};

class DockManager
{
public:
  class Listener
  {
  public:
    virtual void_t addedDock(uint_t id, const String& name) = 0;
    virtual void_t renamedDock(uint_t id, const String& name) = 0;
    virtual void_t removedDock(uint_t id) = 0;
  };

  class Dock
  {
  public:
    uint_t id;
    String name;

    Dock() : id(0) {}

    Dock(uint_t id, const String& name) : id(id), name(name) {}
  };

  DockManager() : nextId(0) {}

  void_t addListener(Listener& listener) {listeners.append(&listener);}
  void_t removeListener(Listener& listener) {listeners.remove(&listener);}

  uint_t addDock(const String& name)
  {
    uint_t id = nextId++;
    docks.append(id, name);
    for (auto i = listeners.begin(), end = listeners.end(); i != end; ++i)
      (*i)->addedDock(id, name);
    return id;
  }

  void_t setDockName(uint_t id, const String& name)
  {
    docks.append(id, name);
    for (auto i = listeners.begin(), end = listeners.end(); i != end; ++i)
      (*i)->renamedDock(id, name);
  }

  void_t removeDock(uint_t id)
  {
    docks.remove(id);
    for (auto i = listeners.begin(), end = listeners.end(); i != end; ++i)
      (*i)->removedDock(id);
  }
  void_t getDocks(List<Dock>& docks)
  {
    for(auto i = this->docks.begin(), end = this->docks.end(); i != end; ++i)
      Dock& dock = docks.append(Dock(i.key(), *i));
  }

private:
  List<Listener*> listeners;
  HashMap<uint_t, String> docks;
  uint_t nextId;
};

class SettingsDlg : private WinAPI::Dialog, private DockManager::Listener
{
public:
  SettingsDlg(Storage& globalStorage);
  ~SettingsDlg();

  UINT show(HWND hwndParent);

private:
  struct DockInfo
  {
    HTREEITEM hItem;
    int_t storageDockIndex;
    List<SettingsPage*> pages;
  };

  Storage& globalStorage;

  DockManager dockManager;
  WinAPI::Button applyButton;
  WinAPI::ImageList pageImageList;
  WinAPI::TreeView pageTreeView;
  HTREEITEM hDocksItem;

  HashMap<UINT, SettingsPage*> pages;
  SettingsPage* currentPage;

  HashMap<uint_t, DockInfo> dockInfoMap;

  void showPage(uint_t dockId, uint_t dialogId);

  // WinAPI::Dialog
  virtual bool onInitDialog();
  virtual bool onDlgMessage(UINT message, WPARAM wParam, LPARAM lParam);
  virtual bool onCommand(UINT command, UINT notificationCode, HWND source);

  // DockManager::Listener
  virtual void_t addedDock(uint_t id, const String& name);
  virtual void_t renamedDock(uint_t id, const String& name);
  virtual void_t removedDock(uint_t id);
};

class SettingsPageGeneral : public SettingsPage
{
private:
  WinAPI::Button autostartButton;

  virtual bool onInitDialog();
  virtual bool onCommand(UINT command, UINT notificationCode, HWND source);
  virtual bool onDlgMessage(UINT message, WPARAM wParam, LPARAM lParam);
};

class SettingsPageDocks : public SettingsPage
{
public:
  SettingsPageDocks(DockManager& dockManager) : dockManager(dockManager) {}

private:
  DockManager& dockManager;

  WinAPI::ImageList imageList;
  WinAPI::Toolbar toolbar;
  WinAPI::ListView listView;

  virtual bool onInitDialog();
  virtual bool onCommand(UINT command, UINT notificationCode, HWND source);
  virtual bool onDlgMessage(UINT message, WPARAM wParam, LPARAM lParam);
};

class SettingsPageDock : public SettingsPage
{
private:
  //virtual bool onInitDialog();
  //virtual bool onCommand(UINT command, UINT notificationCode, HWND source);
};

class SettingsPagePlugins : public SettingsPage
{
public:
  SettingsPagePlugins() : loaded(false) {}

private:
  WinAPI::ListView listView;
  bool loaded;

  bool getPluginInfo(const String& fileName, String& name, String& displayName, uint32_t& version, String& author, String& description);

  virtual bool onInitDialog();
  virtual bool onCommand(UINT command, UINT notificationCode, HWND source);
  virtual bool onDlgMessage(UINT message, WPARAM wParam, LPARAM lParam);
};
