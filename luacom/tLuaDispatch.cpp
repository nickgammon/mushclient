/**
 tLuaDispatch.cpp: tLuaDispatch class.
 This is an IDispatch implementation that wraps a Lua object.
 
 original author: Vinicius Almendra
*/

#include "tLuaDispatch.h"
#include "tLuaCOM.h"
#include "tLuaCOMException.h"
#include "tCOMUtil.h"
#include "LuaAux.h"
#include "tUtil.h"
#include "LuaCompat.h"

// hacks for certain compilers
#if defined(__MINGW32__) || defined(__CYGWIN__)
#define GUIDKIND_DEFAULT_SOURCE_DISP_IID 1
#endif

volatile long tLuaDispatch::NEXT_ID = 0;

tLuaDispatch::ProvideClassInfo2::ProvideClassInfo2(ITypeInfo* p_coclassinfo,
                                                   IUnknown* p_pUnk)
{
  coclassinfo = p_coclassinfo;
  coclassinfo->AddRef();
  pUnk = p_pUnk;
}

tLuaDispatch::ProvideClassInfo2::~ProvideClassInfo2()
{
  coclassinfo->Release();
}

STDMETHODIMP tLuaDispatch::ProvideClassInfo2::QueryInterface(
                                                            REFIID riid,
                                                            void ** ppvObj)
{
  return pUnk->QueryInterface(riid, ppvObj);
}

STDMETHODIMP_(ULONG) tLuaDispatch::ProvideClassInfo2::AddRef(void)
{
  return pUnk->AddRef();
}

STDMETHODIMP_(ULONG) tLuaDispatch::ProvideClassInfo2::Release(void)
{
  return pUnk->Release();
}


STDMETHODIMP tLuaDispatch::ProvideClassInfo2::GetClassInfo(ITypeInfo** ppTypeInfo)
{
  *ppTypeInfo = coclassinfo;
  coclassinfo->AddRef();

  return S_OK;
}

STDMETHODIMP tLuaDispatch::ProvideClassInfo2::GetGUID(DWORD dwGuidKind,
                                                     GUID * pGUID)
{
  ITypeInfo* source_typeinfo = NULL;
  TYPEATTR *typeattr = NULL;
  HRESULT hr = S_OK;

  if(!pGUID)
    return E_POINTER;

  if(dwGuidKind == GUIDKIND_DEFAULT_SOURCE_DISP_IID)
  {
    source_typeinfo = 
      tCOMUtil::GetDefaultInterfaceTypeInfo(coclassinfo, true);

    if(!source_typeinfo)
      return E_UNEXPECTED;

    hr = source_typeinfo->GetTypeAttr(&typeattr);

    if(FAILED(hr))
    {
      source_typeinfo->Release();
      return E_UNEXPECTED;
    }

   *pGUID = typeattr->guid;

   source_typeinfo->ReleaseTypeAttr(typeattr);
   source_typeinfo->Release();
  }
  else
    return E_INVALIDARG;

  return S_OK;
}



///
/// IUnknown Methods
///


STDMETHODIMP
tLuaDispatch::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    if(IsEqualIID(riid, IID_IUnknown)  ||
       IsEqualIID(riid, IID_IDispatch) ||
       IsEqualIID(riid, interface_iid)) {
      *ppv = (IDispatch*)this;
      AddRef();
      return NOERROR;       
    }

    if((IsEqualIID(riid, IID_IProvideClassInfo) ||
        IsEqualIID(riid, IID_IProvideClassInfo2)   )  &&
       classinfo2)
    {
      *ppv = classinfo2;
      classinfo2->AddRef();
      return NOERROR;
    }

    if(IsEqualIID(riid, IID_IConnectionPointContainer) &&
      cpc)
    {
      *ppv = cpc;
      cpc->AddRef();
      return NOERROR;
    }

    if(IsEqualIID(riid, IID_ILuaDispatch))
    {
      *ppv = (ILuaDispatch*)this;
      AddRef();
      return NOERROR;
    }

    *ppv = NULL;
    return ResultFromScode(E_NOINTERFACE);
}


STDMETHODIMP_(ULONG)
tLuaDispatch::AddRef()
{
    return ++m_refs;
}


