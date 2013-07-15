/**
* Bdock2 plugin api.
* @author Colin Graf
*/

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

struct Icon;
struct Timer;
struct Plugin;

struct Icon
{
  HBITMAP icon;
  unsigned int flags;
  int (*handleMouseEvent)(struct Icon* icon, unsigned int message, int x, int y);
  void* userData;
};

#define IF_GHOST 0x01
#define IF_ACTIVE 0x02
#define IF_SMALL 0x04
#define IF_HALFBG 0x08
#define IF_FULLBG 0x10

struct Timer
{
  unsigned int interval;
  int (*handleTimerEvent)(struct Timer* timer);
  void* userData;
};

struct Dock
{
  struct Icon* (*createIcon)(HBITMAP icon, unsigned int flags);
  int (*destroyIcon)(struct Icon* icon);
  int (*updateIcon)(struct Icon* icon);
  int (*updateIcons)(struct Icon** icons, unsigned int count);
  int (*getIconRect)(struct Icon* icon, RECT* rect);
  DWORD (*showMenu)(HMENU hmenu, int x, int y);

  struct Timer* (*createTimer)(unsigned int interval);
  int (*updateTimer)(struct Timer* timer);
  int (*destroyTimer)(struct Timer* timer);

  int (*enterStorageSection)(const char* name);
  int (*enterStorageNumSection)(unsigned int pos);
  int (*leaveStorageSection)();

  int (*deleteStorageSection)(const char* name);
  int (*deleteStorageNumSection)(unsigned int pos);

  unsigned int (*getStorageNumSectionCount)();
  int (*setStorageNumSectionCount)(unsigned int count);

  const wchar_t* (*getStorageString)(const char* name, unsigned int* length, const wchar_t* default, unsigned int defaultLength);
  int (*getStorageInt)(const char* name, int default);
  unsigned int (*getStorageUInt)(const char* name, unsigned int default);
  int (*getStorageData)(const char* name, char** data, unsigned int* length, const char* defaultData, unsigned int defaultLength);
  int (*setStorageString)(const char* name, const wchar_t* value, unsigned int length);
  int (*setStorageInt)(const char* name, int value);
  int (*setStorageUInt)(const char* name, unsigned int value);
  int (*setStorageData)(const char* name, char* data, unsigned int length);

  int (*deleteStorageEntry)(const char* name);
};

#ifdef __cplusplus
}
#endif 
