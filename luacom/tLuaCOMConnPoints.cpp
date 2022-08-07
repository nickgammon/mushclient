/**
  tLuaCOMConnPoints.cpp - COM connection points.
  
  WARNING: This code has been significantly refactored but largely untested
  because I don't use this feature, I don't know who does, and test cases are lacking.
  There were bugs before refactoring and likely bugs added after refactoring.
  Please thoroughly review this code it you are using connection points.
  
  Some of this mirrors the MS COM tutorial code sample (connect.cpp).
*/

#include <windows.h>
#include <ole2.h>
#include <olectl.h>
#include <ocidl.h>
#include <algorithm>

#include "tLuaCOM.h"
#include "tLuaCOMConnPoints.h"
#include "tCOMUtil.h"
#include "tLuaCOMException.h"
#include "LuaAux.h"

#include "luacom_internal.h"
#include "LuaCompat.h"

static const char * CONNPOINT_NAME = "__LUACOM_CONNPOINT";


// first connection point cookie value
enum { COOKIE_START_VALUE = 400 };


///
/// tLuaCOMEnumConnPoints
///



/**
  Constructor.
  pHost - object to enumerate connection points for
*/
tLuaCOMEnumConnPoints::tLuaCOMEnumConnPoints(IUnknown* pHost) :
  mRefCount(0),
  mpHost(pHost),
  mPoints(),
  mNextIndex(0)
{
}


/**
  Destructor.
*/
tLuaCOMEnumConnPoints::~tLuaCOMEnumConnPoints()
{
}


/**
  Initialization.
*/
HRESULT tLuaCOMEnumConnPoints::Init(
    const std::vector<tCOMPtr<IConnectionPoint> > & points, ULONG nextIndex)
{
  mPoints.assign(mPoints.begin(), mPoints.end());
  mNextIndex = nextIndex;
  return S_OK;
}


/**
  IUnknown::QueryInterface implementation.
*/
STDMETHODIMP tLuaCOMEnumConnPoints::QueryInterface(REFIID riid, void ** ppv)
{
  *ppv = (riid == IID_IUnknown || riid == IID_IEnumConnectionPoints) ?
         static_cast<void*>(this) : NULL;
  if (! *ppv)
    return E_NOINTERFACE;
  reinterpret_cast<IUnknown*>(*ppv)->AddRef();
  return S_OK;
}


/**
  IUnknown::AddRef implementation.
*/
STDMETHODIMP_(ULONG) tLuaCOMEnumConnPoints::AddRef()
{
  mpHost->AddRef();
  return ++mRefCount;
}


/**
  IUnknown::Release implementation.
*/
STDMETHODIMP_(ULONG) tLuaCOMEnumConnPoints::Release()
{
  mpHost->Release();
  if (--mRefCount == 0)
  {
    mRefCount++;
    delete this;
  }
  return mRefCount;
}


/**
  IEnumConnectionPoints::Next implementation.
*/
STDMETHODIMP tLuaCOMEnumConnPoints::Next(
    ULONG cReq, IConnectionPoint** paConnPts, ULONG* pcEnumerated)
{
  // check args
  if (! paConnPts)
    return E_POINTER;
  if (!(mNextIndex < mPoints.size()))
    return S_FALSE;
  if (!pcEnumerated && cReq != 1)
    return E_POINTER;

  // copy
  size_t i = 0;
  for (; i < cReq && mNextIndex < mPoints.size(); i++, mNextIndex++)
  {
    paConnPts[i] = mPoints[mNextIndex];
    if (paConnPts[i]) paConnPts[i]->AddRef();
  }
  if (pcEnumerated)
    *pcEnumerated = i;

  return S_OK;
}


/**
  IEnumConnectionPoints::Skip implementation.
*/
STDMETHODIMP tLuaCOMEnumConnPoints::Skip(ULONG cSkip)
{
  if (!(mNextIndex + cSkip < mPoints.size()))
    return S_FALSE;
  mNextIndex += cSkip;
  return S_OK;
}


/**
  IEnumConnectionPoints::Reset implementation.
*/
STDMETHODIMP tLuaCOMEnumConnPoints::Reset()
{
  mNextIndex = 0;
  return S_OK;
}


