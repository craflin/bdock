
#ifndef Plugin_H
#define Plugin_H

class Icon;
class Dock;

class Plugin
{
public:
  static Plugin* lookup(struct API::Plugin* key);

  Dock* dock;
  Storage* storage;
  struct API::Plugin* key;

  Plugin(Dock* dock, Storage* storage);
  ~Plugin();

  bool init(const wchar* name);

  // api functions
  API::Icon* createIcon(HBITMAP icon, uint flags);
  bool destroyIcon(API::Icon* icon);
  bool updateIcon(API::Icon* icon);
  bool updateIcons(API::Icon** icons, uint count);
  bool getIconRect(API::Icon* icon, RECT* rect);
  API::Timer* createTimer(uint interval);
  bool updateTimer(API::Timer* timer);
  bool destroyTimer(API::Timer* timer);

private:
  static std::map<struct API::Plugin*, Plugin*> plugins;
  
  HMODULE hmodule;
  std::set<Icon*> icons;
  stdext::hash_set<Timer*> timers;
  Icon* lastIcon;

  void addIcon(Icon* icon);
  void removeIcon(Icon* icon);
  void deleteIcon(Icon* icon);

  void addTimer(Timer* timer) { timers.insert(timer); }
  void removeTimer(Timer* timer) { timers.erase(timer); }
  void deleteTimer(Timer* timer);
};

#endif