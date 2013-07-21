
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

  bool Shell::createLink(LPCTSTR file, LPCTSTR link, LPCTSTR description)
  { 
      bool success = false;

#ifdef UNICODE
      LPCTSTR wsFile = file;
      LPCTSTR wsLink = link;
      LPCTSTR wsDescription = description;
#else
      WCHAR wsFile[MAX_PATH]; 
      WCHAR wsLink[MAX_PATH]; 
      WCHAR wsDescription[MAX_PATH]; 
      MultiByteToWideChar(CP_ACP, 0, file, -1, wsFile, MAX_PATH); 
      MultiByteToWideChar(CP_ACP, 0, link, -1, wsLink, MAX_PATH); 
      MultiByteToWideChar(CP_ACP, 0, description, -1, wsDescription, MAX_PATH); 
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
#ifdef UNICODE
    data = 0;
    buffer = 0;
    LoadString(Application::getInstance(), stringId, (LPTSTR)&data, 0);
#else
    data = 0;
    buffer = 0;
    LPCWSTR wdata;
    LoadStringW(Application::getInstance(), stringId, (LPTSTR)&wdata, 0);
    if(wdata)
    {
      int len = wstrlen(wdata);
      data = buffer = new TCHAR[len];
      WideCharToMultiByte(
#error TODO
    }
#endif
  }

  String::~String()
  {
    if(buffer)
      delete[] buffer;
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
      HWND hwndParent, Icon& icon, Icon& smallIcon, 
      Cursor& cursor, LPCTSTR windowName, UINT exStyle, UINT style)
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
      if(onCommand(LOWORD(wParam), (HWND) lParam))
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
  }

  bool Window::setTheme(LPCTSTR pszSubAppName, LPCTSTR pszSubIdList) {return SetWindowTheme(hwnd, pszSubAppName, pszSubIdList) == S_OK;}
  bool Window::isVisible() {return IsWindowVisible(hwnd) == TRUE;}
  bool Window::show(INT nCmdShow) {return ShowWindow(hwnd, nCmdShow) == TRUE;}
  bool Window::move(INT X, INT Y, INT nWidth, INT nHeight, bool bRepaint) {return MoveWindow(hwnd, X, Y, nWidth, nHeight, bRepaint) == TRUE;}
  bool Window::registerShellHookWindow() {return RegisterShellHookWindow(hwnd) == TRUE;}
  bool Window::deregisterShellHookWindow() {return DeregisterShellHookWindow(hwnd) == TRUE;}
  bool Window::setTimer(UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc) {return SetTimer(hwnd, nIDEvent, uElapse, lpTimerFunc) == TRUE;}
  bool Window::killTimer(UINT_PTR nIDEvent) {return KillTimer(hwnd, nIDEvent) == TRUE;}
  LRESULT Window::sendMessage(UINT msg, WPARAM wParam, LPARAM lParam) {return SendMessage(hwnd, msg, wParam, lParam);}
  bool Window::postMessage(UINT msg, WPARAM wParam, LPARAM lParam) {return PostMessage(hwnd, msg, wParam, lParam) == TRUE;}

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

  UINT Dialog::show(UINT ressourceId, HWND hwndParent)
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
    case WM_COMMAND: return onCommand(LOWORD(wParam), (HWND) lParam);
    }
    return false;
  }

  bool Dialog::onInitDialog()
  {
    return true;
  }

  bool Dialog::onCommand(UINT command, HWND source)
  {
    if(command == IDOK || command == IDCANCEL)
    {
      end(command);
      return true;
    }
    return false;
  }

  bool Control::initialize(Window& parent, UINT id)
  {
    if(hwnd)
      return false;
    hwnd = GetDlgItem(parent, id);
    return hwnd != NULL;
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

  INT ImageList::add(Icon& icon)
  {
    return ImageList_AddIcon(himagelist, icon);
  }

  bool ListView::setImageList(ImageList& imageList, INT iImageList)
  {
    ListView_SetImageList(hwnd, imageList, iImageList);
    return true;
  }

  INT ListView::addItem(LVITEM& item)
  {
    item.iItem = ListView_GetItemCount(hwnd);
    return ListView_InsertItem(hwnd, &item);
  }

  bool TreeView::setImageList(ImageList& imageList, INT iImageList)
  {
    TreeView_SetImageList(hwnd, imageList, iImageList);
    return true;
  }

  HTREEITEM TreeView::insertItem(TVINSERTSTRUCT& item)
  {
    return TreeView_InsertItem(hwnd, &item);
    return 0;
  }

} // namespace