/**
  IEnumConnectionPoints::Clone implementation.
*/
STDMETHODIMP tLuaCOMEnumConnPoints::Clone(IEnumConnectionPoints** ppIEnum)
{
  HRESULT hr;
  
  *ppIEnum = NULL;
  tLuaCOMEnumConnPoints* penumcp = new tLuaCOMEnumConnPoints(mpHost); // E_OUTOFMEMORY
  hr = penumcp->Init(mPoints, mNextIndex);
  if (SUCCEEDED(hr))
    hr = penumcp->QueryInterface(
          IID_IEnumConnectionPoints, reinterpret_cast<void **>(ppIEnum));

  return hr;
}



///
/// tLuaCOMConnPoint
///



/**
  Constructor.
*/
tLuaCOMConnPoint::tLuaCOMConnPoint(lua_State *p_L, IUnknown* pHost) :
  L(p_L),
  mRefCount(0),
  mpHost(pHost),
  mNextCookie(COOKIE_START_VALUE),
  mConnections(), mCookies()
{
  // creates a new lua tag associated with this connection point
  luaCompat_pushTypeByName(L, MODULENAME, LCOM_CONNPOINT_TYPENAME);
  lua_pushcfunction(L, l_tagmeth_index);
  lua_setfield(L, -2, "__index");
  lua_pop(L, 1);
}


tLuaCOMConnPoint::~tLuaCOMConnPoint()
{
  for (size_t i=0; i<mSinks.size(); i++)
  {
    if (mSinks[i]) mSinks[i]->Unlock();
  }
}


HRESULT tLuaCOMConnPoint::Init(REFIID rIIDSink, ITypeInfo *pTypeinfo)
{
  mIIDSink = rIIDSink;
  mpTypeinfo = pTypeinfo;
  mConnections.clear();
  mCookies.clear();
  return S_OK;
}



STDMETHODIMP tLuaCOMConnPoint::QueryInterface(REFIID riid, void ** ppv)
{
  HRESULT hr = E_NOINTERFACE;

  *ppv = NULL;
  if (IID_IUnknown == riid || IID_IConnectionPoint == riid)
    *ppv = static_cast<void*>(this);
  if (*ppv)
  {
    reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    hr = S_OK;
  }

  return hr;
}


STDMETHODIMP_(ULONG) tLuaCOMConnPoint::AddRef()
{
  return ++mRefCount;
}


STDMETHODIMP_(ULONG) tLuaCOMConnPoint::Release()
{
  if (--mRefCount == 0)
  {
    mRefCount++;
    delete this;
  }
  return mRefCount;
}


/**
  IConnectionPoint::GetConnectionInterface implementation.
*/
STDMETHODIMP tLuaCOMConnPoint::GetConnectionInterface(IID* pIIDSink)
{
  if (!pIIDSink)
    return E_POINTER;
  *pIIDSink = mIIDSink;
  return S_OK;
}


/**
  IConnectionPoint::GetConnectionPointContainer implementation.
*/
STDMETHODIMP tLuaCOMConnPoint::GetConnectionPointContainer(
    IConnectionPointContainer** ppConnPtCon)
{
  HRESULT hr = mpHost->QueryInterface(
      IID_IConnectionPointContainer, reinterpret_cast<void **>(ppConnPtCon));
  return hr;
}


/**
  IConnectionPoint::Advise implementation.
*/
STDMETHODIMP tLuaCOMConnPoint::Advise(IUnknown* pUnkSink, DWORD* pdwCookie)
{
  HRESULT hr;

  *pdwCookie = 0;

  tCOMPtr<IDispatch> psink;
  hr = pUnkSink->QueryInterface(IID_IDispatch, reinterpret_cast<void **>(&psink));
  if (FAILED(hr))
     return CONNECT_E_CANNOTCONNECT;

  // find or create free location
  std::vector<DWORD>::iterator free = std::find(mCookies.begin(), mCookies.end(), 0);
  if (free == mCookies.end())
    {
    // create free location
    mConnections.push_back(tCOMPtr<IUnknown>());
    mCookies.push_back(0);
    mSinks.push_back(NULL);
    free = mCookies.end() - 1;
    }
  size_t ifree = free - mCookies.begin();

  // insert
  mConnections[ifree] = tCOMPtr<IUnknown>(psink.Raw());
  mCookies[ifree] = mNextCookie;

  // VB supplies a very weird type info, so we stay with ours,
  // as it's where VB will look for the DISPID's anyway
  try
  { mSinks[ifree] = tLuaCOM::CreateLuaCOM(L, psink, IID_NULL, mpTypeinfo); }
  catch(class tLuaCOMException&)
  { return CONNECT_E_CANNOTCONNECT; }

  *pdwCookie = mNextCookie;
  mNextCookie++;

  return S_OK;
}


