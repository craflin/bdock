
#include "stdafx.h"

extern "C" 
{
typedef struct API::Plugin* (*PCREATEPROC)(struct API::Dock*);
typedef int (*PINITPROC)(struct API::Plugin*);
typedef void (*PDESTROYPROC)(struct API::Plugin*);
}

std::unordered_map<HMODULE, Plugin*> Plugin::plugins;

Plugin* Plugin::lookup(void* returnAddress)
{
  HMODULE hmodule = NULL;
  if(!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCTSTR) returnAddress, &hmodule))
    return 0;
  std::unordered_map<HMODULE, Plugin*>::iterator i = plugins.find(hmodule);
  if(i != plugins.end())
    return i->second;
  return 0;
}

Plugin::Plugin(Dock* dock, Storage* storage) : dock(dock), storage(storage), plugin(0), hmodule(0) {}

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

    plugins.erase(hmodule);
    FreeLibrary(hmodule);
    hmodule = 0;
  }
  
  while(icons.begin() != icons.end())
    deleteIcon(*icons.begin());
  while(timers.begin() != timers.end())
    deleteTimer(*timers.begin());
}

bool Plugin::init(const wchar* name)
{
  ASSERT(!hmodule && !plugin);

  std::wstring path(L"Plugins/");
  path += name;
  path += L'/';
  path += name;
  path += L".dll";

  hmodule = LoadLibrary(path.c_str());
  if(!hmodule)
  {
    path = name;
    path += L".dll";

    hmodule = LoadLibrary(path.c_str());
    if(!hmodule)
      return false;
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
  plugins[hmodule] = this;

  PINITPROC init = (PINITPROC)GetProcAddress(hmodule, "init");
  if(init && init(plugin) != 0)
    return false;

  return true;
}

void Plugin::swapIcon(Icon* icon1, Icon* icon2)
{
  auto a = icons.find(icon1);
  auto b = icons.find(icon2);
  auto c = icons.insert(b, (Icon*) 0);
  icons.erase(b);
  icons.insert(a, icon2);
  icons.erase(a);
  icons.insert(c, icon1);
  icons.erase(c);
}

API::Icon* Plugin::createIcon(HBITMAP icon, uint flags)
{
  Icon* newIcon = new Icon(icon, flags, this);
  Icon* lastIcon = icons.empty() ? 0 : icons.back();
  addIcon(newIcon);
  dock->addIcon(lastIcon, newIcon);
  dock->update();
  return newIcon;
}

bool Plugin::destroyIcon(API::Icon* icon)
{
  if(icons.find((Icon*)icon) == icons.end())
    return false;
  deleteIcon((Icon*)icon);
  dock->update();
  return true;
}

bool Plugin::updateIcon(API::Icon* icon)
{
  if(icons.find((Icon*)icon) == icons.end())
    return false;
  dock->updateIcon((Icon*)icon);
  return true;
}

bool Plugin::updateIcons(API::Icon** icons, uint count)
{
  //if(icons.find((Icon*)icon) == icons.end())
    //return false;
  // TODO: optimize this
  dock->update();
  return true;
}

bool Plugin::getIconRect(API::Icon* icon, RECT* rect)
{
  std::unordered_set<Icon*>::iterator i = icons.find((Icon*)icon);
  if(i == icons.end())
    return false;
  *rect = (*i)->rect;
  const POINT& pos = dock->getPosition();
  rect->left += pos.x;
  rect->right += pos.x;
  rect->top += pos.y;
  rect->bottom += pos.y;
  return true;
}

API::Icon* Plugin::getFirstIcon()
{
  return icons.empty() ? 0 : icons.front();
}

API::Icon* Plugin::getLastIcon()
{
  return icons.empty() ? 0 : icons.back();
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

API::Timer* Plugin::createTimer(uint interval)
{
  Timer* newTimer = new Timer(interval, this);
  addTimer(newTimer);
  dock->addTimer(newTimer);
  return newTimer;
}

bool Plugin::updateTimer(API::Timer* timer)
{
  if(timers.find((Timer*)timer) == timers.end())
    return false;
  dock->updateTimer((Timer*)timer);
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
  dock->removeIcon(icon);
  delete icon;
}

void Plugin::addIcon(Icon* icon)
{
  icons.push_back(icon);
}

void Plugin::removeIcon(Icon* icon)
{
  icons.erase(icon);
}

void Plugin::deleteTimer(Timer* timer)
{
  removeTimer(timer);
  dock->removeTimer(timer);
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

int Plugin::Interface::updateIcons(API::Icon** icons, uint count)
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
    return plugin->dock->showMenu(hmenu, x, y);
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

int Plugin::Interface::enterStorageSection(const char* name)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin && plugin->storage->enterSection(name))
    return 0;
  return -1;
}

int Plugin::Interface::enterStorageNumSection(unsigned int pos)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin && plugin->storage->enterNumSection(pos))
    return 0;
  return -1;
}

int Plugin::Interface::leaveStorageSection()
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin)
  {
    Storage* storage = plugin->storage;
    if(storage->getCurrentStorage() != storage && storage->leave())
      return 0;
  }
  return -1;
}

int Plugin::Interface::deleteStorageSection(const char* name)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin && plugin->storage->deleteSection(name))
    return 0;
  return -1;
}

int Plugin::Interface::deleteStorageNumSection(uint pos)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin && plugin->storage->deleteNumSection(pos))
    return 0;
  return -1;
}

int Plugin::Interface::swapStorageNumSections(unsigned int pos1, unsigned int pos2)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin && plugin->storage->swapNumSections(pos1, pos2))
    return 0;
  return -1;
}

unsigned int Plugin::Interface::getStorageNumSectionCount()
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin)
    return plugin->storage->getNumSectionCount();
  return 0;
}

int Plugin::Interface::setStorageNumSectionCount(unsigned int count)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin && plugin->storage->setNumSectionCount(count))
    return 0;
  return -1;
}

const wchar* Plugin::Interface::getStorageString(const char* name, unsigned int* length, const wchar* default, unsigned int defaultLength)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin)
    return plugin->storage->getStr(name, length, default, defaultLength);
  if(length)
    *length = defaultLength;
  return default;
}

int Plugin::Interface::getStorageInt(const char* name, int default)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin)
    return plugin->storage->getInt(name, default);
  return default;
}

unsigned int Plugin::Interface::getStorageUInt(const char* name, unsigned int default)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin)
    return plugin->storage->getUInt(name, default);
  return default;
}

int Plugin::Interface::getStorageData(const char* name, char** data, unsigned int* length, const char* defaultData, unsigned int defaultLength)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin)
    return plugin->storage->getData(name, data, length, defaultData, defaultLength) ? 0 : -1;
  *data = (char*)defaultData;
  if(length)
    *length = defaultLength;
  return -1;
}

int Plugin::Interface::setStorageString(const char* name, const wchar_t* value, unsigned int length)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin && plugin->storage->setStr(name, value, length))
    return 0;
  return -1;
}

int Plugin::Interface::setStorageInt(const char* name, int value)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin && plugin->storage->setInt(name, value))
    return 0;
  return -1;
}

int Plugin::Interface::setStorageUInt(const char* name, unsigned int value)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin && plugin->storage->setUInt(name, value))
    return 0;
  return -1;
}

int Plugin::Interface::setStorageData(const char* name, char* data, unsigned int length)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin && plugin->storage->setData(name, data, length))
    return 0;
  return -1;
}

int Plugin::Interface::deleteStorageEntry(const char* name)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin && plugin->storage->deleteEntry(name))
    return 0;
  return -1;
}
