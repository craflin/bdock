
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
  bool pinned;

  IconData(Launcher& launcher, HICON hicon, Icon* icon, HWND hwnd, const std::wstring& path, const std::wstring& parameters);
  ~IconData();
};

class Launcher : public WinAPI::Window
{
public:
  Launcher(Dock& dock);
  ~Launcher();

  bool create();

private:

  static int instances;

  Dock& dock;
  WinAPI::Icon defaultIcon;
  HWND activeHwnd;
  std::unordered_map<HWND, Icon*> icons;
  std::vector<Icon*> launchers;

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
