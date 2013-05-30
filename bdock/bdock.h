
#ifndef BDock_H
#define BDock_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef BDOCK_EXPORTS
#define BDOCK_API __declspec(dllexport)
#else
#define BDOCK_API __declspec(dllexport)
#endif

struct Icon;
struct Plugin;

typedef int (*PMOUSEEVENTPROC)(struct Plugin* plugin, struct Icon*, unsigned int message, int x, int y);
typedef int (*PTIMERPROC)(struct Plugin* plugin, struct Timer*);

struct Plugin
{
};

struct Icon 
{
  HBITMAP icon;  
  unsigned int flags;
  void* userData;
  PMOUSEEVENTPROC mouseEventProc;
  //char title[32];
};

struct Timer
{
  unsigned int interval;
  void* userData;
  PTIMERPROC timerProc;
};

#define IF_GHOST 0x01
#define IF_ACTIVE 0x02
#define IF_SMALL 0x04
#define IF_HALFBG 0x08
#define IF_FULLBG 0x10


BDOCK_API struct Icon* createIcon(struct Plugin* plugin, HBITMAP icon, unsigned int flags);
BDOCK_API int destroyIcon(struct Plugin* plugin, struct Icon* icon);
BDOCK_API int updateIcon(struct Plugin* plugin, struct Icon* icon);
BDOCK_API int updateIcons(struct Plugin* plugin, struct Icon** icons, unsigned int count);
BDOCK_API int getIconRect(struct Plugin* plugin, struct Icon* icon, RECT* rect);
BDOCK_API DWORD showMenu(struct Plugin* plugin, HMENU hmenu, int x, int y);
BDOCK_API HBITMAP createBitmapFromIcon(HICON icon, SIZE* size);

BDOCK_API struct Timer* createTimer(struct Plugin* plugin, unsigned int interval);
BDOCK_API int updateTimer(struct Plugin* plugin, struct Timer* timer);
BDOCK_API int destroyTimer(struct Plugin* plugin, struct Timer* timer);

BDOCK_API int enterStorageSection(struct Plugin* plugin, const char* name);
BDOCK_API int enterStorageNumSection(struct Plugin* plugin, unsigned int pos);
BDOCK_API int leaveStorageSection(struct Plugin* plugin);

BDOCK_API int deleteStorageSection(struct Plugin* plugin, const char* name);
BDOCK_API int deleteStorageNumSection(struct Plugin* plugin, unsigned int pos);

BDOCK_API unsigned int getStorageNumSectionCount(struct Plugin* plugin);
BDOCK_API int setStorageNumSectionCount(struct Plugin* plugin, unsigned int count);

BDOCK_API const wchar_t* getStorageString(struct Plugin* plugin, const char* name, unsigned int* length, const wchar_t* default, unsigned int defaultLength);
BDOCK_API int getStorageInt(struct Plugin* plugin, const char* name, int default);
BDOCK_API unsigned int getStorageUInt(struct Plugin* plugin, const char* name, unsigned int default);
BDOCK_API int getStorageData(struct Plugin* plugin, const char* name, char** data, unsigned int* length, const char* defaultData, unsigned int defaultLength);
BDOCK_API int setStorageString(struct Plugin* plugin, const char* name, const wchar_t* value, unsigned int length);
BDOCK_API int setStorageInt(struct Plugin* plugin, const char* name, int value);
BDOCK_API int setStorageUInt(struct Plugin* plugin, const char* name, unsigned int value);
BDOCK_API int setStorageData(struct Plugin* plugin, const char* name, char* data, unsigned int length);

BDOCK_API int deleteStorageEntry(struct Plugin* plugin, const char* name);

#ifdef __cplusplus
}
#endif 

#endif BDock_H
