
#pragma once

class DllInjection
{
public:
  DllInjection();
  ~DllInjection();

  bool init(DWORD pid, const wchar_t* dll);

private:
  HANDLE process;
public:
  HMODULE injectedModule;
};

extern WNDPROC TrayWndProc;
LRESULT CALLBACK TrayWndProcHook(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef struct _NOTIFYICONDATA32 {
    DWORD cbSize;
    DWORD hWnd;
    UINT uID;
    UINT uFlags;
    UINT uCallbackMessage;
    DWORD hIcon;
    WCHAR  szTip[128];
    DWORD dwState;
    DWORD dwStateMask;
    WCHAR  szInfo[256];
    union {
        UINT  uTimeout;
        UINT  uVersion;  // used with NIM_SETVERSION, values 0, 3 and 4
    } DUMMYUNIONNAME;
    WCHAR  szInfoTitle[64];
    DWORD dwInfoFlags;
    GUID guidItem;
    DWORD hBalloonIcon;
} NOTIFYICONDATA32, *PNOTIFYICONDATA32;

class SystemTray;

class IconData
{
public:
  SystemTray& systemTray;
  HICON hicon;
  Icon* icon;
  HWND hwnd;
  uint_t callbackMessage;
  uint_t version;
  uint_t id;

  IconData(SystemTray& systemTray, HICON hicon, Icon* icon, PNOTIFYICONDATA32 nid);
  ~IconData();
};

class SystemTray
{
public:
  Dock& dock;

  SystemTray(Dock& dock);
  ~SystemTray();

  bool init();

private:
  static ATOM wndClass;
  static int instances;

  DllInjection dllInjection;
  HWND hwnd;
  HashMap<String, Icon*> icons;

  static int handleMouseEvent(Icon* icon, unsigned int message, int x, int y);
  static LRESULT CALLBACK wndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

  void addIcon(PNOTIFYICONDATA32 nid);
  void updateIcon2(PNOTIFYICONDATA32 nid);
  void removeIcon(PNOTIFYICONDATA32 nid);
  void setIconVersion(PNOTIFYICONDATA32 nid);
};
