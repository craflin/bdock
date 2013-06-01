
#ifndef WMI_H
#define WMI_H

#ifdef __cplusplus
extern "C"
{
#endif

BOOL wmiInit();
void wmiCleanup();
BOOL wmiRequest(LPCWSTR query);
BOOL wmiRequestFormat(LPCWSTR format, ...);
BOOL wmiGetNextResult();
BOOL wmiGetStrValue(LPCWSTR name, LPCWSTR* result);

#ifdef __cplusplus
}
#endif

#endif

