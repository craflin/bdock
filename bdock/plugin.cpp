
#include "stdafx.h"

extern "C" 
{
typedef struct API::Plugin* (*PCREATEPROC)();
typedef int (*PINITPROC)(struct API::Plugin*);
typedef void (*PDESTROYPROC)(struct API::Plugin*);
}

std::map<struct API::Plugin*, Plugin*> Plugin::plugins;

Plugin* Plugin::lookup(struct API::Plugin* key)
{
  std::map<struct API::Plugin*, Plugin*>::iterator i = plugins.find(key);
  if(i != plugins.end())
    return i->second;
  return 0;
}

Plugin::Plugin(Dock* dock, Storage* storage) : dock(dock), storage(storage), hmodule(0), key(0), lastIcon(0) {}

Plugin::~Plugin()
{
  if(hmodule)
  {
    if(key)
    {
      PDESTROYPROC destroy = (PDESTROYPROC)GetProcAddress(hmodule, "destroy");
      if(destroy)
        destroy(key);
      plugins.erase(key);
      key = 0;
    }

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
  ASSERT(!hmodule && !key);

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

  PCREATEPROC create = (PCREATEPROC)GetProcAddress(hmodule, "create");
  if(!create)
    return false;

  key = create();
  if(!key)
    return false;

  if(plugins.find(key) != plugins.end())
  {
    PDESTROYPROC destroy = (PDESTROYPROC)GetProcAddress(hmodule, "destroy");
    if(destroy)
      destroy(key);
    key = 0;
    return false;
  }

  plugins[key] = this;

  PINITPROC init = (PINITPROC)GetProcAddress(hmodule, "init");
  if(init && init(key) != 0)
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
