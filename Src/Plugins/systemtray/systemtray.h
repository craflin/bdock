
#ifndef SystemTray_H
#define SystemTray_H

class DllInjection
{
public:
  DllInjection();
  ~DllInjection();

  bool init(DWORD pid, const wchar* dll);

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

class IconData
{
public:
  HICON hicon;
  Icon* icon;
  HWND hwnd;
  uint callbackMessage;
  uint version;
  uint id;

  IconData(HICON hicon, Icon* icon, PNOTIFYICONDATA32 nid);
  ~IconData();
};

class SystemTray : public Plugin
{
public:
  SystemTray();
  ~SystemTray();

  bool init();

private:
  DllInjection dllInjection;
  static ATOM wndClass;
  static int instances;
  HWND hwnd;
  stdext::hash_map<std::string, Icon*> icons;

  typedef struct Icon* (*PCREATEICON)(struct Plugin* plugin, HBITMAP icon, unsigned int flags);
  typedef int (*PDESTROYICON)(struct Plugin* plugin, struct Icon* icon);
  typedef int (*PUPDATEICON)(struct Plugin* plugin, struct Icon* icon);
  typedef HBITMAP (*PCREATEBITMAPFROMICON)(HICON icon, SIZE* size);

  PCREATEICON createIcon;
  PDESTROYICON destroyIcon;
  PUPDATEICON updateIcon;
  PCREATEBITMAPFROMICON createBitmapFromIcon;

  static int mouseEventProc(struct Plugin* plugin, Icon* icon, unsigned int message, int x, int y);
  static LRESULT CALLBACK wndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

  void addIcon(PNOTIFYICONDATA32 nid);
  void updateIcon2(PNOTIFYICONDATA32 nid);
  void removeIcon(PNOTIFYICONDATA32 nid);
  void setIconVersion(PNOTIFYICONDATA32 nid);
};

#endif //SystemTray_H
