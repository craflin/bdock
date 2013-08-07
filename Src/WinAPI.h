
#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <Windows.h>
#include <tchar.h>
#include <CommCtrl.h>

namespace WinAPI 
{
  class Application
  {
  public:
    Application(HINSTANCE hinstance, DWORD dwICC =  ICC_WIN95_CLASSES);

    static void setModule(HMODULE hmodule);
    static HINSTANCE getInstance() {return hinstance;}
    static void quit(UINT exitCode);

    UINT run();

  private:
    static HINSTANCE hinstance;
  };

  class Shell
  {
  public:
    static bool getFolderPath(INT folder, LPTSTR path, UINT size);
    static bool createLink(LPCTSTR file, LPCTSTR link, LPCTSTR description, LPCTSTR workingDir);

    static HINSTANCE execute(HWND hwnd, LPCTSTR lpOperation, LPCTSTR lpFile, LPCTSTR lpParameters, 
      LPCTSTR lpDirectory, INT nShowCmd);
  };

  class String 
  {
  public:
    String() : data(0), buffer(0) {}
    String(UINT stringId);
    ~String();

    operator LPCTSTR() {return data;}

  private:
    LPCTSTR data;
    LPTSTR buffer;
  };

  class Icon
  {
  public:
    
    Icon() : hicon(0) {}
    Icon(LPCTSTR icon);
    Icon(UINT iconId);
    Icon(HINSTANCE hinst, UINT iconId);

    operator HICON() {return hicon;}

  private:
    HICON hicon;
  };

  class Cursor
  {
  public:
    Cursor() : hcursor(0) {}
    Cursor(LPCTSTR cursor);
    Cursor(UINT cursorId);
    Cursor(HINSTANCE hinst, UINT cursorId);

    operator HCURSOR() {return hcursor;}

  private:
    HCURSOR hcursor;
  };

  class Menu
  {
  public:
    Menu() : hmenu(0) {}
    Menu(UINT menuId);
    ~Menu();

  private:
    HMENU hmenu;
  };

  class Window
  {
  public:
    Window() : hwnd(0) {}
    ~Window();

    bool create(LPCTSTR className, UINT windowClassStyle = CS_DBLCLKS, 
      HWND hwndParent = NULL, HICON icon = NULL, HICON smallIcon = NULL, 
      HCURSOR cursor = Cursor(IDC_ARROW), LPCTSTR windowName = _T(""), 
      UINT exStyle = 0, UINT style = WS_POPUP);

    bool setTheme(LPCTSTR pszSubAppName, LPCTSTR pszSubIdList);
    bool isVisible();
    bool show(INT nCmdShow);
    bool move(INT X, INT Y, INT nWidth, INT nHeight, bool bRepaint);
    void setStyle(UINT style) {SetWindowLong(hwnd, GWL_STYLE, style);}
    UINT getStyle() const {return GetWindowLong(hwnd, GWL_STYLE);}

    bool registerShellHookWindow();
    bool deregisterShellHookWindow();

    bool setTimer(UINT_PTR nIDEvent, UINT uElaspe, TIMERPROC lpTimerFunc);
    bool killTimer(UINT_PTR nIDEvent);

    LRESULT sendMessage(UINT msg, WPARAM wParam, LPARAM lParam);
    bool postMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    operator HWND() {return hwnd;}

  protected:
    HWND hwnd;

    virtual LRESULT onMessage(UINT message, WPARAM wParam, LPARAM lParam);

    virtual bool onCommand(UINT command, HWND source) {return false;}
    virtual bool onContextMenu(INT x, INT y) {return false;}
  };

  class Dialog : public Window
  {
  public:
    Dialog() {}
    ~Dialog();

    bool create(UINT ressourceId, HWND hwndParent);
    UINT show(UINT ressourceId, HWND hwndParent = NULL);

    bool end(UINT result);

  protected:

    virtual bool onDlgMessage(UINT message, WPARAM wParam, LPARAM lParam);

    virtual bool onInitDialog();

    virtual bool onCommand(UINT command, HWND source);
  };

  class Control : public Window
  {
  public:
    bool initialize(Window& parent, UINT id);
  };

  class Button : public Control
  {
  public:
    bool setCheck(INT check);
    INT getCheck();
  };

  class ImageList
  {
  public:
    ImageList() : himagelist(0) {}
    ~ImageList();

    bool create(INT cx, INT cy, UINT flags, INT cInitial, INT cGrow);

    INT add(HICON icon);

    operator HIMAGELIST() {return himagelist;}

  private:
    HIMAGELIST himagelist;
  };

  class ListView : public Control
  {
  public:
    bool setImageList(HIMAGELIST imageList, INT iImageList);
    INT addItem(LVITEM& item);
  };

  class TreeView : public Control
  {
  public:
    bool setImageList(HIMAGELIST imageList, INT iImageList);
    HTREEITEM insertItem(TVINSERTSTRUCT& item);
  };

} // namespace
