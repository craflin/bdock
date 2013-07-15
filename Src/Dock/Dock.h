
#pragma once

class Icon;
class Plugin;

class Dock
{
public:
  static HINSTANCE hinstance;

  Icon* lastIcon;
  Icon* firstIcon;
  Settings settings;
  POINT pos;

  Dock(Storage& globalStorage, Storage* dockStorage);
  ~Dock();

  bool init(HINSTANCE hinstance);

  void addIcon(Icon* icon);
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
  static ATOM wndClass;

  Storage& globalStorage;
  Storage* storage;
  HWND hwnd;
  Skin* skin;
  HBITMAP bmp;
  SIZE size;
  int iconCount;
  std::unordered_set<Plugin*> plugins;
  Icon* lastHitIcon;
  std::unordered_set<Timer*> timers;
  HWND activeHwnd;
  bool activeHwndRudeFullscreen;

  bool loadSkin(const wchar* name);
  bool loadPlugin(const wchar* name, Storage* storage);

  void addPlugin(Plugin* plugin) { plugins.insert(plugin); }
  void removePlugin(Plugin* plugin) { plugins.erase(plugin); }
  void deletePlugin(Plugin* plugin);

  void update(RECT* update);
  void draw(HDC dest, const RECT& update);
  void calcIconRects(Icon* previous, Icon* firstToUpdate);

  Icon* hitTest(int x, int y);

  void handleContextMenu(int x, int y);
  bool handleMouseEvent(UINT message, int x, int y);

  static LRESULT CALLBACK wndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

  static bool isFullscreen(HWND hwnd);
};