/**
  IConnectionPoint::Unadvise implementation.
*/
STDMETHODIMP tLuaCOMConnPoint::Unadvise(DWORD dwCookie)
{
  if (dwCookie == 0)
    return E_UNEXPECTED;

  // find location
  std::vector<DWORD>::iterator it
      = std::find(mCookies.begin(), mCookies.end(), dwCookie);
  if (it == mCookies.end())
    return S_OK;  // [or E_POINTER?] not found
  size_t pos = it - mCookies.begin();

  // remove it
  mConnections[pos].Release();  
  mCookies[pos] = 0;
  mSinks[pos]->Unlock(); mSinks[pos] = NULL;

  return S_OK;
}


/**
  IConnectionPoint::EnumConnections implementation.
*/
STDMETHODIMP tLuaCOMConnPoint::EnumConnections(IEnumConnections** ppIEnum)
{
  HRESULT hr;
  *ppIEnum = NULL;

  // copy only non-empty elements
  std::vector<tCOMPtr<IUnknown> > connections;
  std::vector<DWORD> cookies;
  for (size_t i=0, j=0; i < mConnections.size(); i++)
  {
    if (mCookies[i] != 0)
    {
      connections.push_back(mConnections[i]);
      cookies.push_back(mCookies[i]);
      j++;
    }
  }

  // create IEnumConnections.
  tLuaCOMEnumConnections* penumc = new tLuaCOMEnumConnections(this); // E_OUTOFMEMORY
  hr = penumc->Init(connections, cookies, 0);
  if (SUCCEEDED(hr))
    hr = penumc->QueryInterface(
          IID_IEnumConnections, reinterpret_cast<void **>(ppIEnum));

  return hr;
}



///
/// tLuaCOMEnumConnections
///



/**
  Constructor.
  pHost - object to be enumerated
*/
tLuaCOMEnumConnections::tLuaCOMEnumConnections(IUnknown* pHost) :
  mRefCount(0), mpHost(pHost),
  mConnections(), mCookies(),
  mNextIndex(0)
{
}


/**
  Destructor.
*/
tLuaCOMEnumConnections::~tLuaCOMEnumConnections()
{
}


/**
  Initialization.
*/
HRESULT tLuaCOMEnumConnections::Init(
    std::vector<tCOMPtr<IUnknown> > & connections,
    std::vector<DWORD> & cookies, ULONG nextIndex)
{
  mConnections = connections;   // E_OUTOFMEMORY
  mCookies = cookies;
  mNextIndex = nextIndex;
  return S_OK;
}


/**
  IUnknown::QueryInterface implementation.
*/
STDMETHODIMP tLuaCOMEnumConnections::QueryInterface(
    REFIID riid, void ** ppv)
{
  if (riid == IID_IUnknown || riid == IID_IEnumConnections)
    *ppv = static_cast<void*>(this);
  else
    *ppv = NULL;

  if (*ppv)
  {
    reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    return S_OK;
  }
  else
    return E_NOINTERFACE;
}


/**
  IUnknown::AddRef implementation.
*/
STDMETHODIMP_(ULONG) tLuaCOMEnumConnections::AddRef()
{
  mpHost->AddRef();
  return ++mRefCount;
}


/**
  IUnknown::Release implementation.
*/
STDMETHODIMP_(ULONG) tLuaCOMEnumConnections::Release()
{
  mpHost->Release();
  if (--mRefCount == 0)
  {
    mRefCount++;
    delete this;
  }
  return mRefCount;
}


