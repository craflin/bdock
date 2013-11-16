
#pragma once

class Launcher;

class IconData
{
public:
  Launcher& launcher;
  HICON hicon;
  Icon* icon;
  HWND hwnd;
  String path;
  String parameters;
  bool pinned; // TODO: remove this and use launcherIndex >= 0 ?
  int launcherIndex;

  IconData(Launcher& launcher, HICON hicon, Icon* icon, HWND hwnd, const String& path, const String& parameters, int launcherIndex);
  ~IconData();
};

class Launcher : public WinAPI::Window
{
public:
  Dock& dock;

  Launcher(Dock& dock);
  ~Launcher();

  bool create();

private:
  WinAPI::Icon defaultIcon;
  HWND activeHwnd;
  IconData* hotIcon;
  HashSet<IconData*> icons;
  HashMap<HWND, IconData*> iconsByHWND;

  static int handleMouseEvent(Icon* icon, unsigned int message, int x, int y);
  static int handleMoveEvent(Icon* icon);

  virtual LRESULT onMessage(UINT message, WPARAM wParam, LPARAM lParam);

  void addIcon(HWND hwnd);
  void activateIcon(HWND hwnd);
  void removeIcon(HWND hwnd);
  void removeIcon(IconData& iconData);
  void updateIcon(HWND hwnd, bool forceUpdate);

  static bool hasTaskBarIcon(HWND hwnd);

  void showContextMenu(Icon* icon, int x, int y);
  bool launch(Icon& icon);
  static bool getCommandLine(HWND hwnd, String& path, String& parameters);
};
