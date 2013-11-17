
#include "stdafx.h"

#include <intrin.h>

extern "C" 
{
typedef struct API::Plugin* (*PCREATEPROC)(struct API::Dock*);
typedef int (*PINITPROC)(struct API::Plugin*);
typedef void (*PDESTROYPROC)(struct API::Plugin*);
}

HashMap<HMODULE, Plugin*> Plugin::plugins;

Plugin* Plugin::lookup(void* returnAddress)
{
  HMODULE hmodule = NULL;
  if(!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCTSTR) returnAddress, &hmodule))
    return 0;
  HashMap<HMODULE, Plugin*>::Iterator i = plugins.find(hmodule);
  if(i != plugins.end())
    return *i;
  return 0;
}

Plugin::Plugin(Dock& dock, Storage& storage) : dock(dock), storage(storage), plugin(0), hmodule(0) {}

Plugin::~Plugin()
{
  if(hmodule)
  {
    if(plugin)
    {
      PDESTROYPROC destroy = (PDESTROYPROC)GetProcAddress(hmodule, "destroy");
      if(destroy)
        destroy(plugin);
      plugin = 0;
    }

    plugins.remove(hmodule);
    FreeLibrary(hmodule);
    hmodule = 0;
  }
  
  while(icons.begin() != icons.end())
    deleteIcon(*icons.begin());
  while(timers.begin() != timers.end())
    deleteTimer(*timers.begin());
}

bool Plugin::init(const String& name)
{
  ASSERT(!hmodule && !plugin);

  String path(_T("Plugins/"));
  path += name;
  path += _T('/');
  path += name;
  path += _T(".dll");

  hmodule = LoadLibrary(path);
  if(!hmodule)
  {
    path = name;
    path += _T(".dll");

    hmodule = LoadLibrary(path);
    if(!hmodule)
      return false;
#ifdef _DEBUG
    TCHAR filenameBuf[MAX_PATH];
    DWORD filenameLen = GetModuleFileName(hmodule, filenameBuf, MAX_PATH);
    String copyFilename(filenameBuf, filenameLen);
    copyFilename +=L"-copy";
    FreeLibrary(hmodule);
    if(!CopyFileW(filenameBuf, copyFilename, FALSE))
      return false;
    hmodule = LoadLibrary(copyFilename);
    if(!hmodule)
      return false;
#endif
  }

  if(plugins.find(hmodule) != plugins.end())
    return false;

  PCREATEPROC create = (PCREATEPROC)GetProcAddress(hmodule, "create");
  if(!create)
    return false;

  dockAPI.createIcon = Interface::createIcon;
  dockAPI.destroyIcon = Interface::destroyIcon;
  dockAPI.updateIcon = Interface::updateIcon;
  dockAPI.updateIcons = Interface::updateIcons;
  dockAPI.getIconRect = Interface::getIconRect;
  dockAPI.getFirstIcon = Interface::getFirstIcon;
  dockAPI.getLastIcon = Interface::getLastIcon;
  dockAPI.getNextIcon = Interface::getNextIcon;
  dockAPI.getPreviousIcon = Interface::getPreviousIcon;
  dockAPI.showMenu = Interface::showMenu;
  dockAPI.createTimer = Interface::createTimer;
  dockAPI.updateTimer = Interface::updateTimer;
  dockAPI.destroyTimer = Interface::destroyTimer;
  dockAPI.enterStorageSection = Interface::enterStorageSection;
  dockAPI.enterStorageNumSection = Interface::enterStorageNumSection;
  dockAPI.leaveStorageSection = Interface::leaveStorageSection;
  dockAPI.deleteStorageSection = Interface::deleteStorageSection;
  dockAPI.deleteStorageNumSection = Interface::deleteStorageNumSection;
  dockAPI.swapStorageNumSections = Interface::swapStorageNumSections;
  dockAPI.getStorageNumSectionCount = Interface::getStorageNumSectionCount;
  dockAPI.setStorageNumSectionCount = Interface::setStorageNumSectionCount;
  dockAPI.getStorageString = Interface::getStorageString;
  dockAPI.getStorageInt = Interface::getStorageInt;
  dockAPI.getStorageUInt = Interface::getStorageUInt;
  dockAPI.getStorageData = Interface::getStorageData;
  dockAPI.setStorageString = Interface::setStorageString;
  dockAPI.setStorageInt = Interface::setStorageInt;
  dockAPI.setStorageUInt = Interface::setStorageUInt;
  dockAPI.setStorageData = Interface::setStorageData;
  dockAPI.deleteStorageEntry = Interface::deleteStorageEntry;

  plugin = create(&dockAPI);
  if(!plugin)
    return false;
  plugins.append(hmodule, this);

  PINITPROC init = (PINITPROC)GetProcAddress(hmodule, "init");
  if(init && init(plugin) != 0)
    return false;

  return true;
}