/**
  IEnumConnections::Next implementation.
*/
STDMETHODIMP tLuaCOMEnumConnections::Next(
    ULONG cReq, CONNECTDATA* paConnections, ULONG* pcEnumerated)
{
  // check bad arguments
  if (!paConnections)
    return E_POINTER;
  if (!pcEnumerated && cReq != 1)
    return E_POINTER;
  if (!(mNextIndex < mConnections.size()))
    {
    *pcEnumerated = 0;
    return S_FALSE;
    }

  // copy values and advance mNextIndex.
  unsigned int i = 0;
  for ( ; i<cReq && mNextIndex < mConnections.size(); i++, mNextIndex++)
    {
    if (mConnections[mNextIndex]) mConnections[mNextIndex]->AddRef();
    paConnections[i].pUnk     = mConnections[mNextIndex];
    paConnections[i].dwCookie = mCookies[mNextIndex];
    }

  // number of values returned
  if (pcEnumerated)
    *pcEnumerated = i;

  return S_OK;
}


/**
  IEnumConnections::Skip implementation.
*/
STDMETHODIMP tLuaCOMEnumConnections::Skip(ULONG cSkip)
{
  if (!(mNextIndex + cSkip < mConnections.size()))
    return S_FALSE;
  mNextIndex += cSkip;
  return S_OK;
}


/**
  IEnumConnections::Reset implementation.
*/
STDMETHODIMP tLuaCOMEnumConnections::Reset()
{
  mNextIndex = 0;
  return S_OK;
}


/**
  IEnumConnections::Clone implementation.
*/
STDMETHODIMP tLuaCOMEnumConnections::Clone(IEnumConnections** ppIEnum)
{
  HRESULT hr;
  *ppIEnum = NULL;

  tLuaCOMEnumConnections* penumc = new tLuaCOMEnumConnections(mpHost); // E_OUTOFMEMORY
  hr = penumc->Init(mConnections, mCookies, mNextIndex);
  if (SUCCEEDED(hr))
    hr = penumc->QueryInterface(
        IID_IEnumConnections, reinterpret_cast<void **>(ppIEnum));

  return hr;
}



///
/// tLuaCOMConnPointContainer
///



/**
  Constructor.
*/
tLuaCOMConnPointContainer::tLuaCOMConnPointContainer(lua_State* pL, IUnknown* pOuter) :
  mpOuter(pOuter),
  L(pL)
{
  CHECKPARAM(pL); CHECKPARAM(pOuter);

  // creates connection point for source interface
  tCOMPtr<IProvideClassInfo2> ci2;
  CHK_COM_CODE(mpOuter->QueryInterface(IID_IProvideClassInfo2, reinterpret_cast<void **>(&ci2))); 
  IID iid;
  CHK_COM_CODE(ci2->GetGUID(GUIDKIND_DEFAULT_SOURCE_DISP_IID, &iid));
  tCOMPtr<ITypeInfo> coclassinfo;
  CHK_COM_CODE(ci2->GetClassInfo(&coclassinfo));
  ITypeInfo *events_typeinfo = tCOMUtil::GetDefaultInterfaceTypeInfo(coclassinfo, true);
  CHK_LCOM_ERR(events_typeinfo, "No default source typeinfo.");
  mPoints.push_back(new tLuaCOMConnPoint(L, mpOuter));
  CHK_COM_CODE(mPoints[0]->Init(iid, events_typeinfo));
  mDefaultPoint = mPoints[0];
}



tLuaCOMConnPointContainer::~tLuaCOMConnPointContainer()
{
}


STDMETHODIMP tLuaCOMConnPointContainer::QueryInterface(REFIID riid, void ** ppv)
{
  return mpOuter->QueryInterface(riid, ppv);
}



STDMETHODIMP_(ULONG) tLuaCOMConnPointContainer::AddRef()
{
  return mpOuter->AddRef();
}


STDMETHODIMP_(ULONG) tLuaCOMConnPointContainer::Release()
{
  return mpOuter->Release();
}


