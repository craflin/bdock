
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

    static HINSTANCE getInstance() {return hinstance;}

    UINT run();

  private:
    static HINSTANCE hinstance;
  };

  class Shell
  {
  public:
    static bool getFolderPath(INT folder, LPTSTR path, UINT size);
    static bool createLink(LPCTSTR file, LPCTSTR link, LPCTSTR description);

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

  class Window
  {
  public:
    Window() : hwnd(0) {}
    ~Window();

    bool create(LPCTSTR className, UINT windowClassStyle = CS_DBLCLKS, 
      HWND hwndParent = NULL, Icon& icon = Icon(), Icon& smallIcon = Icon(), 
      Cursor& cursor = Cursor(IDC_ARROW), LPCTSTR windowName = _T(""), 
      UINT exStyle = 0, UINT style = WS_POPUP);

    bool setWindowTheme(LPCTSTR pszSubAppName, LPCTSTR pszSubIdList);

    operator HWND() {return hwnd;}

  protected:
    HWND hwnd;

    virtual LRESULT onMessage(UINT message, WPARAM wParam, LPARAM lParam);
  };

  class Dialog : public Window
  {
  public:
    Dialog() {}
    ~Dialog();

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

    INT add(Icon& icon);

    operator HIMAGELIST() {return himagelist;}

  private:
    HIMAGELIST himagelist;
  };

  class ListView : public Control
  {
  public:
    bool setImageList(ImageList& imageList, INT iImageList);
    INT addItem(LVITEM& item);
  };

  class TreeView : public Control
  {
  public:
    bool setImageList(ImageList& imageList, INT iImageList);
    HTREEITEM insertItem(TVINSERTSTRUCT& item);
  };

} // namespace