STDMETHODIMP_(ULONG)
tLuaDispatch::Release()
{
  assert(m_refs > 0);
  if(--m_refs == 0)
  {
    // unlock Lua table.
    luaL_unref(L, LUA_REGISTRYINDEX, table_ref);

    // free libs

    while(num_methods--)
    {
      typeinfo->ReleaseFuncDesc(funcinfo[num_methods].funcdesc);
      delete funcinfo[num_methods].name;
    }

    delete funcinfo;
    typeinfo->Release();
    typeinfo = NULL;

    delete typehandler;

    if(cpc)
      delete cpc;

    if(classinfo2)
      delete classinfo2;

    // destroi objeto
    delete this;
    return 0;
  }

  return m_refs;
}


///
///  IDispatch Methods
///



STDMETHODIMP
tLuaDispatch::GetTypeInfoCount(unsigned int FAR* pctinfo)
{
    *pctinfo = 1;
    return NOERROR;
}


STDMETHODIMP
tLuaDispatch::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
  if(itinfo != 0)
    return ResultFromScode(TYPE_E_ELEMENTNOTFOUND);

  if(pptinfo == NULL)
    return ResultFromScode(E_POINTER);

  typeinfo->AddRef();
  *pptinfo = typeinfo;

  return NOERROR;
}


STDMETHODIMP
tLuaDispatch::GetIDsOfNames(
    REFIID riid,
    OLECHAR FAR* FAR* rgszNames,
    unsigned int cNames,
    LCID lcid,
    DISPID FAR* rgdispid)
{
    // this object only exposes a "default" interface.
    if(!IsEqualIID(riid, IID_NULL))
      return ResultFromScode(DISP_E_UNKNOWNINTERFACE);

    return DispGetIDsOfNames(typeinfo, rgszNames, cNames, rgdispid);
}

STDMETHODIMP
tLuaDispatch::Invoke(
    DISPID dispidMember,
    REFIID riid,
    LCID lcid,
    unsigned short wFlags,
    DISPPARAMS FAR* pdispparams,
    VARIANT FAR* pvarResult,
    EXCEPINFO FAR* pexcepinfo,
    unsigned int FAR* puArgErr)
{
  HRESULT hresult         = 0;
  const stkIndex baseidx = lua_gettop(L); // stack index on entry

  if(wFlags & ~(DISPATCH_METHOD | DISPATCH_PROPERTYGET | DISPATCH_PROPERTYPUT |
      DISPATCH_PROPERTYPUTREF))
    return ResultFromScode(E_INVALIDARG);

  if(!IsEqualIID(riid, IID_NULL))
    return ResultFromScode(DISP_E_UNKNOWNINTERFACE);

  // lookup method dispatch ID.
  int index = 0;
  for(index = 0; index < num_methods; index++)
  {
    if(funcinfo[index].funcdesc->memid == dispidMember &&
       funcinfo[index].funcdesc->invkind & wFlags)
       break;
  }

  if(index >= num_methods)
  {
    tUtil::log_verbose("tLuaDispatch", "DISPID 0x%.8x not found.",
      dispidMember);
    return (ResultFromScode(DISP_E_MEMBERNOTFOUND));
  }

  // gets a reference to the implementation table
  lua_rawgeti(L, LUA_REGISTRYINDEX, table_ref);
  int implementation_table = lua_gettop(L);

  HRESULT retval = NOERROR;
  try
  {
    switch(wFlags)
    {
    case DISPATCH_METHOD:
      retval = method(
        funcinfo[index].name,
        funcinfo[index].funcdesc,
        pdispparams,
        pvarResult,
        pexcepinfo,
        puArgErr);
      break;

    case DISPATCH_METHOD | DISPATCH_PROPERTYGET:
      // sometimes the caller can't tell the difference between
      // a property get and a method call. We try first a
      // method call and then, if it's not successful, try
      // a property get

      {
        HRESULT ret = method(
          funcinfo[index].name,
          funcinfo[index].funcdesc,
          pdispparams,
          pvarResult,
          pexcepinfo,
          puArgErr);

        if(ret != S_OK)
        {
          lua_settop(L, implementation_table); // reset before next call
          retval = propertyget(
            funcinfo[index].name,
            funcinfo[index].funcdesc,
            pdispparams,
            pvarResult,
            pexcepinfo,
            puArgErr);
        }
      }
      break;

    case DISPATCH_PROPERTYGET:
      retval = propertyget(
        funcinfo[index].name,
        funcinfo[index].funcdesc,
        pdispparams,
        pvarResult,
        pexcepinfo,puArgErr);
      break;

    case DISPATCH_PROPERTYPUT:
      retval = propertyput(
        funcinfo[index].name,
        pdispparams,
        pvarResult,
        pexcepinfo,puArgErr);
      break;

    case DISPATCH_PROPERTYPUTREF:
        retval = DISP_E_EXCEPTION;
        FillExceptionInfo(pexcepinfo, "Property putref not supported.");
      break;

    default: // we can't arrive here unless for a bug...
      retval =  DISP_E_MEMBERNOTFOUND;
      break;
    }
  }
  catch(class tLuaCOMException& e)
  {
    retval = DISP_E_EXCEPTION;
    FillExceptionInfo(pexcepinfo, e.getMessage());
  }

  if(retval == DISP_E_MEMBERNOTFOUND)
  {
    tUtil::log_verbose("tLuaDispatch", "Member '%s' not found (0x%.8x)",
      funcinfo[index].name, dispidMember);
  }
  else
  {
    tUtil::log_verbose("tLuaDispatch", "Member '%s' called (0x%.8x)",
      funcinfo[index].name, dispidMember);
  }

  lua_settop(L, baseidx); // reset on exit
  return retval;
}

