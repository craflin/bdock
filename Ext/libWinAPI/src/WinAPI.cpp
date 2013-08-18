
#include "winapi.h"

#include <Shlobj.h>
#include <Shellapi.h>
#include <Uxtheme.h>
#include <Windowsx.h>

#ifndef NDEBUG
#include <cassert>
#define ASSERT(e) assert(e)
#define VERIFY(e) ASSERT(e)
#else
#define ASSERT(e)
#define VERIFY(e) e
#endif

#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "UxTheme.lib")

namespace WinAPI 
{
  HINSTANCE Application::hinstance = 0;

  Application::Application(HINSTANCE hinstance, DWORD dwICC)
  {
    Application::hinstance = hinstance;

    INITCOMMONCONTROLSEX iccex;
    iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    iccex.dwICC = dwICC;
    VERIFY(InitCommonControlsEx(&iccex));
  }

  void Application::setModule(HMODULE hmodule)
  {
    Application::hinstance = hmodule;
  }

  UINT Application::run()
  {
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
      //if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
    return (UINT) msg.wParam;
  }

  void Application::quit(UINT exitCode) {PostQuitMessage(exitCode);}

  bool Shell::getFolderPath(INT folder, LPTSTR path, UINT size)
  {
    if(size < MAX_PATH)
    {
      TCHAR maxPath[MAX_PATH];
      if(SHGetFolderPath(0, folder, 0, SHGFP_TYPE_CURRENT, maxPath) == S_FALSE)
        return false;
      _tcscpy_s(path, size, maxPath);
      return true;
    }
    else if(SHGetFolderPath(0, folder, 0, SHGFP_TYPE_CURRENT, path) == S_FALSE)
        return false;
    return true;
  }

  bool Shell::createLink(LPCTSTR file, LPCTSTR link, LPCTSTR description, LPCTSTR workingDir)
  { 
      bool success = false;

#ifdef UNICODE
      LPCTSTR wsFile = file;
      LPCTSTR wsLink = link;
      LPCTSTR wsDescription = description;
      LPCTSTR wsWorkingDir = workingDir;
#else
      WCHAR wsFile[MAX_PATH]; 
      WCHAR wsLink[MAX_PATH]; 
      WCHAR wsDescription[MAX_PATH]; 
      WCHAR wsWorkingDir[MAX_PATH]; 
      MultiByteToWideChar(CP_ACP, 0, file, -1, wsFile, MAX_PATH); 
      MultiByteToWideChar(CP_ACP, 0, link, -1, wsLink, MAX_PATH); 
      MultiByteToWideChar(CP_ACP, 0, description, -1, wsDescription, MAX_PATH); 
      MultiByteToWideChar(CP_ACP, 0, workingDir, -1, wsWorkingDir, MAX_PATH); 
#endif
 
      // Get a pointer to the IShellLink interface. It is assumed that CoInitialize
      // has already been called.
      IShellLink* psl; 
      HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl); 
      if (SUCCEEDED(hres)) 
      {
          // Set the path to the shortcut target and add the description. 
          psl->SetPath(wsFile); 
          psl->SetDescription(wsDescription); 
          psl->SetWorkingDirectory(wsWorkingDir);

          // Query IShellLink for the IPersistFile interface, used for saving the 
          // shortcut in persistent storage. 
          IPersistFile* ppf; 
          hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf); 

