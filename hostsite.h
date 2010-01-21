// Site which hosts the VBscript engine


#ifndef _HOSTSITE_H
#define _HOSTSITE_H

// {11DFC5E6-AD6F-11D0-8EAE-00A0247B3BFD}
static const CLSID clsidMUSHclient =
{ 0x11dfc5e6, 0xad6f, 0x11d0, { 0x8e, 0xae, 0x0, 0xa0, 0x24, 0x7b, 0x3b, 0xfd } };

HRESULT LoadTypeInfoFromThisModule(REFIID riid, ITypeInfo **ppti);

class CActiveScriptSite :   public IActiveScriptSite,
                            public IActiveScriptSiteWindow {
    LONG m_cRef;
    IDispatch * m_pDispWorld;
    CMUSHclientDoc * m_pDoc;

public:
    CActiveScriptSite(IDispatch * pDispWorld, CMUSHclientDoc * pDoc) 
      : m_cRef(0), m_pDispWorld (pDispWorld), m_pDoc (pDoc)  
          { }

    ~CActiveScriptSite(void) 
      { 
      if (m_pDispWorld) m_pDispWorld->Release (); 
      }

// IUnknown methods
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv) 
      {
      TRACE ("CActiveScriptSite: QueryInterface\n");
      if (riid == IID_IUnknown||riid == IID_IActiveScriptSite)
        *ppv = (IActiveScriptSite*)this;
      else if (riid == IID_IActiveScriptSiteWindow)
        *ppv = (IActiveScriptSiteWindow*)this;
      else 
        return (*ppv = 0), E_OUTOFMEMORY;
      ((IUnknown*)*ppv)->AddRef();
      return S_OK;
      }
    
    STDMETHODIMP_(ULONG) AddRef()
      { 
      TRACE ("CActiveScriptSite: AddRef\n");
      return InterlockedIncrement(&m_cRef); 
      }
    
    STDMETHODIMP_(ULONG) Release() 
      {
      TRACE ("CActiveScriptSite: Release\n");
      if (InterlockedDecrement(&m_cRef))
        return m_cRef;
      delete this;
      return 0;
      }
    
    // IActiveScriptSite methods
    STDMETHODIMP GetItemInfo(LPCOLESTR pstrName, DWORD dwReturnMask,
      IUnknown **ppiunkItem, ITypeInfo **ppti) 
      {
      HRESULT hr = E_FAIL;
      if (dwReturnMask & SCRIPTINFO_IUNKNOWN)
        *ppiunkItem = 0;
      if (dwReturnMask & SCRIPTINFO_ITYPEINFO)
        *ppti = 0;
      // see if top-level name is "world"
      if (wcscmp(pstrName, OLESTR("world")) == 0) 
        {
        if (dwReturnMask & SCRIPTINFO_IUNKNOWN)
          if (*ppiunkItem = m_pDispWorld)
            {
            TRACE ("CActiveScriptSite: GetItemInfo: world type requested\n");
            (*ppiunkItem)->AddRef();
            hr = S_OK;
            }
          
          if (dwReturnMask & SCRIPTINFO_ITYPEINFO)
            {
            TRACE ("CActiveScriptSite: GetItemInfo: dwReturnMask & SCRIPTINFO_ITYPEINFO\n");
            hr = LoadTypeInfoFromThisModule(clsidMUSHclient, ppti);
            }
        }
      return hr;
      }
    
    STDMETHODIMP OnScriptError(IActiveScriptError *pscripterror);
    
    STDMETHODIMP GetLCID(LCID *plcid)
      { 
      TRACE ("CActiveScriptSite: GetLCID\n");
      *plcid = 9; 
      return S_OK; 
      }
    
    STDMETHODIMP GetDocVersionString(BSTR *pbstrVersion) 
      { 
      TRACE ("CActiveScriptSite: GetDocVersionString\n");
      *pbstrVersion = SysAllocString(L""); 
      return S_OK; 
      }
    
    STDMETHODIMP OnScriptTerminate(const VARIANT *pvr, const EXCEPINFO *pei)
      { 
      TRACE ("CActiveScriptSite: OnScriptTerminate\n");
      return S_OK; 
      }
    
    STDMETHODIMP OnStateChange(SCRIPTSTATE ssScriptState)
      { 
      TRACE1 ("CActiveScriptSite: OnStateChange: %i\n", ssScriptState);
      return S_OK; 
      }
    
    STDMETHODIMP OnEnterScript(void)
      { 
      TRACE ("CActiveScriptSite: OnEnterScript\n");
      return S_OK; 
      }
    
    STDMETHODIMP OnLeaveScript(void) 
      { 
      TRACE ("CActiveScriptSite: OnLeaveScript\n");
      return S_OK; 
      }
    
    // IActiveScriptSiteWindow methods    
    STDMETHODIMP GetWindow(HWND *phwnd)
      { 
      TRACE ("CActiveScriptSite: GetWindow\n");
      *phwnd = GetDesktopWindow(); 
      return S_OK; 
      }
    
    STDMETHODIMP EnableModeless(BOOL)
      { 
      TRACE ("CActiveScriptSite: EnableModeless\n");
      return S_OK; 
      }
};

#endif
