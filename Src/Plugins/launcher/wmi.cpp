/*
* A c wrapper for the weird COM-WMI interface.
*/

// TODO: use an extra thread for the COM calls

#define _WIN32_DCOM
#include <comdef.h>
#include <Wbemidl.h>

# pragma comment(lib, "wbemuuid.lib")

IWbemLocator *pLoc = NULL;
IWbemServices *pSvc = NULL;
IWbemClassObject *pclsObj = NULL;
IEnumWbemClassObject* pEnumerator = NULL;
VARIANT vtProp;
VARIANT* pvtProp = NULL;

extern "C" BOOL wmiInit()
{
    HRESULT hres;

    // Step 1: --------------------------------------------------
    // Initialize COM. ------------------------------------------

    hres =  CoInitializeEx(0, /*COINIT_MULTITHREADED*/  COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE); 
    if (FAILED(hres))
    {
        //cout << "Failed to initialize COM library. Error code = 0x" 
        //    << hex << hres << endl;
        return FALSE;                  // Program has failed.
    }

    // Step 2: --------------------------------------------------
    // Set general COM security levels --------------------------
    // Note: If you are using Windows 2000, you need to specify -
    // the default authentication credentials for a user by using
    // a SOLE_AUTHENTICATION_LIST structure in the pAuthList ----
    // parameter of CoInitializeSecurity ------------------------

    hres =  CoInitializeSecurity(
        NULL, 
        -1,                          // COM authentication
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities 
        NULL                         // Reserved
        );

                      
    if (FAILED(hres))
    {
        //cout << "Failed to initialize security. Error code = 0x" 
        //    << hex << hres << endl;
        CoUninitialize();
        return FALSE;                    // Program has failed.
    }
    
    // Step 3: ---------------------------------------------------
    // Obtain the initial locator to WMI -------------------------

    

    hres = CoCreateInstance(
        CLSID_WbemLocator,             
        0, 
        CLSCTX_INPROC_SERVER, 
        IID_IWbemLocator, (LPVOID *) &pLoc);
 
    if (FAILED(hres))
    {
        //cout << "Failed to create IWbemLocator object."
        //    << " Err code = 0x"
        //    << hex << hres << endl;
        CoUninitialize();
        return FALSE;                 // Program has failed.
    }

    // Step 4: -----------------------------------------------------
    // Connect to WMI through the IWbemLocator::ConnectServer method

    
  
    // Connect to the root\cimv2 namespace with
    // the current user and obtain pointer pSvc
    // to make IWbemServices calls.
    hres = pLoc->ConnectServer(
         _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
         NULL,                    // User name. NULL = current user
         NULL,                    // User password. NULL = current
         0,                       // Locale. NULL indicates current
         NULL,                    // Security flags.
         0,                       // Authority (e.g. Kerberos)
         0,                       // Context object 
         &pSvc                    // pointer to IWbemServices proxy
         );
    
    if (FAILED(hres))
    {
        //cout << "Could not connect. Error code = 0x" 
        //     << hex << hres << endl;
        pLoc->Release();     
        CoUninitialize();
        return FALSE;                // Program has failed.
    }

    //cout << "Connected to ROOT\\CIMV2 WMI namespace" << endl;


    // Step 5: --------------------------------------------------
    // Set security levels on the proxy -------------------------

    hres = CoSetProxyBlanket(
       pSvc,                        // Indicates the proxy to set
       RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
       RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
       NULL,                        // Server principal name 
       RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
       RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
       NULL,                        // client identity
       EOAC_NONE                    // proxy capabilities 
    );

    if (FAILED(hres))
    {
        //cout << "Could not set proxy blanket. Error code = 0x" 
        //    << hex << hres << endl;
        pSvc->Release();
        pLoc->Release();     
        CoUninitialize();
        return FALSE;               // Program has failed.
    }
    return TRUE;
}

extern "C" BOOL wmiRequest(LPCWSTR query)
{
  if(!pSvc)
    return FALSE;

  if(pclsObj)
  {
    pclsObj->Release();
    pclsObj = NULL;
  }
  if(pEnumerator)
  {
    pEnumerator->Release();
    pEnumerator = NULL;
  }

  HRESULT hres = pSvc->ExecQuery(
      bstr_t("WQL"), 
      bstr_t(query),
      WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
      NULL,
      &pEnumerator);
  
  if (FAILED(hres))
      return FALSE;

  return TRUE;
}

extern "C" BOOL wmiRequestFormat(LPCWSTR format, ...)
{
  WCHAR str[512];
  va_list ap;
  va_start (ap, format);
  vswprintf_s(str, format, ap);
  va_end (ap);
  return wmiRequest(str);
}

extern "C" BOOL wmiGetNextResult()
{
  if(!pEnumerator)
    return FALSE;

  if(pclsObj)
  {
    pclsObj->Release();
    pclsObj = NULL;
  }

  ULONG uReturn = 0;
  HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

  if(0 == uReturn)
    return FALSE;
  return TRUE;
}

extern "C" BOOL wmiGetStrValue(LPCWSTR name, LPCWSTR* result)
{
  if(!pclsObj)
    return FALSE;

  if(pvtProp)
  {
    VariantClear(pvtProp);
    pvtProp = NULL;
  }

  pvtProp = &vtProp;
  HRESULT hr = pclsObj->Get(name, 0, pvtProp, 0, 0);
  if (FAILED(hr))
  {
    pvtProp = NULL;
    return FALSE;
  }

  if(pvtProp->vt != VT_BSTR)
  {
    VariantClear(pvtProp);
    pvtProp = NULL;
    return FALSE;
  }

  *result = pvtProp->bstrVal;
  return TRUE;
}

extern "C" void wmiCleanup()
{
  if(!pSvc)
    return;

  if(pvtProp)
  {
    VariantClear(pvtProp);
    pvtProp = NULL;
  }

  if(pclsObj)
  {
    pclsObj->Release();
    pclsObj = NULL;
  }
  if(pEnumerator)
  {
    pEnumerator->Release();
    pEnumerator = NULL;
  }
  if(pLoc)
  {
    pLoc->Release();
    pLoc = NULL;
  }
  if(pSvc)
  {
    pSvc->Release();
    pSvc = NULL;
  }

  CoUninitialize();
}
