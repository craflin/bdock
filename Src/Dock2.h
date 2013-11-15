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
  int (*handleMoveEvent)(struct Icon* icon);
  void* userData;
};

#define IF_GHOST 0x01
#define IF_ACTIVE 0x02
#define IF_SMALL 0x04
#define IF_HALFBG 0x08 // TODO: remove this?
#define IF_FULLBG 0x10 // TODO: remove this?
#define IF_HOT 0x20

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

  struct Icon* (*getFirstIcon)();
  struct Icon* (*getLastIcon)();
  struct Icon* (*getNextIcon)(struct Icon* icon);
  struct Icon* (*getPreviousIcon)(struct Icon* icon);

  DWORD (*showMenu)(HMENU hmenu, int x, int y);

  struct Timer* (*createTimer)(unsigned int interval);
  int (*updateTimer)(struct Timer* timer);
  int (*destroyTimer)(struct Timer* timer);

  int (*enterStorageSection)(const wchar_t* name);
  int (*enterStorageNumSection)(unsigned int pos);
  int (*leaveStorageSection)();

  int (*deleteStorageSection)(const wchar_t* name);
  int (*deleteStorageNumSection)(unsigned int pos);
  int (*swapStorageNumSections)(unsigned int pos1, unsigned int pos2);

  unsigned int (*getStorageNumSectionCount)();
  int (*setStorageNumSectionCount)(unsigned int count);

  const wchar_t* (*getStorageString)(const wchar_t* name, unsigned int* length, const wchar_t* default, unsigned int defaultLength);
  int (*getStorageInt)(const wchar_t* name, int default);
  unsigned int (*getStorageUInt)(const wchar_t* name, unsigned int default);
  int (*getStorageData)(const wchar_t* name, const void** data, unsigned int* length, const void* defaultData, unsigned int defaultLength);
  int (*setStorageString)(const wchar_t* name, const wchar_t* value, unsigned int length);
  int (*setStorageInt)(const wchar_t* name, int value);
  int (*setStorageUInt)(const wchar_t* name, unsigned int value);
  int (*setStorageData)(const wchar_t* name, char* data, unsigned int length);

  int (*deleteStorageEntry)(const wchar_t* name);
};

#ifdef __cplusplus
}
#endif 