STDMETHODIMP tLuaDispatch::PushIfSameState(lua_State *p_L) {
  lua_rawgeti(L, LUA_REGISTRYINDEX, table_ref);
  if(lua_isnil(p_L, -1)) {
    lua_pop(p_L,1);
    return E_FAIL;
  }
  const void *p1 = lua_topointer(p_L, -1);
  lua_rawgeti(L, LUA_REGISTRYINDEX, table_ref);
  const void *p2 = lua_topointer(L, -1);
  if(p1 == p2) {
    lua_pop(L, 1);
    return S_OK;
  } else {
    lua_pop(L, 1);
    lua_pop(p_L, 1);
    return E_FAIL;
  }
}

tLuaDispatch::tLuaDispatch(lua_State* p_L, ITypeInfo * pTypeinfo, int ref)
{
  HRESULT hr;

  L = p_L;

  // gets a locked reference to the implementation object
  lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
  int locked_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  table_ref = locked_ref;

  m_refs = 0;

  typeinfo = pTypeinfo;

  typeinfo->AddRef();

  // initialize type converter
  typehandler = new tLuaCOMTypeHandler(typeinfo);

  {
    TYPEATTR * ptypeattr = NULL;
    FUNCDESC *funcdesc = NULL;

    hr = typeinfo->GetTypeAttr(&ptypeattr);

    CHECK(SUCCEEDED(hr), INTERNAL_ERROR);

    memcpy(&interface_iid, &ptypeattr->guid, sizeof(IID));


    // Obtain all the descriptions of the functions and store them in a table.

    unsigned int n = ptypeattr->cFuncs;
    typeinfo->ReleaseTypeAttr(ptypeattr);

    funcinfo = new tFuncInfo[n];

    num_methods = 0;

    for (unsigned int i = 0; i < n;i++)
    {
      hr = typeinfo->GetFuncDesc(i, &funcdesc);

      CHECK(SUCCEEDED(hr) && funcdesc != NULL, INTERNAL_ERROR);

      // ignores functions not accessible via Automation
      if(funcdesc->wFuncFlags & FUNCFLAG_FRESTRICTED)
        continue;

      funcinfo[num_methods].funcdesc = funcdesc;

      BSTR names[1];
      unsigned int dumb; 
      hr = typeinfo->GetNames(funcdesc->memid, names, 1, &dumb); 
      CHECK(SUCCEEDED(hr), INTERNAL_ERROR);

      const int str_size = SysStringLen(names[0]);

      funcinfo[num_methods].name = new char[str_size + 1];
      wcstombs(funcinfo[num_methods].name, names[0], str_size+1);
      SysFreeString(names[0]);

      num_methods++;
    }
  }

  cpc = NULL;
  classinfo2 = NULL;

  ID = InterlockedIncrement(&(tLuaDispatch::NEXT_ID));
  tUtil::log_verbose("tLuaDispatch", "%.4d:created", ID);

  return;
}


