#ifndef __LUACOM_CONNECTIONPOINTS_H
#define __LUACOM_CONNECTIONPOINTS_H

#include "luabeans.h"
#include <vector>
#include <tCOMUtil.h>

/**
  IEnumConnectionPoints implementation.
*/
class tLuaCOMEnumConnPoints : public IEnumConnectionPoints
{
public:
  // Construction.
  tLuaCOMEnumConnPoints(IUnknown* pHost);
  ~tLuaCOMEnumConnPoints();
  HRESULT Init(const std::vector<tCOMPtr<IConnectionPoint> > & points, ULONG nextIndex);

  // IUnknown methods.
  STDMETHODIMP         QueryInterface(REFIID, void **);
  STDMETHODIMP_(ULONG) AddRef();
  STDMETHODIMP_(ULONG) Release();

  // IEnumConnectionPoints methods.
  STDMETHODIMP         Next(ULONG, IConnectionPoint**, ULONG*);
  STDMETHODIMP         Skip(ULONG);
  STDMETHODIMP         Reset();
  STDMETHODIMP         Clone(IEnumConnectionPoints**);

private:
  // COM reference count.
  ULONG mRefCount;

  // IUnknown pointer to host COM object being enumerated.
  IUnknown* mpHost;

  // Allocated array of Connection Point interface pointers.
  std::vector<tCOMPtr<IConnectionPoint> > mPoints;

  // Next() returns mPoints[mNextIndex]
  ULONG mNextIndex;
};

typedef tLuaCOMEnumConnPoints* PtLuaCOMEnumConnPoints;


/**
  IConnectionPoint implementtaion.
*/
class tLuaCOMConnPoint : public IConnectionPoint
{
public:
  void push();
  // Construction.
  tLuaCOMConnPoint(lua_State* L, IUnknown* pHost);
  ~tLuaCOMConnPoint();
  HRESULT Init(REFIID riid, ITypeInfo *typeinfo);

  // IUnknown methods.
  STDMETHODIMP         QueryInterface(REFIID, void **);
  STDMETHODIMP_(ULONG) AddRef();
  STDMETHODIMP_(ULONG) Release();

  // IConnectionPoint methods.
  STDMETHODIMP GetConnectionInterface(IID*);
  STDMETHODIMP GetConnectionPointContainer(IConnectionPointContainer**);
  STDMETHODIMP Advise(IUnknown*, DWORD*);
  STDMETHODIMP Unadvise(DWORD);
  STDMETHODIMP EnumConnections(IEnumConnections**);

  // Lua functions
  static int l_tagmeth_index(lua_State* L);

private:
  static int l_call_sinks(lua_State *L);

  // COM reference count
  ULONG mRefCount;

  // host object
  IUnknown* mpHost;

  // connection point sink IID
  IID mIIDSink;

  // cookie counter for mCookies)
  DWORD mNextCookie;

  // connection point sinks
  std::vector<tCOMPtr<IUnknown> > mConnections;
  std::vector<DWORD> mCookies;

  // holds LuaCOM objects for each sink
  std::vector<tLuaCOM*> mSinks;

  ITypeInfo* mpTypeinfo;
  lua_State* L;
};

typedef tLuaCOMConnPoint* PtLuaCOMConnPoint;


/**
  IEnumConnections implementation.
*/
class tLuaCOMEnumConnections : public IEnumConnections
{
public:
  // Construction.
  tLuaCOMEnumConnections(IUnknown* pHost);
  ~tLuaCOMEnumConnections();
  HRESULT Init(std::vector<tCOMPtr<IUnknown> > & connections,
      std::vector<DWORD> & cookies, ULONG nextIndex);

  // IUnknown methods.
  STDMETHODIMP         QueryInterface(REFIID, void **);
  STDMETHODIMP_(ULONG) AddRef();
  STDMETHODIMP_(ULONG) Release();

  // IEnumConnections methods.
  STDMETHODIMP         Next(ULONG, CONNECTDATA*, ULONG*);
  STDMETHODIMP         Skip(ULONG);
  STDMETHODIMP         Reset();
  STDMETHODIMP         Clone(IEnumConnections**);

private:
  // COM reference count
  ULONG mRefCount;

  // host object to be enumerated
  IUnknown* mpHost;

  // connections.  Like CONNECTDATA
  std::vector<tCOMPtr<IUnknown> > mConnections;
  std::vector<DWORD> mCookies;

  // Next() returns mConnections[nNextIndex]
  ULONG mNextIndex;
};

typedef tLuaCOMEnumConnections* PtLuaCOMEnumConnections;


/**
  IConnectionPointContainer implementation.
*/
class tLuaCOMConnPointContainer : public IConnectionPointContainer
{
public:
  tLuaCOMConnPoint* GetDefault();
  tLuaCOMConnPointContainer(lua_State *pL, IUnknown* pOuter);
  ~tLuaCOMConnPointContainer();

  // IUnknown methods.
  STDMETHODIMP         QueryInterface(REFIID, void **);
  STDMETHODIMP_(ULONG) AddRef();
  STDMETHODIMP_(ULONG) Release();

  // IConnectionPointContainer methods.
  STDMETHODIMP         FindConnectionPoint(REFIID, IConnectionPoint**);
  STDMETHODIMP         EnumConnectionPoints(IEnumConnectionPoints**);

private:
  IUnknown* mpOuter;

  tLuaCOMConnPoint* mDefaultPoint;

  std::vector<tCOMPtr<tLuaCOMConnPoint> > mPoints;

  lua_State* L;
};


#endif // __LUACOM_CONNECTIONPOINTS_H
