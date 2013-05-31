
#ifndef Launcher_H
#define Launcher_H

class IconData
{
public:
  HICON hicon;
  Icon* icon;
  HWND hwnd;
  std::wstring path;
  std::wstring parameters;
  bool pinned;

  IconData(HICON hicon, Icon* icon, HWND hwnd, const std::wstring& path, const std::wstring& parameters);
  ~IconData();
};

class Launcher : public Plugin
{
public:
  Launcher();
  ~Launcher();

  bool init();

private:
  static ATOM wndClass;
  static int instances;
  HICON defaultIcon;
  HWND hwnd;
  HWND activeHwnd;
  std::map<HWND,Icon*> icons;
  std::vector<Icon*> launchers;

  static int mouseEventProc(struct Plugin* plugin, Icon* icon, unsigned int message, int x, int y);

  static LRESULT CALLBACK wndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

  void addIcon(HWND hwnd);
  void activateIcon(HWND hwnd);
  void removeIcon(HWND hwnd);
  void updateIcon(HWND hwnd, bool forceUpdate);

  bool hasTaskBarIcon(HWND hwnd);

  void showContextMenu(Icon* icon, int x, int y);
  bool launch(Icon& icon);
  bool getCommandLine(HWND hwnd, std::wstring& path, std::wstring& parameters);
};

#endif //Launcher_H