void_t Plugin::swapIcons(Icon* icon1, Icon* icon2)
{
  auto a = icons.find(icon1);
  auto b = icons.find(icon2);
  auto c = icons.insert(b, (Icon*) 0);
  icons.remove(b);
  icons.insert(a, icon2);
  icons.remove(a);
  icons.insert(c, icon1);
  icons.remove(c);
}

API::Icon* Plugin::createIcon(HBITMAP icon, uint_t flags)
{
  Icon* newIcon = new Icon(icon, flags, this);
  Icon* lastIcon = icons.isEmpty() ? 0 : icons.back();
  addIcon(newIcon);
  dock.addIcon(lastIcon, newIcon);
  dock.update();
  return newIcon;
}

bool Plugin::destroyIcon(API::Icon* icon)
{
  if(icons.find((Icon*)icon) == icons.end())
    return false;
  deleteIcon((Icon*)icon);
  dock.update();
  return true;
}

bool Plugin::updateIcon(API::Icon* icon)
{
  if(icons.find((Icon*)icon) == icons.end())
    return false;
  dock.updateIcon((Icon*)icon);
  return true;
}

bool Plugin::updateIcons(API::Icon** icons, uint_t count)
{
  //if(icons.find((Icon*)icon) == icons.end())
    //return false;
  // TODO: optimize this
  dock.update();
  return true;
}

bool Plugin::getIconRect(API::Icon* icon, RECT* rect)
{
  HashSet<Icon*>::Iterator i = icons.find((Icon*)icon);
  if(i == icons.end())
    return false;
  *rect = (*i)->rect;
  const POINT& pos = dock.getPosition();
  rect->left += pos.x;
  rect->right += pos.x;
  rect->top += pos.y;
  rect->bottom += pos.y;
  return true;
}

API::Icon* Plugin::getFirstIcon()
{
  return icons.isEmpty() ? 0 : icons.front();
}

API::Icon* Plugin::getLastIcon()
{
  return icons.isEmpty() ? 0 : icons.back();
}

API::Icon* Plugin::getNextIcon(API::Icon* icon)
{
  auto i = icons.find((Icon*) icon);
  if(i == icons.end())
    return 0;
  ++i;
  return i == icons.end() ? 0 : *i;
}

API::Icon* Plugin::getPreviousIcon(API::Icon* icon)
{
  auto i = icons.find((Icon*) icon);
  if(i == icons.end() || i == icons.begin())
    return 0;
  return *(--i);
}

API::Timer* Plugin::createTimer(uint_t interval)
{
  Timer* newTimer = new Timer(interval, this);
  addTimer(newTimer);
  dock.addTimer(newTimer);
  return newTimer;
}

bool Plugin::updateTimer(API::Timer* timer)
{
  if(timers.find((Timer*)timer) == timers.end())
    return false;
  dock.updateTimer((Timer*)timer);
  return true;
}

bool Plugin::destroyTimer(API::Timer* timer)
{
  if(timers.find((Timer*)timer) == timers.end())
    return false;
  deleteTimer((Timer*)timer);
  return true;
}

void Plugin::deleteIcon(Icon* icon)
{
  removeIcon(icon);
  dock.removeIcon(icon);
  delete icon;
}

void Plugin::addIcon(Icon* icon)
{
  icons.append(icon);
}

void Plugin::removeIcon(Icon* icon)
{
  icons.remove(icon);
}

void Plugin::deleteTimer(Timer* timer)
{
  removeTimer(timer);
  dock.removeTimer(timer);
  delete timer;
}


API::Icon* Plugin::Interface::createIcon(HBITMAP icon, unsigned int flags)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin)
    return plugin->createIcon(icon, flags);
  return 0;
}

int Plugin::Interface::destroyIcon(API::Icon* icon)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin && plugin->destroyIcon(icon))
    return 0;
  return -1;
}

int Plugin::Interface::updateIcon(API::Icon* icon)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin && plugin->updateIcon(icon))
    return 0;
  return -1;
}

int Plugin::Interface::updateIcons(API::Icon** icons, uint_t count)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin && plugin->updateIcons(icons, count))
    return 0;
  return -1;
}

int Plugin::Interface::getIconRect(API::Icon* icon, RECT* rect)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin && plugin->getIconRect(icon, rect))
    return 0;
  return -1;
}

API::Icon* Plugin::Interface::getFirstIcon()
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin)
    return plugin->getFirstIcon();
  return 0;
}

API::Icon* Plugin::Interface::getLastIcon()
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin)
    return plugin->getLastIcon();
  return 0;
}

API::Icon* Plugin::Interface::getNextIcon(API::Icon* icon)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin)
    return plugin->getNextIcon(icon);
  return 0;
}

API::Icon* Plugin::Interface::getPreviousIcon(API::Icon* icon)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin)
    return plugin->getPreviousIcon(icon);
  return 0;
}

DWORD Plugin::Interface::showMenu(HMENU hmenu, int x, int y)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin)
    return plugin->dock.showMenu(hmenu, x, y);
  return 0;
}

struct API::Timer* Plugin::Interface::createTimer(unsigned int interval)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin)
    return plugin->createTimer(interval);
  return 0;
}

