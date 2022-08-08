/**
  tLuaControl.cpp

  Some of this closely mirrors COleControl in wrapping.
*/

#include <ole2.h>
#include <oleidl.h>
#include <ocidl.h>

// for MINGW and/or Wine
#include <olectl.h>
#ifndef VIEWSTATUS_OPAQUE
#define VIEWSTATUS_OPAQUE 1
#endif
#ifndef POINTERINACTIVE_ACTIVATEONENTRY
#define POINTERINACTIVE_ACTIVATEONENTRY 1
#endif

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include <lauxlib.h>
}
#include "LuaCompat.h"

#include "luacom_internal.h"
#include "tLuaDispatch.h"
#include "tLuaControl.h"

static const OLEVERB verbs[] = {
  { OLEIVERB_SHOW,            0, 0, 0},
  { OLEIVERB_HIDE,            0, 0, 0},
  { OLEIVERB_INPLACEACTIVATE, 0, 0, 0},
  { OLEIVERB_PRIMARY,         0, 0, 0},
  { OLEIVERB_UIACTIVATE,      0, 0, 0}
};
//unused: OLEIVERB_PRIMARY, OLEIVERB_OPEN, OLEIVERB_DISCARDUNDOSTATE


#define ASSERT(fTest, szMsg) \
    if (!(fTest))  { DisplayAssert(szMsg, #fTest, __FILE__, __LINE__); }
#define FAIL(szMsg){ DisplayAssert(szMsg, "FAIL", __FILE__, __LINE__); }
#define CHECK_POINTER(val) if (!(val)) return E_POINTER
#define ADDREF_OBJECT(ptr) if (ptr) (ptr)->AddRef()

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
    const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
MIDL_DEFINE_GUID(CLSID, CLSID_Teste,0x687362C8,0x00D6,0x4eff,0x92,0x07,0xDD,0xB2,0x2E,0xE2,0x30,0x6D);


///
/// Local functions
///


/**
  Displays an assert message box (Abort, Retry: int 3, Ignore).
*/
static void DisplayAssert(LPCSTR pszMsg, LPCSTR pszAssert, LPCSTR pszFile, UINT line)
{
  // format message with file/line
  char szMsg[250];
  LPCSTR lpszText = pszMsg;
  if (pszFile) {
    if (!(pszMsg&&*pszMsg)) pszMsg = pszAssert;
    sprintf(szMsg, "%s\nFile %s, Line %d", pszMsg, pszFile, line);
    lpszText = szMsg;
  }

  int res = ::MessageBoxA(GetActiveWindow(), lpszText,
      "LuaCOM Assertion",
      MB_ICONHAND|MB_ABORTRETRYIGNORE|MB_SYSTEMMODAL);
  switch (res) {
  case IDABORT: ::FatalAppExitA(0, lpszText); break;
  case IDRETRY: ::DebugBreak(); break;  // win32 break (int 3)
  }
}

// Himetric based on http://spec.winprog.org/captcom/journal3/page2.html
static const float HIMETRIC_PER_PIXEL = 26.4583333333f;

static void PixelToHimetric(const SIZEL* pixelSize, SIZEL* himetricSize)
{
  himetricSize->cx = static_cast<long>(pixelSize->cx * HIMETRIC_PER_PIXEL + 0.5);
  himetricSize->cy = static_cast<long>(pixelSize->cy * HIMETRIC_PER_PIXEL + 0.5);
}

static void HimetricToPixel(const SIZEL* himetricSize, SIZEL* pixelSize)
{
  pixelSize->cx = static_cast<long>(himetricSize->cx / HIMETRIC_PER_PIXEL + 0.5);
  pixelSize->cy = static_cast<long>(himetricSize->cy / HIMETRIC_PER_PIXEL + 0.5);
}


/**
  Gets global parking window.
*/
static HWND GetParkingWindow()
{
  if (g_hwndParking)
    return g_hwndParking;

  // register window class
  WNDCLASSA wndclass;
  ZeroMemory(&wndclass, sizeof(wndclass));
  wndclass.lpfnWndProc   = DefWindowProc;
  wndclass.hInstance     = g_hInstance;
  wndclass.lpszClassName = "LuaCOM_Parking";
  if (! ::RegisterClassA(&wndclass)) {
    FAIL("Couldn't register parking window class!");
    return NULL;
  }

  // create window
  g_hwndParking = ::CreateWindowA("LuaCOM_Parking", NULL, WS_POPUP,
                                  0, 0, 0, 0, NULL, NULL, g_hInstance, NULL);
  ASSERT(g_hwndParking, "Couldn't create global parking window!");
  return g_hwndParking;
}

tLuaControl::tLuaControl(lua_State* L, ITypeInfo *pTypeinfo, int ref) :
  tLuaDispatch(L, pTypeinfo, ref),
  m_hRgn(NULL),
  m_hwnd(NULL),
  m_hwndParent(NULL),
  m_fDirty(false),
  m_fInPlaceActive(false),
  m_fInPlaceVisible(false),
  m_fUIActive(false)
{
  // Lua `cx,cy = registry[table_ref]:InitialSize()`
  lua_rawgeti(L, LUA_REGISTRYINDEX, table_ref);
  lua_pushstring(L, "InitialSize");
  lua_gettable(L, -2);
  lua_pushvalue(L, -2);
  lua_remove(L, -3);
  tStringBuffer err;
  if(!luaCompat_call(L, 1, 2, err)) {
    m_Size.cx = static_cast<LONG>(lua_tointeger(L,-2));
    m_Size.cy = static_cast<LONG>(lua_tointeger(L,-1));
  }
  else {
    m_Size.cx = 50;   // "certain hosts don't like 0,0 as your initial size"
    m_Size.cy = 20;
  }

  memset(&m_rcLocation, 0, sizeof(m_rcLocation));
}


///
/// tLuaControl 
///


tLuaControl::~tLuaControl()
{
  if (m_hwnd) this->DestroyWindow();
  if (m_hRgn) ::DeleteObject(m_hRgn);
}


void tLuaControl::DestroyWindow()
{
  // Lua `registry[table_ref]:DestroyWindow()`
  lua_rawgeti(L, LUA_REGISTRYINDEX, table_ref);
  lua_pushstring(L, "DestroyWindow");
  lua_gettable(L, -2);
  lua_pushvalue(L, -2);
  lua_remove(L, -3);
  luaCompat_call(L, 1, 0);
}


STDMETHODIMP tLuaControl::QueryInterface(REFIID riid, void** ppv)
{
  IUnknown* ret;
  if(::IsEqualIID(riid, IID_IUnknown)  ||
     ::IsEqualIID(riid, IID_IDispatch) ||
     ::IsEqualIID(riid, interface_iid))
  { ret = static_cast<IDispatch*>(this); }
  else if((::IsEqualIID(riid, IID_IProvideClassInfo) ||
            ::IsEqualIID(riid, IID_IProvideClassInfo2) )  && classinfo2)
  { ret = static_cast<IProvideClassInfo*>(classinfo2); }
  else if(::IsEqualIID(riid, IID_IConnectionPointContainer) && cpc)
    ret = cpc;
  else if(::IsEqualIID(riid, IID_ILuaDispatch))
    ret = static_cast<ILuaDispatch*>(this);
  else if(::IsEqualIID(riid, IID_IOleObject))
    ret = static_cast<IOleObject*>(this);
  else if(::IsEqualIID(riid, IID_IOleControl))
    ret = static_cast<IOleControl*>(this);
  else if(::IsEqualIID(riid, IID_IOleInPlaceObject))
    ret = static_cast<IOleInPlaceObject*>(this);
  else if(::IsEqualIID(riid, IID_IOleInPlaceActiveObject))
    ret = static_cast<IOleInPlaceActiveObject*>(this);
  else if(::IsEqualIID(riid, IID_IViewObject))
    ret = static_cast<IViewObject2*>(this);
  else if(::IsEqualIID(riid, IID_IViewObject2))
    ret = static_cast<IViewObject2*>(this);
  /* else if(::IsEqualIID(riid, IID_IQuickActivate))
    ret = static_cast<IQuickActivate*>(this); */ // [why disabled?]
  else if(::IsEqualIID(riid, IID_IPersistStreamInit))
    ret = static_cast<IPersistStreamInit*>(this);
  else if(::IsEqualIID(riid, IID_IPersistStorage))
    ret = static_cast<IPersistStorage*>(this);
  else
    ret = NULL;

  *ppv = ret;
  if (ret) {
    this->AddRef();
    return NOERROR;
  }
  else
    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) tLuaControl::AddRef()
{
  return ++m_refs;
}


STDMETHODIMP_(ULONG) tLuaControl::Release()
{
  assert(m_refs > 0);
  if(--m_refs == 0)
  {
    // unlock Lua table
    luaL_unref(L, LUA_REGISTRYINDEX, table_ref);

    // free libs
    while(num_methods--) {
      typeinfo->ReleaseFuncDesc(funcinfo[num_methods].funcdesc);
      delete funcinfo[num_methods].name;
    }

    delete funcinfo;
    typeinfo->Release(); typeinfo = NULL;
    delete typehandler;
    if(cpc) delete cpc;
    if(classinfo2) delete classinfo2;
    delete this; // destroy object
  }
  return m_refs;
}


/**
  IOleControl::GetControlInfo implementation.
*/
STDMETHODIMP tLuaControl::GetControlInfo(CONTROLINFO *pControlInfo)
{
  CHECK_POINTER(pControlInfo);
  ASSERT(pControlInfo->cb == sizeof(CONTROLINFO), "Host doesn't initialize CONTROLINFO structure");

  pControlInfo->hAccel = NULL;
  pControlInfo->cAccel = 0;

  return S_OK;
}


/**
  IOleControl::OnMnemonic implementation.
*/
STDMETHODIMP tLuaControl::OnMnemonic(LPMSG pMsg)
{
  return this->InPlaceActivate(true);
}


/**
  COleControl:OnAmbientPropertyChange    [IOleControl]
*/
STDMETHODIMP tLuaControl::OnAmbientPropertyChange(DISPID dispid)
{
  return S_OK;
}


/**
  IOleControl::FreezeEvents implementation.
*/
STDMETHODIMP tLuaControl::FreezeEvents(BOOL fFreeze)
{
  return S_OK;
}


/**
  IOleObject::SetClientSite implementation.
*/
STDMETHODIMP tLuaControl::SetClientSite(IOleClientSite* pcs)
{
  m_pControlSite.Release();

  m_pClientSite.Attach(pcs);
  if (m_pClientSite) m_pClientSite->AddRef();

  if (m_pClientSite) {
    m_pClientSite->QueryInterface(IID_IOleControlSite, reinterpret_cast<void **>(&m_pControlSite));
  }

  return S_OK;
}


/**
  IOleObject::GetClientSite implementation.
*/
STDMETHODIMP tLuaControl::GetClientSite(IOleClientSite **ppClientSite)
{
  CHECK_POINTER(ppClientSite);

  *ppClientSite = m_pClientSite;
  ADDREF_OBJECT(*ppClientSite);
  return S_OK;
}


/**
  IOleObject::SetHostNames implementation.
*/
STDMETHODIMP tLuaControl::SetHostNames(LPCOLESTR szContainerApp, LPCOLESTR szContainerObj) {
  return S_OK;
}


/**
  IOleObject::Close implementation.
*/
STDMETHODIMP tLuaControl::Close(DWORD dwSaveOption)
{
  if (m_fInPlaceActive) {
    HRESULT hr = this->InPlaceDeactivate();
    if (FAILED(hr)) return hr;
  }

  // handle save flag
  if ((dwSaveOption == OLECLOSE_SAVEIFDIRTY || dwSaveOption == OLECLOSE_PROMPTSAVE) && m_fDirty) {
    if (m_pOleAdviseHolder) m_pOleAdviseHolder->SendOnSave();
  }
  return S_OK;
}


/**
  IOleObject::SetMoniker implementation.
*/
STDMETHODIMP tLuaControl::SetMoniker(DWORD dwWitchMoniker, IMoniker* pmk)
{
  return E_NOTIMPL;
}


/**
  IOleObject::GetMoniker implementation.
*/
STDMETHODIMP tLuaControl::GetMoniker(DWORD dwAssign, DWORD dwWitchMoniker, IMoniker** ppMoniker)
{
  return E_NOTIMPL;
}


/**
  IOleObject::InitFromData implementation.
*/
STDMETHODIMP tLuaControl::InitFromData(IDataObject* pDataObject, BOOL fCreation, DWORD dwReserved)
{
  return E_NOTIMPL;
}


/**
  IOleObject::IsUpToDate implementation.
*/
STDMETHODIMP tLuaControl::IsUpToDate()
{
  return S_OK;
}


/**
  IOleObject::GetClipboardData implementation.
*/
STDMETHODIMP tLuaControl::GetClipboardData(DWORD dwReserved, IDataObject** ppDataObject)
{
  *ppDataObject = NULL;
  return E_NOTIMPL;
}


/**
  IOleObject::DoVerb implemenation
*/
STDMETHODIMP tLuaControl::DoVerb(
    LONG lVerb, LPMSG pMsg, IOleClientSite *pActiveSite,
    LONG lIndex, HWND hwndParent, LPCRECT prcPosRect)
{
  HRESULT hr;

  switch (lVerb) {
    case OLEIVERB_SHOW:
    case OLEIVERB_INPLACEACTIVATE:
    case OLEIVERB_UIACTIVATE:
      hr = this->InPlaceActivate(lVerb == OLEIVERB_UIACTIVATE);
      return hr;
      //FIX: handle cases for in-place activatable v.s. nonactivatable objects?
      // e.g. for OLEIVERB_UIACTIVATE, "If the object does not support in-place
      // activation, it should return E_NOTIMPL" (msdn)
    case OLEIVERB_HIDE:
      this->UIDeactivate();
      if (m_fInPlaceVisible) this->SetInPlaceVisible(false);
      return S_OK;
    default:
      if (lVerb > 0) { // derived control defined verb
        // treat unrecognized verb as OLEIVERB_PRIMARY (i.e. just activate in some way)
        hr = this->InPlaceActivate(false);
        return FAILED(hr) ? hr : OLEOBJ_S_INVALIDVERB;
      }
      else { // unrecognized negative verb
        return E_NOTIMPL;
      }
      break;
  }
}


/**
  IOleObject::EnumVerbs implementation.
*/
STDMETHODIMP tLuaControl::EnumVerbs(IEnumOLEVERB** ppEnumVerbs)
{
  *ppEnumVerbs = new CEnumOLEVERB(verbs, 5);
  return S_OK;
}


/**
  IOleObject::Update implementation.
*/
STDMETHODIMP tLuaControl::Update()
{
  return S_OK;
}


/**
  IOleObject::GetUserClassID implementation.
*/
STDMETHODIMP tLuaControl::GetUserClassID(CLSID* pClsid)
{
  if(!pClsid) return E_POINTER;
	
  // Lua `classID = registry[table_ref]:GetClass()`
  lua_rawgeti(L, LUA_REGISTRYINDEX, table_ref);
  lua_pushstring(L, "GetClass");
  lua_gettable(L, -2);
  lua_pushvalue(L, -2);
  lua_remove(L, -3);
  if(luaCompat_call(L, 1, 1)) return E_FAIL;

  tStringBuffer classID(lua_tostring(L, -1));

  return ::CLSIDFromString(tUtil::string2bstr(classID),pClsid);
}


/**
  IOleObject::GetUserType implementation.
*/
STDMETHODIMP tLuaControl::GetUserType(DWORD dwFormOfType, LPOLESTR* pszUserType)
{
  CLSID clsid;
  this->GetUserClassID(&clsid);
  return ::OleRegGetUserType(clsid, dwFormOfType, pszUserType);
}


/**
  IOleObject::SetExtent implementation.
*/
STDMETHODIMP tLuaControl::SetExtent(DWORD  dwDrawAspect, SIZEL *psizel)
{
  if (dwDrawAspect & DVASPECT_CONTENT) {
    // change the units to pixels, and resize the control.
    SIZEL sl;
    ::HimetricToPixel(psizel, &sl);

    // Lua `result = registry[table_ref]:SetExtent(sl.cx, sl.cy)`
    lua_rawgeti(L, LUA_REGISTRYINDEX, table_ref);
    lua_pushstring(L, "SetExtent");
    lua_gettable(L, -2);
    lua_pushvalue(L, -2);
    lua_remove(L, -3);
    lua_pushnumber(L, sl.cx);
    lua_pushnumber(L, sl.cy);
    if(luaCompat_call(L, 3, 1)) return E_FAIL;
    bool resize = lua_toboolean(L,-1);  // allow resize?

    if(resize)
      ::HimetricToPixel(psizel, &m_Size);

    // update HWND
    if (m_fInPlaceActive) {
      // "theoretically, one should not need to call OnPosRectChange
      // here, but there appear to be a few host related issues that
      // will make us keep it here.  we won't, however, both with
      // windowless ole controls, since they are all new hosts who
      // should know better"
      RECT rect;
      ::GetWindowRect(m_hwnd, &rect);
      ::MapWindowPoints(NULL, m_hwndParent, reinterpret_cast<POINT*>(&rect), 2);
      rect.right = rect.left + m_Size.cx;
      rect.bottom = rect.top + m_Size.cy;
      m_pInPlaceSite->OnPosRectChange(&rect);

      if (m_hwnd) {
          ::SetWindowPos(m_hwnd, m_hwndParent, 0, 0, m_Size.cx, m_Size.cy,
                          SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
      }
    }
    else if (m_hwnd) {
      ::SetWindowPos(m_hwnd, m_hwndParent, 0, 0, m_Size.cx, m_Size.cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
    return resize ? S_OK : E_FAIL;  // user accepted given size?
  }
  else {
    return DV_E_DVASPECT;  // other aspects unsupported
  }
}


/**
  IOleObject::GetExtent implementation.
*/
STDMETHODIMP tLuaControl::GetExtent(DWORD dwDrawAspect, SIZEL *pSizeLOut)
{
  if (dwDrawAspect & DVASPECT_CONTENT) {
    ::PixelToHimetric(&m_Size, pSizeLOut);
    return S_OK;
  }
  else {
    return DV_E_DVASPECT;
  }
}


/**
  IOleObject::Advise implementation.
*/
STDMETHODIMP tLuaControl::Advise(IAdviseSink *pAdviseSink, DWORD *pdwConnection)
{
  // create if not exist
  if (!m_pOleAdviseHolder) {
    HRESULT hr = ::CreateOleAdviseHolder(&m_pOleAdviseHolder);
    if (FAILED(hr)) return hr;
  }

  return m_pOleAdviseHolder->Advise(pAdviseSink, pdwConnection);
}


/**
  IOleObject::Unadvise implementation.
*/
STDMETHODIMP tLuaControl::Unadvise(DWORD dwConnection)
{
  if (!m_pOleAdviseHolder) {
    FAIL("Somebody called Unadvise on IOleObject without calling Advise!");
    return E_FAIL;
  }
  return m_pOleAdviseHolder->Unadvise(dwConnection);
}


/**
  IOleObject::EnumAdvise implementation.
*/
STDMETHODIMP tLuaControl::EnumAdvise(IEnumSTATDATA **ppEnumOut)
{
  if (!m_pOleAdviseHolder) {
    FAIL("Somebody Called EnumAdvise without setting up any connections!");
    *ppEnumOut = NULL;
    return E_FAIL;
  }
  return m_pOleAdviseHolder->EnumAdvise(ppEnumOut);
}


/** 
  IOleObject::GetMiscStatus implementation.
*/
STDMETHODIMP tLuaControl::GetMiscStatus(DWORD dwAspect, DWORD* pdwStatus)
{
  if(!pdwStatus) return E_POINTER;
  if(dwAspect != DVASPECT_CONTENT) return *pdwStatus = 0, E_FAIL;
  *pdwStatus = OLEMISC_RECOMPOSEONRESIZE | OLEMISC_INSIDEOUT
               | OLEMISC_ACTIVATEWHENVISIBLE | OLEMISC_ALWAYSRUN;
  return S_OK;
}


/**
  IOleObject::SetColorScheme implementation.
*/
STDMETHODIMP tLuaControl::SetColorScheme(LOGPALETTE* pLogpal)
{
  return E_NOTIMPL;
}


/**
  IOleWindow::GetWindow / IOleInPlaceObject::GetWindow implementation.
*/
STDMETHODIMP tLuaControl::GetWindow(HWND* phwnd)
{
  *phwnd = m_hwnd;
  return (*phwnd) ? S_OK : E_UNEXPECTED;
}


/**
  IOleWindow::ContextSensitiveHelp / IOleInPlaceObject::ContextSensitiveHelp implementation.
*/
STDMETHODIMP tLuaControl::ContextSensitiveHelp(BOOL fEnterMode)
{
  return E_NOTIMPL;
}


/**
  Activates (and sometimes UI activates) the control.

  Based on COleControl::InPlaceActivate or CComControlBase::InPlaceActivate.
*/
HRESULT tLuaControl::InPlaceActivate(bool lVerb)
{
  HRESULT hr;

  if (!m_pClientSite) // no client site to activate
    return S_OK;

  // obtain IOleInPlaceSite of IOleClientSite.
  if (! m_pInPlaceSite) {
    hr = m_pClientSite->QueryInterface(IID_IOleInPlaceSite, reinterpret_cast<void **>(&m_pInPlaceSite));
    if (FAILED(hr)) return hr;
  }

  // activate the IOleInPlaceSite
  if (!m_fInPlaceActive) {
    #define RETURN_ON_ERROR if(FAILED(hr)) { m_fInPlaceActive = false; return hr; }

    // OnInPlaceActivate() notifies container that a contained object is being activated in place.
    hr = m_pInPlaceSite->CanInPlaceActivate() == S_OK ? S_OK : E_FAIL;
    RETURN_ON_ERROR;
    m_fInPlaceActive = true;  // [simplify by moving to end of block?]
    m_pInPlaceSite->OnInPlaceActivate();
    RETURN_ON_ERROR;

    // obtain info on IOleInPlaceSite
    hr = m_pInPlaceSite->GetWindow(&m_hwndParent);
    RETURN_ON_ERROR;
    RECT rcPos, rcClip;
    OLEINPLACEFRAMEINFO frameinfo; frameinfo.cb = sizeof(OLEINPLACEFRAMEINFO);
    hr = m_pInPlaceSite->GetWindowContext(&m_pInPlaceFrame,
              &m_pInPlaceUIWindow, &rcPos, &rcClip, &frameinfo);
    RETURN_ON_ERROR;
    m_Size.cx = rcPos.right - rcPos.left;
    m_Size.cy = rcPos.bottom - rcPos.top;
    SetObjectRects(&rcPos, &rcClip);

    // create child window (and, if absent, parent window)
    hr = this->CreateInPlaceWindow(rcPos.left, rcPos.top, false) ? S_OK : E_FAIL;
    RETURN_ON_ERROR;

    #undef RETURN_ON_ERROR
  }

  if (!m_fInPlaceVisible)
    this->SetInPlaceVisible(true);

  // optionally activate UI elements (e.g. focus, menus, toolbars, and accelerators)
  if (lVerb && !m_fUIActive) {
    m_fUIActive = true;
    m_pInPlaceSite->OnUIActivate();

    ::SetFocus(m_hwnd);

    // set to this IOleInPlaceActiveObject 
    LPCOLESTR pszObjName = NULL; // no title
    m_pInPlaceFrame->SetActiveObject(this, pszObjName);
    if (m_pInPlaceUIWindow) m_pInPlaceUIWindow->SetActiveObject(this, pszObjName);

    // set no border space
    m_pInPlaceFrame->SetBorderSpace(NULL);
    if (m_pInPlaceUIWindow) m_pInPlaceUIWindow->SetBorderSpace(NULL);
  }

  return S_OK;
}


/**
  IOleInPlaceObject::InPlaceDeactivate implementation.
*/
STDMETHODIMP tLuaControl::InPlaceDeactivate()
{
  // nothing to do
  if (!m_fInPlaceActive)
    return S_OK;

  // transition from UIActive
  if (m_fUIActive)
    this->UIDeactivate();

  m_fInPlaceActive = false;
  m_fInPlaceVisible = false;

  // destroy any window
  if (m_hwnd) {
    this->DestroyWindow();
    m_hwnd = NULL;
  }

  m_pInPlaceFrame.Release();
  m_pInPlaceUIWindow.Release();
  m_pInPlaceSite->OnInPlaceDeactivate();

  return S_OK;
}


/**
  IOleInPlaceObject::UIDeactivate implementation.
*/
STDMETHODIMP tLuaControl::UIDeactivate()
{
  // nothing to do
  if (!m_fUIActive)
    return S_OK;

  m_fUIActive = false;
  
  // notify frame windows
  if (m_pInPlaceUIWindow)
    m_pInPlaceUIWindow->SetActiveObject(NULL, NULL);
  m_pInPlaceFrame->SetActiveObject(NULL, NULL);

  m_pInPlaceSite->OnUIDeactivate(FALSE);

  return S_OK;
}


/**
  IOleInPlaceObject::SetObjectRects implementation.
  based on COleControl::SetObjectRects.
*/
STDMETHODIMP tLuaControl::SetObjectRects(LPCRECT prcPos, LPCRECT prcClip)
{
  // move our window to the new location and handle clipping.
  if (m_hwnd) {
    // update clip region
    HRGN newRgn = NULL;
    if (prcClip) { // define new region
      RECT rcIntersect;
      if ( ::IntersectRect(&rcIntersect, prcPos, prcClip) && ! ::EqualRect(&rcIntersect, prcPos)) {
        ::OffsetRect(&rcIntersect, -(prcPos->left), -(prcPos->top));
        newRgn = ::CreateRectRgnIndirect(&rcIntersect);
      }
    }
    if (m_hRgn || newRgn) { // apply region
      ::SetWindowRgn(m_hwnd, newRgn, TRUE);
      if (m_hRgn != NULL) DeleteObject(m_hRgn);
      m_hRgn = newRgn;
    }

    // set location
    int cx = prcPos->right - prcPos->left;
    int cy = prcPos->bottom - prcPos->top;
    ::SetWindowPos(m_hwnd, m_hwndParent, prcPos->left, prcPos->top, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE);
  }

  // save location
  m_rcLocation = *prcPos;

  return S_OK;
}


/**
  IOleInPlaceObject::ReactivateAndUndo implementation.
*/
STDMETHODIMP tLuaControl::ReactivateAndUndo()
{
  return E_NOTIMPL;
}


/**
  IOleInPlaceActiveObject::TranslateAccelerator implementation.
*/
STDMETHODIMP tLuaControl::TranslateAccelerator(LPMSG pmsg)
{
  return S_FALSE;
}


/**
  IOleInPlaceActiveObject::OnFrameWindowActivate implementation.
*/
STDMETHODIMP tLuaControl::OnFrameWindowActivate(BOOL fActivate)
{
  return this->InPlaceActivate(true);
}


/**
  IOleInPlaceActiveObject::OnDocWindowActivate implementation.
*/
STDMETHODIMP tLuaControl::OnDocWindowActivate(BOOL fActivate)
{
  return this->InPlaceActivate(true);
}


/**
  IOleInPlaceActiveObject::ResizeBorder implementation.
*/
STDMETHODIMP tLuaControl::ResizeBorder(
    LPCRECT prcBorder, IOleInPlaceUIWindow *pUIWindow, BOOL fFrameWindow)
{
  return S_OK;
}


/**
  IOleInPlaceActiveObject::EnableModeless implementation.
*/
STDMETHODIMP tLuaControl::EnableModeless(BOOL fEnable)
{
  return S_OK;
}


/**
  IViewObject2::Draw implementation.
*/
STDMETHODIMP tLuaControl::Draw(
    DWORD dwDrawAspect, LONG lIndex, void *pvAspect,
    DVTARGETDEVICE *ptd, HDC hicTargetDevice, HDC hdcDraw, LPCRECTL prcBounds,
    LPCRECTL prcWBounds, BOOL (__stdcall *pfnContinue)(ULONG_PTR dwContinue), ULONG_PTR dwContinue)
{
  if (dwDrawAspect != DVASPECT_CONTENT) // unsupported
    return DV_E_DVASPECT;
  if (hicTargetDevice)
    return E_NOTIMPL;
  if (!m_hwnd)
    return E_FAIL;

  // get window rect
  RECT rcClient;
  ::GetClientRect(m_hwnd, &rcClient);

  // set up the DC for painting
  if ((rcClient.right - rcClient.left != prcBounds->right - prcBounds->left)
      && (rcClient.bottom - rcClient.top != prcBounds->bottom - prcBounds->top))
  {
    int iMapMode = ::SetMapMode(hdcDraw, MM_ANISOTROPIC);
    SIZE sWOrg, sVOrg;
    ::SetWindowExtEx(hdcDraw, rcClient.right, rcClient.bottom, &sWOrg);
    ::SetViewportExtEx(hdcDraw, prcBounds->right - prcBounds->left, prcBounds->bottom - prcBounds->top, &sVOrg);
  }
  POINT ptWOrg, ptVOrg;
  ::SetWindowOrgEx(hdcDraw, 0, 0, &ptWOrg);
  ::SetViewportOrgEx(hdcDraw, prcBounds->left, prcBounds->top, &ptVOrg);

  // paint (WM_PAINT)
  WNDPROC wndProc       = (WNDPROC)::GetWindowLongPtr(m_hwnd, GWLP_WNDPROC);
  if(!wndProc) wndProc = (WNDPROC)::GetWindowLongPtr(m_hwnd, DWLP_DLGPROC);
  ::CallWindowProc(wndProc, m_hwnd, WM_PAINT, (WPARAM)hdcDraw, 0);

  return S_OK;
}


/**
  IViewObject2::GetColorSet implementation.
*/
STDMETHODIMP tLuaControl::GetColorSet(DWORD dwDrawAspect, LONG lindex,
    void *IgnoreMe, DVTARGETDEVICE *ptd, HDC hicTargetDevice, LOGPALETTE **ppColorSet)
{
  if (dwDrawAspect != DVASPECT_CONTENT)
    return DV_E_DVASPECT;

  return E_NOTIMPL;
}


/**
  IViewObject2::Freeze implementation.
*/
STDMETHODIMP tLuaControl::Freeze(DWORD dwDrawAspect, LONG lIndex, void *IgnoreMe, DWORD *pdwFreeze)
{
  return E_NOTIMPL;
}


/**
  IVewObject2::Unfreeze implementation.
*/
STDMETHODIMP tLuaControl::Unfreeze(DWORD dwFreeze)
{
  return E_NOTIMPL;
}


/**
  IViewObject2::SetAdvise implementation.
*/
STDMETHODIMP tLuaControl::SetAdvise(DWORD dwAspects, DWORD dwAdviseFlags, IAdviseSink *pAdviseSink)
{
  // if it's not a content aspect, we don't support it.
  if (!(dwAspects & DVASPECT_CONTENT)) {
    return DV_E_DVASPECT;
  }

  // store flags needed by GetAdvise()
  m_fViewAdvisePrimeFirst = (dwAdviseFlags & ADVF_PRIMEFIRST) != 0;
  m_fViewAdviseOnlyOnce   = (dwAdviseFlags & ADVF_ONLYONCE) != 0;

  m_pViewAdviseSink.Attach(pAdviseSink);
  ADDREF_OBJECT(m_pViewAdviseSink);

  return S_OK;
}


/**
  IViewObject2::GetAdvise implementation.
*/
STDMETHODIMP tLuaControl::GetAdvise(DWORD *pdwAspects, DWORD *pdwAdviseFlags, IAdviseSink **ppAdviseSink)
{
  if (pdwAspects)
    *pdwAspects = DVASPECT_CONTENT;

  if (pdwAdviseFlags) {
    *pdwAdviseFlags = 0;
    if (m_fViewAdviseOnlyOnce) *pdwAdviseFlags |= ADVF_ONLYONCE;
    if (m_fViewAdvisePrimeFirst) *pdwAdviseFlags |= ADVF_PRIMEFIRST;
  }

  if (ppAdviseSink) {
    *ppAdviseSink = m_pViewAdviseSink;
    ADDREF_OBJECT(*ppAdviseSink);
  }

  return S_OK;
}


/**
  IViewObject2::GetExtent implementation.
*/
STDMETHODIMP tLuaControl::GetExtent(DWORD dwDrawAspect, LONG lindex, DVTARGETDEVICE *ptd, LPSIZEL psizel)
{
  return this->GetExtent(dwDrawAspect, psizel);
}


/**
  helper function.
  Sets visibility of the control (child) window.
*/
void tLuaControl::SetInPlaceVisible(bool show)
{
  m_fInPlaceVisible = show;
  if (m_hwnd) {
    BOOL visible = ((::GetWindowLong(m_hwnd, GWL_STYLE) & WS_VISIBLE) != 0);
    if (!!visible != !!show) // changed
        ::ShowWindow(m_hwnd, show ? SW_SHOWNA : SW_HIDE);
  }
}
// note: IsWindowVisible() returns TRUE only if WS_VISIBLE for self and all ancestors


/**
  Creates the window to use.
  Created at at top-left point (x,y).  fNoRedraw to skip redrawing.  Returns HWND.
*/
HWND tLuaControl::CreateInPlaceWindow(int x, int y, bool fNoRedraw)
{
  // already exists
  if (m_hwnd)
    return m_hwnd;

  if (!m_hwndParent)
    m_hwndParent = ::GetParkingWindow();

  BOOL fVisible = ::IsWindowVisible(m_hwndParent);

  // Lua `m_hwnd = registry[table_ref]:CreateWindow(m_hwndParent, x, y, m_Size.cx, m_Size.cy)`.
  lua_rawgeti(L, LUA_REGISTRYINDEX, table_ref);
  lua_pushstring(L, "CreateWindow");
  lua_gettable(L, -2);
  lua_pushvalue(L, -2);
  lua_remove(L, -3);
  lua_pushlightuserdata(L, m_hwndParent);
  lua_pushnumber(L, x);
  lua_pushnumber(L, y);
  lua_pushnumber(L, m_Size.cx);
  lua_pushnumber(L, m_Size.cy);
  tStringBuffer err;
  if(luaCompat_call(L, 6, 1, err) != 0)
     return NULL;
  m_hwnd = reinterpret_cast<HWND>(luaCompat_getPointer(L, -1));
  lua_pop(L, 1);

  if (m_hwnd) {
    if (fVisible) {
      // show window
      ::SetWindowPos(m_hwnd, m_hwndParent, x, y, m_Size.cx, m_Size.cy,
           SWP_NOZORDER | SWP_SHOWWINDOW | (fNoRedraw ? SWP_NOREDRAW : 0));
    }
  }

  // notify host
  if (m_pClientSite)
    m_pClientSite->ShowObject();

  return m_hwnd;
}


/**
  IPersistStreamInit::GetClassID implementation.
*/
STDMETHODIMP tLuaControl::GetClassID(CLSID *pclsid)
{
  return this->GetUserClassID(pclsid);
}


/**
  IPersistStreamInit::IsDirty implementation.
*/
STDMETHODIMP tLuaControl::IsDirty()
{
  return S_FALSE;
}


/**
  IPersistStreamInit::InitNew implementation.
*/
STDMETHODIMP tLuaControl::InitNew()
{
  return S_OK;
}


/**
  IPersistStreamInit::GetSizeMax implementation.
*/
STDMETHODIMP tLuaControl::GetSizeMax(ULARGE_INTEGER *pulMaxSize)
{
  return E_NOTIMPL;
}


/**
  IPersistStreamInit::Load implementation.
*/
STDMETHODIMP tLuaControl::Load(IStream *pStream)
{
  return S_OK;
}


/**
  IPersistStreamInit::Save implementation.
*/
STDMETHODIMP tLuaControl::Save(IStream *pStream, BOOL fClearDirty)
{
  return S_OK;
}


/**
  IPersistStorage::InitNew implementation.
*/
STDMETHODIMP tLuaControl::InitNew(IStorage *pStorage)
{
  return InitNew();
}


/**
  IPersistStorage::Load implementation.
*/
STDMETHODIMP tLuaControl::Load(IStorage *pStorage)
{
  return S_OK;
}


/**
  IPersistStorage::Save implementation.
*/
STDMETHODIMP tLuaControl::Save(IStorage *pStorage, BOOL fSameAsLoad)
{
  return S_OK;
}


/**
  IPersistStorage::SaveCompleted implementation.
*/
STDMETHODIMP tLuaControl::SaveCompleted(IStorage *pStorageNew)
{
  return S_OK;
}


/**
  IPersistStorage::HandsOffStorage implementation.
*/
STDMETHODIMP tLuaControl::HandsOffStorage()
{
  return S_OK;  // storage unused
}


/**
  IQuickActivate::QuickActivate implementation.
*/
STDMETHODIMP tLuaControl::QuickActivate(QACONTAINER *pContainer, QACONTROL *pControl)
{
  HRESULT hr;

  // check preconditions (for safety)
  if (!pContainer) return E_UNEXPECTED;
  if (!pControl)   return E_UNEXPECTED;
  if (pContainer->cbSize < sizeof(QACONTAINER)) return E_UNEXPECTED;
  if (pControl->cbSize   < sizeof(QACONTROL))   return E_UNEXPECTED;

  // save client site
  if (pContainer->pClientSite) {
    hr = this->SetClientSite(pContainer->pClientSite);
    if (FAILED(hr)) return hr;
  }

  // event sink
  if (pContainer->pUnkEventSink) {
    tLuaCOMConnPointContainer* cpc = GetConnPointContainer();
    IConnectionPoint *connection_point;
    hr = cpc->FindConnectionPoint(IID_IUnknown, &connection_point);
    if (SUCCEEDED(hr)) hr = connection_point->Advise(pContainer->pUnkEventSink, &pControl->dwEventCookie);
    if (FAILED(hr)) {
      pContainer->pUnkEventSink->Release();
      return hr;
    }
  }

  // advise sink
  if (pContainer->pAdviseSink) {
    DWORD dw;
    hr = this->Advise(reinterpret_cast<IAdviseSink*>(pContainer->pAdviseSink), &dw);
    if (FAILED(hr)) return hr;
  }

  // update QACONTROL
  pControl->dwMiscStatus =
      OLEMISC_RECOMPOSEONRESIZE | OLEMISC_INSIDEOUT | OLEMISC_ACTIVATEWHENVISIBLE | OLEMISC_ALWAYSRUN;
  pControl->dwViewStatus = VIEWSTATUS_OPAQUE;
  pControl->dwPointerActivationPolicy = POINTERINACTIVE_ACTIVATEONENTRY;

  return S_OK;
}


/**
  IQuickActivate::SetContentExtent implementation.
*/
STDMETHODIMP tLuaControl::SetContentExtent(LPSIZEL pSize)
{
  return this->SetExtent(DVASPECT_CONTENT, pSize);
}


/**
  IQuickActivate::GetContentExtent implementation.
*/
STDMETHODIMP tLuaControl::GetContentExtent(LPSIZEL pSize)
{
  return this->GetExtent(DVASPECT_CONTENT, pSize);
}


tLuaControl *tLuaControl::CreateLuaControl(
    lua_State* L, ITypeInfo* interface_typeinfo, int ref)
{
  tLuaControl *pcont = new tLuaControl(L, interface_typeinfo, ref);

  // Lua `registry[ref][idxDispatch] = pcont`
  lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
  lua_pushlightuserdata(L, idxDispatch);
  lua_pushlightuserdata(L, pcont);
  lua_rawset(L,-3);
  lua_pop(L, 1);

  pcont->AddRef();

  return pcont;
}
