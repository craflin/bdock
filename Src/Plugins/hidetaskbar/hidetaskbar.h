
#ifndef HideTaskBar_H
#define HideTaskBar_H

class HideTaskBar
{
public:
  HideTaskBar();
  ~HideTaskBar();

  bool init();

private:
  static int instances;
  static bool hidden;
  static LPARAM originalState;

  static ATOM wndClass;
  static HWND hwnd;

  static bool showTaskBar(bool show, LPARAM& state);

  static LRESULT CALLBACK HideTaskBar::wndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};

#endif //HideTaskBar_H