/**
  IConnectionPointContainer::FindConnectionPoint implementation.
*/
STDMETHODIMP tLuaCOMConnPointContainer::FindConnectionPoint(
     REFIID riid, IConnectionPoint** ppConnPt)
{
  *ppConnPt = NULL;

  HRESULT hr = E_NOINTERFACE;
  if (mDefaultPoint)
  {
    IID iid;
    mDefaultPoint->GetConnectionInterface(&iid);

    if(iid == riid)
    {
      hr = mDefaultPoint->QueryInterface(IID_IConnectionPoint,
                 reinterpret_cast<void **>(ppConnPt));
    }
    else
    {
      hr = CONNECT_E_NOCONNECTION;
    }
  }

  return hr;
}


/**
  IConnectionPointContainer::EnumConnectionPoints implementation.
*/
STDMETHODIMP tLuaCOMConnPointContainer::EnumConnectionPoints(
    IEnumConnectionPoints** ppIEnum)
{
  HRESULT hr = S_OK;
  
  // create IEnumConnectionPoints
  *ppIEnum = NULL;
  tLuaCOMEnumConnPoints* penumcp = new tLuaCOMEnumConnPoints(this); // E_OUTOFMEMORY
  std::vector<tCOMPtr<IConnectionPoint> > points; // [ok? empty]
  hr = penumcp->Init(points, 0);
  if (FAILED(hr))
    return hr;
  hr = penumcp->QueryInterface(
        IID_IEnumConnectionPoints, reinterpret_cast<void **>(ppIEnum));

  return hr;
}


tLuaCOMConnPoint* tLuaCOMConnPointContainer::GetDefault()
{
  return mDefaultPoint;
}



///
/// tLuaCOMConnPoint
///



void tLuaCOMConnPoint::push()
{
  // Lua `setmetatable({[CONNPOINT_NAME]=this}, mt)`
  LUASTACK_SET(L);
  lua_newtable(L);
  luaCompat_pushTypeByName(L, MODULENAME, LCOM_CONNPOINT_TYPENAME);
  lua_setmetatable(L, -2);
  lua_pushstring(L, CONNPOINT_NAME);
  lua_pushlightuserdata(L, this);
  lua_settable(L, -3);
  LUASTACK_CLEAN(L, 1);
}

// v = __index(t, k)
int tLuaCOMConnPoint::l_tagmeth_index(lua_State *L)
{
  // Lua `self[CONNPOINT_NAME], k`
  lua_pushstring(L, CONNPOINT_NAME);
  lua_gettable(L, 1);  // self[CONNPOINT_NAME]
  lua_pushvalue(L, 2); // k (event name)

  // pushes closure to call all sinks
  lua_pushcclosure(L, l_call_sinks, 2);

  return 1;
}


/**
  lua C function that dispatches events to sinks
*/
int tLuaCOMConnPoint::l_call_sinks(lua_State *L)
{
  // positions of parameters
  const int self_param        = 1;  // self param
  const int user_first_param  = 2;  // first user param 
  const int user_last_param   = lua_gettop(L);  // last user param
  const int connpoint         = lua_upvalueindex(1);
  const int event             = lua_upvalueindex(2);

  int num_params = (std::max)(0, user_last_param - user_first_param + 1);

  // gets connection point
  tLuaCOMConnPoint* cp = 
    reinterpret_cast<tLuaCOMConnPoint*>(luaCompat_getPointer(L, connpoint));

  // call each sink
  for(size_t i = 0; i < cp->mConnections.size(); i++)
  {
    // pushes function mSinks[i][event] and lock
    LuaBeans::push(L, cp->mSinks[i]);
    cp->mSinks[i]->Lock();
    lua_pushvalue(L, event);
    lua_gettable(L, -2);
    lua_remove(L, -2);

    // self param (mandatory but unused)
    LuaBeans::push(L, cp->mSinks[i]);
    cp->mSinks[i]->Lock();

    // duplicates parameters (if any)
    for(int j = user_first_param; j <= user_last_param; j++)
    {
      lua_pushvalue(L, j);
    }

    // calls function (including self param), ignoring errors
    luaCompat_call(L, num_params+1, 0);

    // cleans stack
    lua_settop(L, user_last_param);
  }

  return 0;
}