int Plugin::Interface::updateTimer(struct API::Timer* timer)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin && plugin->updateTimer(timer))
    return 0;
  return -1;
}

int Plugin::Interface::destroyTimer(struct API::Timer* timer)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin && plugin->destroyTimer(timer))
    return 0;
  return -1;
}

int Plugin::Interface::enterStorageSection(const wchar_t* name)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin)
  {
    String nameStr;
    nameStr.attach(name, String::length(name));
    if(plugin->storage.enterSection(nameStr))
      return 0;
  }
  return -1;
}

int Plugin::Interface::enterStorageNumSection(unsigned int pos)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin && plugin->storage.enterNumSection(pos))
    return 0;
  return -1;
}

int Plugin::Interface::leaveStorageSection()
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin)
  {
    Storage* storage = &plugin->storage;
    if(storage->getCurrentStorage() != storage && storage->leave())
      return 0;
  }
  return -1;
}

int Plugin::Interface::deleteStorageSection(const wchar_t* name)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if (plugin)
  {
    String nameStr;
    nameStr.attach(name, String::length(name));
    if(plugin->storage.deleteSection(nameStr) && plugin->dock.saveStorage())
      return 0;
  }
  return -1;
}

int Plugin::Interface::deleteStorageNumSection(uint_t pos)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin && plugin->storage.deleteNumSection(pos) && plugin->dock.saveStorage())
    return 0;
  return -1;
}

int Plugin::Interface::swapStorageNumSections(unsigned int pos1, unsigned int pos2)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin && plugin->storage.swapNumSections(pos1, pos2) && plugin->dock.saveStorage())
    return 0;
  return -1;
}

unsigned int Plugin::Interface::getStorageNumSectionCount()
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin)
    return plugin->storage.getNumSectionCount();
  return 0;
}

int Plugin::Interface::setStorageNumSectionCount(unsigned int count)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin && plugin->storage.setNumSectionCount(count) && plugin->dock.saveStorage())
    return 0;
  return -1;
}

const wchar_t* Plugin::Interface::getStorageString(const wchar_t* name, unsigned int* length, const wchar_t* default, unsigned int defaultLength)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin)
  {
    String defaultStr;
    String nameStr;
    nameStr.attach(name, String::length(name));
    const String& result = plugin->storage.getStr(nameStr, defaultStr);
    if(&result != &defaultStr)
    {
      if(length)
        *length = (unsigned int)result.length();
      return &result[0];
    }
  }
  if(length)
    *length = defaultLength;
  return default;
}

int Plugin::Interface::getStorageInt(const wchar_t* name, int default)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin)
  {
    String nameStr;
    nameStr.attach(name, String::length(name));
    return plugin->storage.getInt(nameStr, default);
  }
  return default;
}

unsigned int Plugin::Interface::getStorageUInt(const wchar_t* name, unsigned int default)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin)
    return plugin->storage.getUInt(String(name, String::length(name)), default);
  return default;
}

int Plugin::Interface::getStorageData(const wchar_t* name, const void** data, unsigned int* length, const void* defaultData, unsigned int defaultLength)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin)
  {
    String nameStr;
    nameStr.attach(name, String::length(name));
    return plugin->storage.getData(nameStr, *data, *length, defaultData, defaultLength) ? 0 : -1;
  }
  *data = (char*)defaultData;
  if(length)
    *length = defaultLength;
  return -1;
}

int Plugin::Interface::setStorageString(const wchar_t* name, const wchar_t* value, unsigned int length)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if (plugin)
  {
    String nameStr;
    nameStr.attach(name, String::length(name));
    if(plugin->storage.setStr(nameStr, String(value, length)) && plugin->dock.saveStorage())
      return 0;
  }
  return -1;
}

int Plugin::Interface::setStorageInt(const wchar_t* name, int value)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if (plugin)
  {
    String nameStr;
    nameStr.attach(name, String::length(name));
    if(plugin->storage.setInt(nameStr, value) && plugin->dock.saveStorage())
      return 0;
  }
  return -1;
}

int Plugin::Interface::setStorageUInt(const wchar_t* name, unsigned int value)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if (plugin)
  {
    String nameStr;
    nameStr.attach(name, String::length(name));
    if(plugin->storage.setUInt(nameStr, value) && plugin->dock.saveStorage())
      return 0;
  }
  return -1;
}

int Plugin::Interface::setStorageData(const wchar_t* name, char* data, unsigned int length)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if (plugin)
  {
    String nameStr;
    nameStr.attach(name, String::length(name));
    if(plugin->storage.setData(nameStr, data, length) && plugin->dock.saveStorage())
      return 0;
  }
  return -1;
}

int Plugin::Interface::deleteStorageEntry(const wchar_t* name)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if (plugin)
  {
    String nameStr;
    nameStr.attach(name, String::length(name));
    if(plugin->storage.deleteEntry(nameStr) && plugin->dock.saveStorage())
      return 0;
  }
  return -1;
}