tLuaDispatch * tLuaDispatch::CreateLuaDispatch(lua_State* L,
                                               ITypeInfo* interface_typeinfo,
                                               int ref
                                               )
{
  tLuaDispatch *pdisp = 
    new tLuaDispatch(L, interface_typeinfo, ref);

  lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
  lua_pushlightuserdata(L, idxDispatch);
  lua_pushlightuserdata(L, pdisp);
  lua_rawset(L,-3);
  lua_pop(L, 1);

  pdisp->AddRef();

  return pdisp;
}

/*
 Fills an EXCEPINFO structure.
*/
void tLuaDispatch::FillExceptionInfo(EXCEPINFO *pexcepinfo, const char *text)
{
  CHECKPARAM(text);

  tUtil::log("tLuaDispatch", "Exception (%s)", text);

  if(!pexcepinfo)
    return;

  pexcepinfo->wCode             = 1000;
  pexcepinfo->wReserved         = 0;
  pexcepinfo->bstrSource        = tUtil::string2bstr("LuaCOM");
  pexcepinfo->bstrDescription   = tUtil::string2bstr(text);
  pexcepinfo->bstrHelpFile      = NULL;
  pexcepinfo->pvReserved        = NULL;
  pexcepinfo->pfnDeferredFillIn = NULL;
  pexcepinfo->scode             = 0;
}


// helper function for Invoke().
// note: does not restore Lua stack
HRESULT tLuaDispatch::propertyget(const char* name,
                                  FUNCDESC* funcdesc,
                                  DISPPARAMS *pdispparams,
                                  VARIANT *pvarResult,
                                  EXCEPINFO *pexcepinfo,
                                  unsigned int *puArgErr)
{
  // the value contained in the table.
  lua_pushstring(L, name);
  lua_gettable(L, -2);
  stkIndex member = lua_gettop(L);

  if(lua_isnil(L, member))
  {
    return DISP_E_MEMBERNOTFOUND;
  }

  // if field holds a function, calls it passing the parameters
  if(lua_isfunction(L, member))
  {
    // first, the self parameter
    lua_pushvalue(L, -2);
    int i = pdispparams->cArgs;

    while(i--)
      typehandler->com2lua(L, pdispparams->rgvarg[i]);

    tStringBuffer err;
    luaCompat_call(L, pdispparams->cArgs + 1, 1, err);

    if(err)
    {
      FillExceptionInfo(pexcepinfo, err);
      return DISP_E_EXCEPTION;
    }
  }

  if(pdispparams->cArgs > 0) // parameterized propertyget
  {
    if(lua_istable(L, member))
    {
      lua_pushvalue(L, member);
      typehandler->com2lua(L, pdispparams->rgvarg[pdispparams->cArgs - 1]);

      lua_gettable(L, -2);
      member = lua_gettop(L);
    }
    else
    {
      // functions as a normal propget, ignoring parameters
    }
  }

  // Converts Lua result to DISPPARAMS.

  if(pvarResult != NULL)
  {
    typehandler->setRetval(L, funcdesc, member, pvarResult);
  }

  return S_OK;
}

