
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

Plugin::Plugin(Dock* dock, Storage* storage) : dock(dock), storage(storage), plugin(0), hmodule(0), lastIcon(0) {}

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

  std::wstring path(L"plugins/");
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
  dockAPI.showMenu = Interface::showMenu;
  dockAPI.createTimer = Interface::createTimer;
  dockAPI.updateTimer = Interface::updateTimer;
  dockAPI.destroyTimer = Interface::destroyTimer;
  dockAPI.enterStorageSection = Interface::enterStorageSection;
  dockAPI.enterStorageNumSection = Interface::enterStorageNumSection;
  dockAPI.leaveStorageSection = Interface::leaveStorageSection;
  dockAPI.deleteStorageSection = Interface::deleteStorageSection;
  dockAPI.deleteStorageNumSection = Interface::deleteStorageNumSection;
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

API::Icon* Plugin::createIcon(HBITMAP icon, uint flags)
{
  Icon* newIcon = new Icon(icon, flags, this, lastIcon ? lastIcon : (dock->settings.alignment == Settings::right ? 0 : dock->lastIcon), dock->firstIcon);
  addIcon(newIcon);
  dock->addIcon(newIcon);
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
  std::set<Icon*>::iterator i = icons.find((Icon*)icon);
  if(i == icons.end())
    return false;
  *rect = (*i)->rect;
  rect->left += dock->pos.x;
  rect->right += dock->pos.x;
  rect->top += dock->pos.y;
  rect->bottom += dock->pos.y;
  return true;
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
  icons.insert(icon);
  lastIcon = icon;
}

void Plugin::removeIcon(Icon* icon)
{
  icons.erase(icon);
  if(icon == lastIcon)
  {
    if(icons.begin() == icons.end())
      lastIcon = 0;
    else
    {
      for(;;)
      {
        lastIcon = lastIcon->previous;
        if(!lastIcon || lastIcon->plugin == this)
          break;
      }
    }
  }
}

void Plugin::deleteTimer(Timer* timer)
{
  removeTimer(timer);
  dock->removeTimer(timer);
  delete timer;
}


struct API::Icon* Plugin::Interface::createIcon(HBITMAP icon, unsigned int flags)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin)
    return plugin->createIcon(icon, flags);
  return 0;
}

int Plugin::Interface::destroyIcon(struct API::Icon* icon)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin && plugin->destroyIcon(icon))
    return 0;
  return -1;
}

int Plugin::Interface::updateIcon(struct API::Icon* icon)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin && plugin->updateIcon(icon))
    return 0;
  return -1;
}

int Plugin::Interface::updateIcons(struct API::Icon** icons, uint count)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin && plugin->updateIcons(icons, count))
    return 0;
  return -1;
}

int Plugin::Interface::getIconRect(struct API::Icon* icon, RECT* rect)
{
  Plugin* plugin = lookup(_ReturnAddress());
  if(plugin && plugin->getIconRect(icon, rect))
    return 0;
  return -1;
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
