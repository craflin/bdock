
#pragma once

class Icon;
class Plugin;

class Dock : private WinAPI::Window
{
public:
  Dock(Storage& globalStorage, Storage& dockStorage);
  ~Dock();

  bool create();
  const POINT& getPosition() {return pos;}

  void addIcon(Icon* insertAfter, Icon* icon);
  void removeIcon(Icon* icon);
  void addTimer(Timer* timer);
  void removeTimer(Timer* timer);

  void updateIcon(Icon* icon);
  void updateTimer(Timer* timer);
  void update();

  bool showSettingsDlg();

  // api functions
  DWORD showMenu(HMENU hmenu, int x, int y);

private:
  Settings settings;
  POINT pos;
  Storage& globalStorage;
  Storage& dockStorage;
  Skin* skin;
  HBITMAP bmp;
  SIZE size;
  list_set<Icon*> icons;
  std::unordered_set<Plugin*> plugins;
  Icon* lastHitIcon;
  std::unordered_set<Timer*> timers;

  bool loadSkin(const wchar* name);
  bool loadPlugin(const wchar* name, Storage* storage);

  void addPlugin(Plugin* plugin) { plugins.insert(plugin); }
  void removePlugin(Plugin* plugin) { plugins.erase(plugin); }
  void deletePlugin(Plugin* plugin);

  void update(RECT* update);
  void draw(HDC dest, const RECT& update);
  void calcIconRects(const list_set<Icon*>::iterator& firstToUpdate);

  Icon* hitTest(int x, int y);

  void handleContextMenu(int x, int y);
  bool handleMouseEvent(UINT message, int x, int y);

  virtual LRESULT onMessage(UINT message, WPARAM wParam, LPARAM lParam);

  static bool isFullscreen(HWND hwnd);
};