// helper function for Invoke().
// note: does not restore Lua stack
HRESULT tLuaDispatch::propertyput(const char* name,
                                  DISPPARAMS *pdispparams,
                                  VARIANT *pvarResult,
                                  EXCEPINFO *pexcepinfo,
                                  unsigned int *puArgErr)
{
  // gets the current of the field
  lua_pushstring(L, name);
  lua_gettable(L, -2);

  // If field to be set holds a function, calls it passing the
  // value to be set
  if(lua_isfunction(L, -1))
  {
    stkIndex function = lua_gettop(L);

    // first, the self param
    lua_pushvalue(L, -2);

    // pushes all the arguments
    int i = pdispparams->cArgs;

    while(i--)
      typehandler->com2lua(L, pdispparams->rgvarg[i]);

    tStringBuffer err;
    luaCompat_call(L, pdispparams->cArgs + 1, 0, err);

    if(err)
    {
      FillExceptionInfo(pexcepinfo, err);
      return DISP_E_EXCEPTION;
    }
  }
  else if(pdispparams->cArgs == 1) // propput normal
  {
    lua_pop(L, 1); // removes value of the field

    lua_pushstring(L, name);

    // value to be set
    typehandler->com2lua(L, pdispparams->rgvarg[pdispparams->cArgs - 1]);

    lua_settable(L, -3);
  }
  else if(pdispparams->cArgs == 2) // parameterized propertyput
  {
    stkIndex member = lua_gettop(L);

    if(lua_istable(L, member) || lua_isuserdata(L, member)) 
    {
      lua_pushvalue(L, member);

      // index
      typehandler->com2lua(L, pdispparams->rgvarg[pdispparams->cArgs - 1]);

      // value to be set
      typehandler->com2lua(L, pdispparams->rgvarg[pdispparams->cArgs - 2]);

      lua_settable(L, -3);
    }
    else
    {
      FillExceptionInfo(pexcepinfo, "Parametrized property puts only "
        "implemented for tables and userdata's.");

      return DISP_E_EXCEPTION;
    }
  }
  else
  {
    FillExceptionInfo(pexcepinfo, "Property puts with more than two "
      "parameters aren't supported.");

    return DISP_E_EXCEPTION;
  }

  return S_OK;
}


// Helper function for Invoke().
// note: does not restore Lua stack
HRESULT tLuaDispatch::method(const char* name,
                             FUNCDESC* funcdesc,
                             DISPPARAMS *pdispparams,
                             VARIANT *pvarResult,
                             EXCEPINFO *pexcepinfo,
                             unsigned int *puArgErr)
{
  // push Lua function (member) from implementation table
  lua_pushstring(L, name);
  lua_gettable(L, -2);
  const stkIndex memberidx = lua_gettop(L);  // and stack index
      
  // test whether it's really a function
  if(!lua_isfunction(L, memberidx))
  {
    return DISP_E_MEMBERNOTFOUND;
  }

  // convert parameters and stacks

  // push self argument
  lua_rawgeti(L, LUA_REGISTRYINDEX, table_ref);
  
  // push COM arguments
  typehandler->pushLuaArgs(
    L,
    pdispparams,
    funcdesc->lprgelemdescParam);

  // call Lua function
  tStringBuffer errmsg;
  int result = luaCompat_call(L, lua_gettop(L) - memberidx, LUA_MULTRET, errmsg);
  if(result)
  {
    FillExceptionInfo(pexcepinfo, LuaAux::makeLuaErrorMessage(result, errmsg));
    return DISP_E_EXCEPTION;
  }
  // note: only return values are left above baseidx
  stkIndex returnidx = memberidx; // start index of return values

  // set COM return value
  if(lua_type(L, returnidx) != LUA_TNONE)
  {
    if(pvarResult != NULL && funcdesc->elemdescFunc.tdesc.vt != VT_VOID)
    {
      VARIANTARG result;
      VariantInit(&result);

      typehandler->lua2com(L, returnidx, result, funcdesc->elemdescFunc.tdesc.vt);

      *pvarResult = result;

      returnidx++; // consume
    }
  }

  // set COM out arguments
  if(lua_type(L, returnidx) != LUA_TNONE)
    typehandler->setOutValues(L, funcdesc, pdispparams, returnidx);

  return S_OK;
}

void tLuaDispatch::SetCoClassinfo(ITypeInfo *coclassinfo)
{
  classinfo2 = new ProvideClassInfo2(coclassinfo, (IDispatch*)this);
  CHKMALLOC(classinfo2);
}

void tLuaDispatch::BeConnectable(void)
{
  try
  {
    cpc = new tLuaCOMConnPointContainer(L, (IDispatch*)this);
  }
  catch(class tLuaCOMException& e)
  {
    UNUSED(e);
    cpc = NULL;
  }
}

tLuaCOMConnPointContainer* tLuaDispatch::GetConnPointContainer()
{
  return cpc;
}

tLuaDispatch::~tLuaDispatch()
{
  tUtil::log_verbose("tLuaDispatch", "%.4d:destroyed", ID);
}
