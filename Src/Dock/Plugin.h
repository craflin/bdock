
#pragma once

class Icon;
class Dock;

class Plugin
{
public:
  static Plugin* lookup(void_t* returnAddress);

  Dock& dock;
  Storage& storage;

  Plugin(Dock& dock, Storage& storage);
  ~Plugin();

  bool_t init(const String& name);

  void_t swapIcons(Icon* icon1, Icon* icon2);

  // api functions
  API::Icon* createIcon(HBITMAP icon, uint_t flags);
  bool_t destroyIcon(API::Icon* icon);
  bool_t updateIcon(API::Icon* icon);
  bool_t updateIcons(API::Icon** icons, uint_t count);
  bool_t getIconRect(API::Icon* icon, RECT* rect);
  API::Icon* getFirstIcon();
  API::Icon* getLastIcon();
  API::Icon* getNextIcon(API::Icon* icon);
  API::Icon* getPreviousIcon(API::Icon* icon);
  API::Timer* createTimer(uint_t interval);
  bool_t updateTimer(API::Timer* timer);
  bool_t destroyTimer(API::Timer* timer);

private:
  static HashMap<HMODULE, Plugin*> plugins;

  API::Dock dockAPI;
  API::Plugin* plugin;
  HMODULE hmodule;
  HashSet<Icon*> icons;
  HashSet<Timer*> timers;

  void_t addIcon(Icon* icon);
  void_t removeIcon(Icon* icon);
  void_t deleteIcon(Icon* icon);

  void_t addTimer(Timer* timer) { timers.append(timer); }
  void_t removeTimer(Timer* timer) { timers.remove(timer); }
  void_t deleteTimer(Timer* timer);

  struct Interface
  {
    static API::Icon* createIcon(HBITMAP icon, unsigned int flags);
    static int destroyIcon(API::Icon* icon);
    static int updateIcon(API::Icon* icon);
    static int updateIcons(API::Icon** icons, uint_t count);
    static int getIconRect(API::Icon* icon, RECT* rect);
    static API::Icon* getFirstIcon();
    static API::Icon* getLastIcon();
    static API::Icon* getNextIcon(API::Icon* icon);
    static API::Icon* getPreviousIcon(API::Icon* icon);
    static DWORD showMenu(HMENU hmenu, int x, int y);
    static struct API::Timer* createTimer(unsigned int interval);
    static int updateTimer(struct API::Timer* timer);
    static int destroyTimer(struct API::Timer* timer);
    static int enterStorageSection(const wchar_t* name);
    static int enterStorageNumSection(unsigned int pos);
    static int leaveStorageSection();
    static int deleteStorageSection(const wchar_t* name);
    static int deleteStorageNumSection(uint_t pos);
    static int swapStorageNumSections(unsigned int pos1, unsigned int pos2);
    static unsigned int getStorageNumSectionCount();
    static int setStorageNumSectionCount(unsigned int count);
    static const wchar_t* getStorageString(const wchar_t* name, unsigned int* length, const wchar_t* default, unsigned int defaultLength);
    static int getStorageInt(const wchar_t* name, int default);
    static unsigned int getStorageUInt(const wchar_t* name, unsigned int default);
    static int getStorageData(const wchar_t* name, const void** data, unsigned int* length, const void* defaultData, unsigned int defaultLength);
    static int setStorageString(const wchar_t* name, const wchar_t* value, unsigned int length);
    static int setStorageInt(const wchar_t* name, int value);
    static int setStorageUInt(const wchar_t* name, unsigned int value);
    static int setStorageData(const wchar_t* name, char* data, unsigned int length);
    static int deleteStorageEntry(const wchar_t* name);
  };
};
