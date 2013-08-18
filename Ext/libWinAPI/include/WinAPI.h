
#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <Windows.h>
#include <tchar.h>
#include <CommCtrl.h>
#include <shlobj.h>

namespace WinAPI 
{
  class Uncopyable
  {
  public:
    Uncopyable() {}
  private:
    Uncopyable(const Uncopyable&);
    Uncopyable& operator=(const Uncopyable&);
  };

  class Application : Uncopyable
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

  class Shell : Uncopyable
  {
  public:
    static bool getFolderPath(INT folder, LPTSTR path, UINT size);
    static bool createLink(LPCTSTR file, LPCTSTR link, LPCTSTR description, LPCTSTR workingDir);

    static HINSTANCE execute(HWND hwnd, LPCTSTR lpOperation, LPCTSTR lpFile, LPCTSTR lpParameters, 
      LPCTSTR lpDirectory, INT nShowCmd);
  };

  class String : Uncopyable
  {
  public:
    String() : data(0), len(0) {}
    String(UINT stringId);
    String(LPCTSTR str, UINT len);
    ~String();

    void assign(LPCTSTR str, UINT len);
    void alloc(UINT len);
    void resize(UINT len);

    UINT length() const {return len;}

    operator LPCTSTR() const {return data;}
    operator LPTSTR() {return data;}

  private:
    LPTSTR data;
    UINT len;
  };

  class Icon : Uncopyable
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

  class Cursor : Uncopyable
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

  class Menu : Uncopyable
  {
  public:
    Menu() : hmenu(0) {}
    Menu(UINT menuId);
    ~Menu();

  private:
    HMENU hmenu;
  };

  class Window : Uncopyable
  {
  public:
    Window() : hwnd(0) {}
    virtual ~Window();

    bool create(LPCTSTR className, UINT windowClassStyle = CS_DBLCLKS, 
      HWND hwndParent = NULL, HICON icon = NULL, HICON smallIcon = NULL, 
      HCURSOR cursor = Cursor(IDC_ARROW), LPCTSTR windowName = _T(""), 
      UINT exStyle = 0, UINT style = WS_POPUP);

    bool setText(LPCTSTR text);
    bool getText(String& text);
    HWND getParent() const;
    bool setTheme(LPCTSTR pszSubAppName, LPCTSTR pszSubIdList);
    bool isVisible();
    HWND setFocus();
    bool show(INT nCmdShow);
    bool move(INT X, INT Y, INT nWidth, INT nHeight, bool bRepaint);
    bool move(const RECT& rect, bool bRepaint);
    void setStyle(UINT style);
    UINT getStyle() const;
    bool enable(bool enable);
    bool isEnabled() const;
    bool getRect(RECT& rect);
    bool getClientRect(RECT& rect);

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

    virtual bool onCommand(UINT command, UINT notificationCode, HWND source) {return false;}
    virtual bool onContextMenu(INT x, INT y) {return false;}
  };

  class Dialog : public Window
  {
  public:
    Dialog() {}
    virtual ~Dialog();

    bool create(UINT ressourceId, HWND hwndParent);
    UINT showBox(UINT ressourceId, HWND hwndParent = NULL);

    bool end(UINT result);

  protected:

    virtual bool onDlgMessage(UINT message, WPARAM wParam, LPARAM lParam);

    virtual bool onInitDialog();

    virtual bool onCommand(UINT command, UINT notificationCode, HWND source);
  };

  class Control : public Window
  {
  public:
    Control() : attached(false) {}
    ~Control();

    bool attach(HWND hwnd);
    bool attach(HWND parent, UINT id);
  private:
    bool attached;
  };

  class Button : public Control
  {
  public:
    bool setCheck(INT check);
    INT getCheck();
  };

  class Edit : public Control
  {
  public:
  };

  class ImageList : Uncopyable
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
    bool deleteItem(INT item);
    bool deleteAllItems();
    bool setView(UINT viewStyle);
    bool setExtendedStyle(UINT extendedListViewStyle);
    bool setItem(LVITEM& item);
    bool setItemText(INT item, INT subItem, LPCTSTR text);
    bool getItemData(INT item, void*& data);
    bool getEditControl(Edit& edit);
    INT getFirstItem(UINT flags);
    INT getNextItem(INT start, UINT flags);
    HWND editLabel(INT item);
    bool setCheckState(INT item, BOOL fCheck);
  };

  class TreeView : public Control
  {
  public:
    bool setImageList(HIMAGELIST imageList, INT iImageList);
    HTREEITEM insertItem(TVINSERTSTRUCT& item);
    bool deleteItem(HTREEITEM hItem);
    bool deleteAllItems();
    bool getItem(TVITEM& item);
    HTREEITEM getParent(HTREEITEM hItem);
    HTREEITEM getItemParent(HTREEITEM hItem);
    LPARAM getItemData(HTREEITEM hItem);
    bool selectItem(HTREEITEM hItem);
    bool setItemText(HTREEITEM hItem, LPCTSTR str);
    bool getItemText(HTREEITEM hItem, String& str);
  };

  class Toolbar : public Control
  {
  public:
    bool create(HWND hParent, UINT windowStyle = WS_CHILD | WS_VISIBLE | TBSTYLE_WRAPABLE | TBSTYLE_LIST);

    bool setImageList(INT imageListID, HIMAGELIST hImageList);
    bool addButtons(TBBUTTON* buttons, UINT numButtons);
    bool autoSize();
    bool setExtendedStyle(UINT extendedStyle);
    bool setButtonState(UINT cmdId, BYTE state);
  };

} // namespace
