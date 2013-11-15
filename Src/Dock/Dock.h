
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

  bool saveStorage();

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
  HashSet<Icon*> icons;
  Icon* lastHitIcon;
  Icon* hotIcon;
  HashSet<Plugin*> plugins;
  HashSet<Timer*> timers;

  enum DragState
  {
    DRAG_IDLE,
    DRAG_CLICKED,
    DRAG_STARTED,
  } dragState;
  POINT dragPosition;
  Icon* dragIcon;
  HIMAGELIST hDragImageList;

  bool loadSkin(const String& name);
  bool loadPlugin(const String& name, Storage& storage);

  void addPlugin(Plugin* plugin) { plugins.append(plugin); }
  void removePlugin(Plugin* plugin) { plugins.remove(plugin); }
  void deletePlugin(Plugin* plugin);

  void update(RECT* update);
  void draw(HDC dest, const RECT& update);
  void calcIconRects(const HashSet<Icon*>::Iterator& firstToUpdate);

  Icon* hitTest(int x, int y);

  void dragStart(Icon& icon, int x, int y);
  void dragFinish();
  void dragMove(int x, int y);

  void handleContextMenu(int x, int y);
  bool handleMouseEvent(UINT message, int x, int y);

  virtual LRESULT onMessage(UINT message, WPARAM wParam, LPARAM lParam);

  static bool isFullscreen(HWND hwnd, HMONITOR hmon);
};
