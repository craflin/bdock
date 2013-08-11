
#pragma once

class Launcher;

class IconData
{
public:
  Launcher& launcher;
  HICON hicon;
  Icon* icon;
  HWND hwnd;
  std::wstring path;
  std::wstring parameters;
  bool pinned; // TODO: remove this and use launcherIndex >= 0 ?
  int launcherIndex;

  IconData(Launcher& launcher, HICON hicon, Icon* icon, HWND hwnd, const std::wstring& path, const std::wstring& parameters, int launcherIndex);
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
  std::list<IconData*> icons;
  std::unordered_map<HWND, IconData*> iconsByHWND;

  static int handleMouseEvent(Icon* icon, unsigned int message, int x, int y);

  virtual LRESULT onMessage(UINT message, WPARAM wParam, LPARAM lParam);

  void addIcon(HWND hwnd);
  void activateIcon(HWND hwnd);
  void removeIcon(HWND hwnd);
  void updateIcon(HWND hwnd, bool forceUpdate);

  static bool hasTaskBarIcon(HWND hwnd);

  void showContextMenu(Icon* icon, int x, int y);
  bool launch(Icon& icon);
  static bool getCommandLine(HWND hwnd, std::wstring& path, std::wstring& parameters);
};
