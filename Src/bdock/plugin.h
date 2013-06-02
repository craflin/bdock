
#ifndef Plugin_H
#define Plugin_H

class Icon;
class Dock;

class Plugin
{
public:
  static Plugin* lookup(void* returnAddress);

  Dock* dock;
  Storage* storage;

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
  static std::unordered_map<HMODULE, Plugin*> plugins;

  API::Dock dockAPI;
  API::Plugin* plugin;
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

  struct Interface
  {
    static struct API::Icon* createIcon(HBITMAP icon, unsigned int flags);
    static int destroyIcon(struct API::Icon* icon);
    static int updateIcon(struct API::Icon* icon);
    static int updateIcons(struct API::Icon** icons, uint count);
    static int getIconRect(struct API::Icon* icon, RECT* rect);
    static DWORD showMenu(HMENU hmenu, int x, int y);
    static struct API::Timer* createTimer(unsigned int interval);
    static int updateTimer(struct API::Timer* timer);
    static int destroyTimer(struct API::Timer* timer);
    static int enterStorageSection(const char* name);
    static int enterStorageNumSection(unsigned int pos);
    static int leaveStorageSection();
    static int deleteStorageSection(const char* name);
    static int deleteStorageNumSection(uint pos);
    static unsigned int getStorageNumSectionCount();
    static int setStorageNumSectionCount(unsigned int count);
    static const wchar* getStorageString(const char* name, unsigned int* length, const wchar* default, unsigned int defaultLength);
    static int getStorageInt(const char* name, int default);
    static unsigned int getStorageUInt(const char* name, unsigned int default);
    static int getStorageData(const char* name, char** data, unsigned int* length, const char* defaultData, unsigned int defaultLength);
    static int setStorageString(const char* name, const wchar_t* value, unsigned int length);
    static int setStorageInt(const char* name, int value);
    static int setStorageUInt(const char* name, unsigned int value);
    static int setStorageData(const char* name, char* data, unsigned int length);
    static int deleteStorageEntry(const char* name);
  };
};

#endif