          if (SUCCEEDED(hres)) 
          { 
              // Add code here to check return value from MultiByteWideChar 
              // for success.

              // Save the link by calling IPersistFile::Save. 
              hres = ppf->Save(wsLink, TRUE);
              if (SUCCEEDED(hres))
                success = true;
              ppf->Release();
          }
          psl->Release();
      } 
      return success;
  }

  HINSTANCE Shell::execute(HWND hwnd, LPCTSTR lpOperation, LPCTSTR lpFile, LPCTSTR lpParameters, 
      LPCTSTR lpDirectory, INT nShowCmd)
  {
    return ShellExecute(hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);
  }

  String::String(UINT stringId)
  {
    TCHAR buffer[4096];
    int len = LoadString(Application::getInstance(), stringId, buffer, sizeof(buffer) / sizeof(TCHAR));
    if(len < sizeof(buffer) / sizeof(TCHAR) - 1)
    {
      if(len > 0)
      {
        data = new TCHAR[len + 1];
        memcpy(data, buffer, (len + 1) * sizeof(TCHAR));
        this->len = len;
      }
      else
      {
        data = 0;
        this->len = 0;
      }
    }
    else
    {
      for(;;)
      {
        INT dataSize = len * 2;
        data = new TCHAR[dataSize];
        len = LoadString(Application::getInstance(), stringId, data, dataSize);
        if(len < dataSize - 1)
          break;
        delete[] data;
      }
      this->len = len;
    }
  }

  String::String(LPCTSTR str, UINT len)
  {
    data = new TCHAR[len + 1];
    this->len = len;
    memcpy(data, str, len * sizeof(TCHAR));
    data[len] = 0;
  }

  String::~String()
  {
    delete[] data;
  }

  void String::assign(LPCTSTR str, UINT len)
  {
    delete[] data;
    data = new TCHAR[len + 1];
    this->len = len;
    memcpy(data, str, len * sizeof(TCHAR));
    data[len] = 0;
  }

  void String::alloc(UINT len)
  {
    delete[] data;
    data = new TCHAR[len + 1];
    this->len = len;
    data[0] = 0;
  }

  void String::resize(UINT len)
  {
    if(len <= this->len)
    {
      this->len = len;
      data[len] = 0;
    }
    else
    {
      TCHAR* newData = new TCHAR[len + 1];
      memcpy(newData, data, (this->len + 1) * sizeof(TCHAR));
      newData[len] = 0;
      this->len = len;
      delete[] data;
      data = newData;
    }
  }

  Icon::Icon(LPCTSTR icon) : hicon(LoadIcon(NULL, icon)) {}
  Icon::Icon(UINT iconId) : hicon(LoadIcon(Application::getInstance(), MAKEINTRESOURCE(iconId))) {}
  Icon::Icon(HINSTANCE hinst, UINT iconId) : hicon(LoadIcon(hinst, MAKEINTRESOURCE(iconId))) {}

  Cursor::Cursor(LPCTSTR cursor) : hcursor(LoadCursor(NULL, cursor)) {}
  Cursor::Cursor(UINT cursorId) : hcursor(LoadCursor(Application::getInstance(), MAKEINTRESOURCE(cursorId))) {}
  Cursor::Cursor(HINSTANCE hinst, UINT cursorId) : hcursor(LoadCursor(hinst, MAKEINTRESOURCE(cursorId))) {}

  Menu::Menu(UINT menuId) : hmenu(LoadMenu(WinAPI::Application::getInstance(), MAKEINTRESOURCE(menuId))) {}
  Menu::~Menu()
  {
    if(hmenu)
      DestroyMenu(hmenu);
  }

  Window::~Window()
  {
    if(hwnd)
      DestroyWindow(hwnd);
  }

  bool Window::create(LPCTSTR className, UINT windowClassStyle, 
      HWND hwndParent, HICON icon, HICON smallIcon, 
      HCURSOR cursor, LPCTSTR windowName, UINT exStyle, UINT style)
  {
    if(hwnd)
      return false; // already created

    struct WndProc
    {
      static LRESULT CALLBACK wndProcInit(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
      {
        if(message == WM_CREATE)
        {
          LPCREATESTRUCT cs = (LPCREATESTRUCT) lParam;
          Window* wnd = (Window*) cs->lpCreateParams;
          wnd->hwnd = hwnd;
          SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) wnd);
          SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR) wndProc);
          return wnd->onMessage(message, wParam, lParam);
        }
        return DefWindowProc(hwnd, message, wParam, lParam);
      }
      static LRESULT CALLBACK wndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
      {
        Window* wnd = (Window*) GetWindowLongPtr(hwnd, GWLP_USERDATA);
        return wnd->onMessage(message, wParam, lParam);
      }
    };

    // register window class
    HINSTANCE hinst = Application::getInstance();
    {
      WNDCLASSEX wcex;
      wcex.cbSize = sizeof(WNDCLASSEX);
      wcex.style = windowClassStyle; //CS_HREDRAW | CS_VREDRAW;
      wcex.lpfnWndProc = WndProc::wndProcInit;
      wcex.cbClsExtra = 0;
      wcex.cbWndExtra = 0;
      wcex.hInstance = hinst;
      wcex.hIcon = icon;
      wcex.hCursor = cursor;
      wcex.hbrBackground = 0; //(HBRUSH)(COLOR_WINDOW+1);
      wcex.lpszMenuName = 0; //MAKEINTRESOURCE(IDC_BDOCK);
      wcex.lpszClassName = className;
      wcex.hIconSm = smallIcon;
      RegisterClassEx(&wcex);
    }

    // create window
    hwnd = CreateWindowEx(exStyle, className, windowName, style, 0, 0, 0, 0, hwndParent, NULL, hinst, this);
    if(!hwnd)
      return false;

    return true;
  }

  LRESULT Window::onMessage(UINT message, WPARAM wParam, LPARAM lParam)
  {
    switch(message)
    {
    case WM_CONTEXTMENU:
      if(onContextMenu(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
        return 0;
      break;
    case WM_COMMAND:
      if(onCommand(LOWORD(wParam), HIWORD(wParam), (HWND)lParam))
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
  }

  bool Window::getText(String& text)
  {
    int len = GetWindowTextLength(hwnd);
    if(len < 0)
      return false;
    text.alloc(len);
    if(len > 0)
    {
      len = GetWindowText(hwnd, (LPTSTR) text, len + 1);
      if(len < 0)
        return false;
    }
    return len == 0 ? GetLastError() == ERROR_SUCCESS : true;
  }

  bool Window::setText(LPCTSTR text)
  {
    return SetWindowText(hwnd, text) == TRUE;
  }

  HWND Window::getParent() const {return GetParent(hwnd);}
  bool Window::setTheme(LPCTSTR pszSubAppName, LPCTSTR pszSubIdList) {return SetWindowTheme(hwnd, pszSubAppName, pszSubIdList) == S_OK;}
  bool Window::isVisible() {return IsWindowVisible(hwnd) == TRUE;}
  HWND Window::setFocus() {return SetFocus(hwnd);}
  bool Window::show(INT nCmdShow) {return ShowWindow(hwnd, nCmdShow) == TRUE;}
  bool Window::move(INT X, INT Y, INT nWidth, INT nHeight, bool bRepaint) {return MoveWindow(hwnd, X, Y, nWidth, nHeight, bRepaint) == TRUE;}
  bool Window::move(const RECT& rect, bool bRepaint) {return MoveWindow(hwnd, rect.left, rect.top, rect.right - rect.left, rect.bottom -  rect.top, bRepaint) == TRUE;}
  void Window::setStyle(UINT style) {SetWindowLong(hwnd, GWL_STYLE, style);}
  UINT Window::getStyle() const {return GetWindowLong(hwnd, GWL_STYLE);}
  bool Window::enable(bool enable) {return EnableWindow(hwnd, enable) == TRUE;}
  bool Window::isEnabled() const {return IsWindowEnabled(hwnd) == TRUE;}
  bool Window::registerShellHookWindow() {return RegisterShellHookWindow(hwnd) == TRUE;}
  bool Window::deregisterShellHookWindow() {return DeregisterShellHookWindow(hwnd) == TRUE;}
  bool Window::setTimer(UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc) {return SetTimer(hwnd, nIDEvent, uElapse, lpTimerFunc) == TRUE;}
  bool Window::killTimer(UINT_PTR nIDEvent) {return KillTimer(hwnd, nIDEvent) == TRUE;}
  LRESULT Window::sendMessage(UINT msg, WPARAM wParam, LPARAM lParam) {return SendMessage(hwnd, msg, wParam, lParam);}
  bool Window::postMessage(UINT msg, WPARAM wParam, LPARAM lParam) {return PostMessage(hwnd, msg, wParam, lParam) == TRUE;}
  bool Window::getRect(RECT& rect) {return GetWindowRect(hwnd, &rect) == TRUE;}
  bool Window::getClientRect(RECT& rect) {return GetClientRect(hwnd, &rect) == TRUE;}

  Dialog::~Dialog()
  {
    if(hwnd)
    {
      end(IDCANCEL);
      hwnd = 0;
    }
  }

  bool Dialog::create(UINT ressourceId, HWND hwndParent)
  {
    if(hwnd)
      return 0; // already created

    struct DlgProc
    {
      static INT_PTR CALLBACK dlgProcInit(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
      {
        if(message == WM_INITDIALOG)
        {
          Dialog* dlg = (Dialog*) lParam;
          dlg->hwnd = hDlg;
          SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR) dlg);
          SetWindowLongPtr(hDlg, DWLP_DLGPROC, (LONG_PTR) dlgProc);
          if(dlg->onDlgMessage(message, wParam, lParam))
            return (INT_PTR)TRUE;
        }
        return (INT_PTR)FALSE;
      }
      static INT_PTR CALLBACK dlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
      {
        Dialog* dlg = (Dialog*) GetWindowLongPtr(hDlg, DWLP_USER);
        if(dlg->onDlgMessage(message, wParam, lParam))
          return (INT_PTR)TRUE;
        return (INT_PTR)FALSE;
      }
    };

    HWND hwnd = CreateDialogParam(Application::getInstance(), MAKEINTRESOURCE(ressourceId), hwndParent, DlgProc::dlgProcInit, (LPARAM)this);
    return hwnd != NULL;
  }

  UINT Dialog::showBox(UINT ressourceId, HWND hwndParent)
  {
    if(hwnd)
      return 0; // already created

    struct DlgProc
    {
      static INT_PTR CALLBACK dlgProcInit(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
      {
        if(message == WM_INITDIALOG)
        {
          Dialog* dlg = (Dialog*) lParam;
          dlg->hwnd = hDlg;
          SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR) dlg);
          SetWindowLongPtr(hDlg, DWLP_DLGPROC, (LONG_PTR) dlgProc);
          if(dlg->onDlgMessage(message, wParam, lParam))
            return (INT_PTR)TRUE;
        }
        return (INT_PTR)FALSE;
      }
      static INT_PTR CALLBACK dlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
      {
        Dialog* dlg = (Dialog*) GetWindowLongPtr(hDlg, DWLP_USER);
        if(dlg->onDlgMessage(message, wParam, lParam))
          return (INT_PTR)TRUE;
        return (INT_PTR)FALSE;
      }
    };

    UINT result = (UINT) DialogBoxParam(Application::getInstance(), MAKEINTRESOURCE(ressourceId), hwndParent, DlgProc::dlgProcInit, (LPARAM)this);
    hwnd = 0;
    return result;
  }

  bool Dialog::end(UINT result)
  {
    return EndDialog(hwnd, result) != FALSE;
  }

  bool Dialog::onDlgMessage(UINT message, WPARAM wParam, LPARAM lParam)
  {
    switch(message)
    {
    case WM_INITDIALOG: return onInitDialog();
    case WM_COMMAND: return onCommand(LOWORD(wParam), HIWORD(wParam), (HWND) lParam);
    }
    return false;
  }

  bool Dialog::onInitDialog()
  {
    return true;
  }

  bool Dialog::onCommand(UINT command, UINT notificationCode, HWND source)
  {
    if(command == IDOK || command == IDCANCEL)
    {
      end(command);
      return true;
    }
    return false;
  }

  Control::~Control()
  {
    if(attached)
      hwnd = 0; // avoid DestroyWindow call
  }

  bool Control::attach(HWND hwnd)
  {
    if(this->hwnd)
      return false;
    this->hwnd = hwnd;
    if(hwnd)
    {
      attached = true;
      return true;
    }
    return false;
  }

  bool Control::attach(HWND parent, UINT id)
  {
    if(hwnd)
      return false;
    hwnd = GetDlgItem(parent, id);
    if(hwnd)
    {
      attached = true;
      return true;
    }
    return false;
  }

  bool Button::setCheck(INT check)
  {
    Button_SetCheck(hwnd, check);
    return true;
  }

  INT Button::getCheck()
  {
    return Button_GetCheck(hwnd);
  }

  ImageList::~ImageList()
  {
    if(himagelist)
      ImageList_Destroy(himagelist);
  }

  bool ImageList::create(INT cx, INT cy, UINT flags, INT cInitial, INT cGrow)
  {
    if(himagelist)
      return false;
    himagelist = ImageList_Create(cx, cy, flags, cInitial, cGrow);
    return himagelist != NULL;
  }

  INT ImageList::add(HICON icon)
  {
    return ImageList_AddIcon(himagelist, icon);
  }

  bool ListView::setImageList(HIMAGELIST imageList, INT iImageList)
  {
    ListView_SetImageList(hwnd, imageList, iImageList);
    return true;
  }

  INT ListView::addItem(LVITEM& item)
  {
    item.iItem = ListView_GetItemCount(hwnd);
    return ListView_InsertItem(hwnd, &item);
  }

  bool ListView::deleteItem(INT item) {return ListView_DeleteItem(hwnd, item) == TRUE;}
  bool ListView::deleteAllItems() {return ListView_DeleteAllItems(hwnd) == TRUE;}
  bool ListView::setItem(LVITEM& item) {return ListView_SetItem(hwnd, &item) == TRUE;}

  bool ListView::setItemText(INT item, INT subItem, LPCTSTR text)
  {
    LVITEM lvi;
    lvi.mask = LVIF_TEXT;
    lvi.iItem = item;
    lvi.iSubItem = subItem;
    lvi.pszText = (LPTSTR) text;
    return ListView_SetItem(hwnd, &lvi) == TRUE;
  }

  bool ListView::getItemData(INT item, void*& data)
  {
    LVITEM lvi;
    lvi.mask = LVIF_PARAM;
    lvi.iItem = item;
    lvi.iSubItem = 0;
    if(!ListView_GetItem(hwnd, &lvi))
      return false;
    data = (void*)lvi.lParam;
    return true;
  }

  bool ListView::getEditControl(Edit& edit)
  {
    HWND edithwnd = ListView_GetEditControl(hwnd);
    if(edithwnd == NULL)
      return false;
    return edit.attach(edithwnd);
  }

  INT ListView::getFirstItem(UINT flags) {return ListView_GetNextItem(hwnd, -1, flags);}
  INT ListView::getNextItem(INT start, UINT flags) {return ListView_GetNextItem(hwnd, start, flags);}
  HWND ListView::editLabel(INT item) {return ListView_EditLabel(hwnd, item);}

  bool ListView::setCheckState(INT item, BOOL fCheck)
  {
    ListView_SetCheckState(hwnd, (UINT)item, fCheck);
    return true;
  }

  bool ListView::setView(UINT viewStyle)
  {
    return ListView_SetView(hwnd, viewStyle) == 1;
  }

  bool ListView::setExtendedStyle(UINT extendedListViewStyle)
  {
    ListView_SetExtendedListViewStyle(hwnd, extendedListViewStyle);
    return true;
  }

  bool TreeView::setImageList(HIMAGELIST imageList, INT iImageList)
  {
    TreeView_SetImageList(hwnd, imageList, iImageList);
    return true;
  }

  HTREEITEM TreeView::insertItem(TVINSERTSTRUCT& item)
  {
    return TreeView_InsertItem(hwnd, &item);
  }

  bool TreeView::deleteItem(HTREEITEM hItem) {return TreeView_DeleteItem(hwnd, hItem) == TRUE;}
  bool TreeView::deleteAllItems() {return TreeView_DeleteAllItems(hwnd) == TRUE;}
  bool TreeView::getItem(TVITEM& item) {return TreeView_GetItem(hwnd, &item) == TRUE;}
  HTREEITEM TreeView::getParent(HTREEITEM hItem) {return TreeView_GetParent(hwnd, hItem);}
  HTREEITEM TreeView::getItemParent(HTREEITEM hItem) {return TreeView_GetParent(hwnd, hItem);}

  LPARAM TreeView::getItemData(HTREEITEM hItem)
  {
    TVITEM tvi;
    tvi.hItem = hItem;
    tvi.mask = TVIF_PARAM;
    if(TreeView_GetItem(hwnd, &tvi) != TRUE)
      return 0;
    return tvi.lParam;
  }

  bool TreeView::setItemText(HTREEITEM hitem, LPCTSTR str)
  {
    TVITEM tvi;
    tvi.hItem = hitem;
    tvi.mask = TVIF_TEXT;
    tvi.pszText = (LPTSTR) str;
    return TreeView_SetItem(hwnd, &tvi) != 0;
  }

  bool TreeView::getItemText(HTREEITEM hItem, String& str)
  {
    TCHAR bufferBuf[4096];
    TVITEM tvi;
    tvi.pszText = bufferBuf;
    tvi.cchTextMax = sizeof(bufferBuf) / sizeof(*bufferBuf) - 1;
    tvi.hItem = hItem;
    tvi.mask = TVIF_TEXT;
    for(;;)
    {
      if(TreeView_GetItem(hwnd, &tvi) != TRUE)
        return false;
      int len = (int)_tcslen(tvi.pszText);
      if(len >= tvi.cchTextMax - 1)
      {
        str.alloc(len * 2);
        tvi.pszText = (LPTSTR)str;
        tvi.cchTextMax = str.length();
        continue;
      }
      if(tvi.pszText != (LPTSTR)str)
        str.assign(tvi.pszText, len);
      else
        str.resize(len);
      return true;
    }
  }

  bool TreeView::selectItem(HTREEITEM hItem)
  {
    return TreeView_SelectItem(hwnd, hItem) == TRUE;
  }

  bool Toolbar::create(HWND hParent, UINT windowStyle)
  {
    if(hwnd)
      return false;
    hwnd = CreateWindowEx(0, TOOLBARCLASSNAME, NULL,  windowStyle, 0, 0, 0, 0, hParent, NULL, Application::getInstance(), NULL);
    return hwnd != NULL;
  }

  bool Toolbar::setImageList(INT imageListID, HIMAGELIST hImageList)
  {
    SendMessage(hwnd, TB_SETIMAGELIST,  (WPARAM)imageListID,  (LPARAM)hImageList);
    return true;
  }

  bool Toolbar::addButtons(TBBUTTON* buttons, UINT numButtons)
  {
    SendMessage(hwnd, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
    return SendMessage(hwnd, TB_ADDBUTTONS, (WPARAM)numButtons, (LPARAM)buttons) == TRUE;
  }

  bool Toolbar::autoSize()
  {
    SendMessage(hwnd, TB_AUTOSIZE, 0, 0); 
    return true;
  }

  bool Toolbar::setExtendedStyle(UINT extendedStyle)
  {
    SendMessage(hwnd, TB_SETEXTENDEDSTYLE, 0, (LPARAM) extendedStyle);
    return true;
  }

  bool Toolbar::setButtonState(UINT cmdId, BYTE state)
  {
    TBBUTTONINFO tbInfo;
    tbInfo.cbSize  = sizeof(TBBUTTONINFO);
    tbInfo.dwMask  = TBIF_STATE;
    tbInfo.fsState = state;
    return SendMessage(hwnd, TB_SETBUTTONINFO, (WPARAM)cmdId, (LPARAM)&tbInfo) != 0;
  }

} // namespace
