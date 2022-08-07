/**
  tLuaCOMTypeHandler.cpp: Class tLuaCOMTypeHandler, which handles
  conversion of values between Lua and COM values.
*/

// Required for VARIANT field accessors to compile under wineg++.
#ifdef __WINE__
# define _FORCENAMELESSUNION  // used by oaidl.h
#endif

#include <ole2.h>

extern "C"
{
#include <lua.h>
}
#include "LuaCompat.h"

#include <assert.h>
#include <stdio.h>

#include "tLuaCOMTypeHandler.h"
#include "tLuaCOM.h"
#include "tLuaVector.h"
#include "tLuaCOMException.h"
#include "tLuaDispatch.h"
#include "tCOMUtil.h"

#include "tLuaCOMEnumerator.h"

#include "tUtil.h"
#include "LuaAux.h"
#include "luabeans.h"

#include "luacom_internal.h"

#define LUA_NOOBJECT 0

static const VARTYPE com_types[] = {
  VT_R8, VT_R4, VT_ARRAY, VT_R8, VT_R8, VT_I8, VT_UI8, VT_I4, VT_UI4,
  VT_I2, VT_UI2, VT_I1, VT_UI1, VT_INT, VT_UINT, VT_BSTR, VT_NULL, VT_ERROR, VT_BOOL
};

static const char *const com_type_names[] = {
  "double", "float", "array", "decimal", "currency", "int8", "uint8", "int4", "uint4",
  "int2", "uint2", "int1", "uint1", "int", "uint", "string", "null", "error", "bool", NULL
};


tLuaCOMTypeHandler::tLuaCOMTypeHandler(ITypeInfo *ptypeinfo)
{
  m_typeinfo.Attach(ptypeinfo);
  if(m_typeinfo) m_typeinfo->AddRef();
}

tLuaCOMTypeHandler::~tLuaCOMTypeHandler()
{
}


/**
  Pushes onto Lua stack table explicitly representing VARIANT of type `vt` and
  numeric value `val`.  Lua value is of the form {Type=f(vt), Value=val}.
  helper function for com2lua.
*/
void tLuaCOMTypeHandler::pushTableVarNumber(lua_State *L, VARTYPE vt, double val) {
  lua_newtable(L);
  lua_pushstring(L, "Type");
  switch(vt) {
    case VT_CY:
      lua_pushstring(L, "currency");
      break;
    case VT_UI1:
      lua_pushstring(L, "uint1");
      break;
    case VT_UI2:
      lua_pushstring(L, "uint2");
      break;
    case VT_UI4:
      lua_pushstring(L, "uint4");
      break;
    case VT_INT:
      lua_pushstring(L, "int");
      break;
    case VT_UINT:
      lua_pushstring(L, "uint");
      break;
    case VT_I1:
      lua_pushstring(L, "int1");
      break;
    case VT_I2:
      lua_pushstring(L, "int2");
      break;
    case VT_I4:
      lua_pushstring(L, "int4");
      break;
    case VT_R4:
      lua_pushstring(L, "float");
      break;
    case VT_R8:
      lua_pushstring(L, "double");
      break;
    case VT_DECIMAL:
      lua_pushstring(L, "decimal");
      break;
    default:
      lua_pushstring(L, "double");
  }
  lua_settable(L, -3);
  lua_pushstring(L, "Value");
  lua_pushnumber(L, val);
  lua_settable(L, -3);
}


/**
  Pushes onto Lua stack representation of VARIANTARG.
  If is_variant, then Lua value will be in table form (e.g. `{Type=...}`),
  which is more explicit.
*/
void tLuaCOMTypeHandler::com2lua(lua_State* L, VARIANTARG varg_orig, bool is_variant)
{
  LUASTACK_SET(L);

  HRESULT hr = S_OK;

  VARIANT varg;
  VariantInit(&varg);

  lua_getglobal(L, "luacom");
  lua_pushstring(L, "TableVariants");
  lua_gettable(L, -2);
  bool table_variants = lua_toboolean(L, -1) != 0;
  lua_pop(L, 2);


  // dereferences VARIANTARG (if necessary)
  hr = VariantCopyInd(&varg, &varg_orig);
  if (FAILED(hr))
    COM_ERROR(tUtil::GetErrorMessage(hr));

  // Gives a different treatment to SAFEARRAYs
  if (varg.vt & VT_ARRAY)
  {
    // treats an array of VT_UI1 as an array of char and
    // converts it to a string
    if (varg.vt == (VT_ARRAY | VT_UI1))
      safearray2string(L, varg);
    else
      safearray_com2lua(L, varg);
    VariantClear(&varg);
  }
  else
  {
    // used in some type conversions
    VARIANTARG new_varg;
    VariantInit(&new_varg);

    try
    {
      switch (varg.vt)
      {
      case VT_EMPTY:
        if(is_variant && table_variants)
        {
          lua_newtable(L);
          lua_pushstring(L, "Type");
          lua_pushstring(L, "empty");
          lua_settable(L, -3);
        }
        else lua_pushnil(L);
        break;
      case VT_NULL:      // SQL's NULL value.
        if(is_variant && table_variants)
        {
          lua_newtable(L);
          lua_pushstring(L, "Type");
          lua_pushstring(L, "null");
          lua_settable(L, -3);
        }
        else lua_pushnil(L);
        break;
      case VT_CY:
      case VT_UI1:
      case VT_UI2:
      case VT_UI4:
      case VT_INT:
      case VT_UINT:
      case VT_I1:
      case VT_I2:
      case VT_I4:
      case VT_R4:
      case VT_DECIMAL:
      case VT_R8:
        {
          new_varg.vt = VT_R8;
          HRESULT hr = VariantChangeType(&new_varg, &varg, 0, VT_R8);
          CHK_COM_CODE(hr);

          if(is_variant && table_variants)
            pushTableVarNumber(L, varg.vt, new_varg.dblVal);
          else
            lua_pushnumber(L, new_varg.dblVal);
          break;
        }

      case VT_DATE:
        {
          lua_getglobal(L,"luacom");
          lua_pushstring(L,"DateFormat");
          lua_gettable(L, -2);
          tStringBuffer dateformat(lua_tostring(L, -1));
             lua_pop(L, 2);
          if(dateformat == NULL || (strcmp("string",dateformat)==0))
          {
            HRESULT hr = VariantChangeType(&new_varg, &varg, 0, VT_BSTR);
            CHK_COM_CODE(hr);

            lua_pushstring(L, tUtil::bstr2string(new_varg.bstrVal));
          }
          else if(strcmp("table",dateformat)==0)
          {
            SYSTEMTIME date;
            VariantTimeToSystemTime(varg.date,&date);
            lua_newtable(L);
            lua_pushstring(L, "Day");
            lua_pushnumber(L, date.wDay);
            lua_settable(L, -3);
            lua_pushstring(L, "DayOfWeek");
            lua_pushnumber(L, date.wDayOfWeek);
            lua_settable(L, -3);
            lua_pushstring(L, "Month");
            lua_pushnumber(L, date.wMonth);
            lua_settable(L, -3);
            lua_pushstring(L, "Year");
            lua_pushnumber(L, date.wYear);
            lua_settable(L, -3);
            lua_pushstring(L, "Hour");
            lua_pushnumber(L, date.wHour);
            lua_settable(L, -3);
            lua_pushstring(L, "Minute");
            lua_pushnumber(L, date.wMinute);
            lua_settable(L, -3);
            lua_pushstring(L, "Second");
            lua_pushnumber(L, date.wSecond);
            lua_settable(L, -3);
            lua_pushstring(L, "Milliseconds");
            lua_pushnumber(L, date.wMilliseconds);
            lua_settable(L, -3);
          }

          break;
        }


      case VT_ERROR:
        // assumes that a parameter has been omitted
        if(is_variant && table_variants) {
            lua_newtable(L);
            lua_pushstring(L, "Type");
            lua_pushstring(L, "error");
            lua_settable(L, -3);
        }
  else
          lua_pushnil(L);
        break;

      case VT_BOOL:
        if(is_variant && table_variants) {
            lua_newtable(L);
            lua_pushstring(L, "Type");
            lua_pushstring(L, "bool");
            lua_settable(L, -3);
            lua_pushstring(L, "Value");
            lua_pushboolean(L, varg.boolVal);
            lua_settable(L, -3);
        }
  else
          lua_pushboolean(L, varg.boolVal);
        break;

      case VT_BSTR:
        {
          tStringBuffer str = tUtil::bstr2string(varg.bstrVal);
            if(is_variant && table_variants) {
              lua_newtable(L);
            lua_pushstring(L, "Type");
            lua_pushstring(L, "string");
            lua_settable(L, -3);
            lua_pushstring(L, "Value");
            lua_pushlstring(L, str, str.getSize()-1);
            lua_settable(L, -3);
          } else {
              lua_pushlstring(L, str, str.getSize()-1);
          }

          break;
        }

      case VT_DISPATCH:
        {
          unsigned int ninfo = 0;
          IDispatch *pdisp = varg.pdispVal;

          if(pdisp == NULL)
          {
            lua_pushnil(L);
            break;
          }

          IUnknown* pProxMgr;
          ILuaDispatch *pLuaDispatch;
          if((pdisp->QueryInterface(IID_IProxyManager, (void**)&pProxMgr)==E_NOINTERFACE) &&
              SUCCEEDED(pdisp->QueryInterface(IID_ILuaDispatch, (void**)&pLuaDispatch)) &&
              SUCCEEDED(pLuaDispatch->PushIfSameState(L)))
            break;

          tLuaCOM* lcom = NULL;

          try
          {
            lcom = tLuaCOM::CreateLuaCOM(L, pdisp);
          }
          catch(class tLuaCOMException& e)
          {
            UNUSED(e);
            lua_pushnil(L);
            break;
          }

          LuaBeans::push(L, lcom);
        }
        break;

      case VT_UNKNOWN:
        {
          // first, tries to get an IDispatch. If not possible,
          // pushes pointer
          IUnknown* punk = varg.punkVal;
  
          tCOMPtr<IDispatch> pdisp;
          hr = punk->QueryInterface(IID_IDispatch, (void **) &pdisp);

          if(SUCCEEDED(hr))
          {
            IUnknown* pProxMgr;
            ILuaDispatch *pLuaDispatch;
            if((pdisp->QueryInterface(IID_IProxyManager, (void**)&pProxMgr)==E_NOINTERFACE) &&
                SUCCEEDED(pdisp->QueryInterface(IID_ILuaDispatch, (void**)&pLuaDispatch)) &&
                  SUCCEEDED(pLuaDispatch->PushIfSameState(L)))
              break;

               tLuaCOM* lcom = NULL;

            try { lcom = tLuaCOM::CreateLuaCOM(L, pdisp); }
            catch(class tLuaCOMException& e)
            {
              UNUSED(e);
              lua_pushnil(L);
              break;
            }

            LuaBeans::push(L, lcom);
            break;
          }

          tCOMPtr<IEnumVARIANT> pEV;
          hr = punk->QueryInterface(IID_IEnumVARIANT, (void **) &pEV);
          if(SUCCEEDED(hr))
          {
            tLuaCOMEnumerator* enumerator = new tLuaCOMEnumerator(pEV);

            enumerator->push(L);
            break;
          }

          // defaults to pushing and userdata for the IUnknown
          varg.punkVal->AddRef();
          pushIUnknown(L, varg.punkVal);
          break;
        }

      default:
        {
          char msg[100];
          sprintf(msg, "COM->Lua - Type 0x%.2x not implemented.", varg.vt);
          TYPECONV_ERROR(msg);
          break;
        }
      }
    }
    catch(class tLuaCOMException&)
    {
      VariantClear(&varg);
      throw;
    }

    VariantClear(&new_varg);
    VariantClear(&varg);
  }

  LUASTACK_CLEAN(L, 1);
}


// helper function for lua2com.
tLuaCOM *tLuaCOMTypeHandler::convert_table(lua_State *L, stkIndex luaval)
{
  tLuaCOM *lcom;
  stkIndex table = lua_gettop(L);
  lua_pushvalue(L, luaval);
  lua_pushstring(L, "typelib");
  lua_gettable(L, table);
  if(!lua_isnil(L, -1))
  {
    lua_getglobal(L, "luacom");
    lua_pushstring(L, "ImplInterfaceFromTypelib");
    lua_gettable(L, -2);
    lua_remove(L, -2);
    lua_insert(L, table+1);
    lua_pushstring(L, "interface");
    lua_gettable(L, table);
    lua_pushstring(L, "coclass");
    lua_gettable(L, table);
    if(lua_isnil(L, -1)) {
      lua_pop(L, 1);
      lua_call(L, 3, 1);
    } else {
      lua_call(L, 4, 1);
    }
  }
  else
  {
    lua_pop(L, 1);
    lua_getglobal(L, "luacom");
    lua_pushstring(L, "ImplInterface");
    lua_gettable(L, -2);
    lua_remove(L, -2);
    lua_insert(L, table+1);
    lua_pushstring(L, "progid");
    lua_gettable(L, table);
    lua_pushstring(L, "interface");
    lua_gettable(L, table);
    lua_call(L, 3, 1);
  }
  lcom = from_lua(L, lua_gettop(L));
  lua_settop(L, table-1);
  return lcom;
}


/**
  Sets VARIANTARG to stack-based Lua value (luaval) converted to VARTYPE.

  note: type only used when luaCompat_checkTagToCom(L, luaval)
*/
void tLuaCOMTypeHandler::lua2com(lua_State* L, stkIndex luaval, VARIANTARG& varg, VARTYPE type)
{
  LUASTACK_SET(L);

  CHECKPARAM(luaval > 0);

  VariantClear(&varg);

  switch(lua_type(L, luaval))
  {
  case LUA_TNUMBER:
    varg.dblVal = lua_tonumber(L, luaval);
    varg.vt = VT_R8;
    break;


  case LUA_TSTRING:
    {
      tStringBuffer str;
      size_t l_len = lua_strlen(L, luaval);
      str.copyToBuffer(lua_tostring(L, luaval), l_len);
      varg.vt = VT_BSTR;
      varg.bstrVal = tUtil::string2bstr(str, l_len);
    }
    break;


  case LUA_TTABLE:
    {
    // tests whether it's a LuaCOM object
    tLuaCOM* lcom = from_lua(L, luaval);

    if(lcom)
    {
      varg.pdispVal = lcom->GetIDispatch();

      varg.pdispVal->AddRef();
      varg.vt = VT_DISPATCH;
    }
    else
    {
      // tests if it is an already created tLuaDispatch
      lua_pushlightuserdata(L, idxDispatch);
      lua_rawget(L, luaval);
      if(!lua_isnil(L, -1)) {
        varg.pdispVal = (IDispatch*)luaCompat_getPointer(L, -1);
        varg.pdispVal->AddRef();
        varg.vt = VT_DISPATCH;
      }
      else if(luaCompat_checkTagToCom(L, luaval))
      {
        lua_remove(L, -2);
        tLuaCOM* lcom;
        switch(lua_type(L, -1)) {
          case LUA_TTABLE:
            lcom = convert_table(L, luaval);
            break;
          case LUA_TFUNCTION:
            lua_pushvalue(L, luaval);
            lua_pushnumber(L, type);
            lua_call(L, 2, 1);
            lcom = from_lua(L, lua_gettop(L));
            lua_pop(L, 1);
            break;
          default:
            lua_pop(L,1);
            TYPECONV_ERROR("Invalid conversion function.");
        }
        if(!lcom) TYPECONV_ERROR("Conversion function failed.");
        varg.pdispVal = lcom->GetIDispatch();
        varg.pdispVal->AddRef();
        varg.vt = VT_DISPATCH;
      }
      else {
        lua_pushstring(L, "Type");
        lua_gettable(L, luaval);
        if(!lua_isnil(L, -1)) { // Table describes a variant
          tStringBuffer vtype(lua_tostring(L, -1));
          lua_pushstring(L, "Value");
          lua_gettable(L, luaval);
          if(strcmp(vtype, "double") == 0) {
            varg.vt = VT_R8;
            varg.dblVal = lua_tonumber(L, -1);
          } else if(strcmp(vtype, "array") == 0) {
            safearray_lua2com(L, lua_gettop(L), varg, VT_VARIANT);
          } else if(strncmp(vtype, "array of ", strlen("array of ")) == 0) {
            VARTYPE vtarr = VT_EMPTY;
            const char *varrtype = vtype + strlen("array of ");
            for (int i=0; com_type_names[i]; i++) {
              if (strcmp(com_type_names[i], varrtype) == 0) {
                vtarr = com_types[i];
                break;
              }
            }
            safearray_lua2com(L, lua_gettop(L), varg, vtarr);
          } else if(strcmp(vtype, "float") == 0) {
            varg.vt = VT_R4;
            varg.fltVal = (float)lua_tonumber(L, -1);
          } else if(strcmp(vtype, "currency") == 0) {
            varg.vt = VT_R8;
            varg.dblVal = lua_tonumber(L, -1);
            VariantChangeType(&varg, &varg, 0, VT_CY);
          } else if(strcmp(vtype, "decimal") == 0) {
            varg.vt = VT_R8;
            varg.dblVal = lua_tonumber(L, -1);
            VariantChangeType(&varg, &varg, 0, VT_DECIMAL);
          } else if(strcmp(vtype, "int8") == 0) {
            varg.vt = VT_I8;
            varg.llVal = (LONGLONG)lua_tonumber(L, -1);
          } else if(strcmp(vtype, "uint8") == 0) {
            varg.vt = VT_UI8;
            varg.ullVal = (ULONGLONG)lua_tonumber(L, -1);
          } else if(strcmp(vtype, "int4") == 0) {
            varg.vt = VT_I4;
            varg.lVal = (int)lua_tonumber(L, -1);
          } else if(strcmp(vtype, "uint4") == 0) {
            varg.vt = VT_UI4;
            varg.ulVal = (unsigned int)lua_tonumber(L, -1);
          } else if(strcmp(vtype, "int2") == 0) {
            varg.vt = VT_I2;
            varg.iVal = (short)lua_tonumber(L, -1);
          } else if(strcmp(vtype, "uint2") == 0) {
            varg.vt = VT_UI2;
            varg.uiVal = (unsigned short)lua_tonumber(L, -1);
          } else if(strcmp(vtype, "int1") == 0) {
            varg.vt = VT_I1;
            varg.cVal = (char)lua_tonumber(L, -1);
          } else if(strcmp(vtype, "uint1") == 0) {
            varg.vt = VT_UI1;
            varg.bVal = (unsigned char)lua_tonumber(L, -1);
          } else if(strcmp(vtype, "int") == 0) {
            varg.vt = VT_INT;
            varg.intVal = (int)lua_tonumber(L, -1);
          } else if(strcmp(vtype, "uint") == 0) {
            varg.vt = VT_UINT;
            varg.uintVal = (unsigned int)lua_tonumber(L, -1);
          } else if(strcmp(vtype, "string") == 0) {
            varg.vt = VT_BSTR;
            varg.bstrVal = tUtil::string2bstr(lua_tostring(L, -1));
          } else if(strcmp(vtype, "null") == 0) {
            varg.vt = VT_NULL;
          } else if(strcmp(vtype, "error") == 0) {
            varg.vt = VT_ERROR;
            varg.scode = (SCODE)lua_tonumber(L, -1);
          } else if(strcmp(vtype, "bool") == 0) {
            varg.vt = VT_BOOL;
            varg.boolVal = lua_toboolean(L, -1) ? VARIANT_TRUE : VARIANT_FALSE;
          } else {
            varg.vt = VT_EMPTY;
          }
        }
        else { // maybe a date?
          int isdate = 0;
          SYSTEMTIME date;
          lua_pushstring(L, "Day");
          lua_gettable(L, luaval);
          isdate = isdate || !lua_isnil(L, -1);
          date.wDay = (WORD)lua_tonumber(L, -1);
          lua_pop(L, 1);
          lua_pushstring(L, "DayOfWeek");
          lua_gettable(L, luaval);
          isdate = isdate || !lua_isnil(L, -1);
          date.wDayOfWeek = (WORD)lua_tonumber(L, -1);
          lua_pop(L, 1);
          lua_pushstring(L, "Month");
          lua_gettable(L, luaval);
          isdate = isdate || !lua_isnil(L, -1);
          date.wMonth = (WORD)lua_tonumber(L, -1);
          lua_pop(L, 1);
          lua_pushstring(L, "Year");
          lua_gettable(L, luaval);
          isdate = isdate || !lua_isnil(L, -1);
          date.wYear = (WORD)lua_tonumber(L, -1);
          lua_pop(L, 1);
          lua_pushstring(L, "Hour");
          lua_gettable(L, luaval);
          isdate = isdate || !lua_isnil(L, -1);
          date.wHour = (WORD)lua_tonumber(L, -1);
          lua_pop(L, 1);
          lua_pushstring(L, "Minute");
          lua_gettable(L, luaval);
          isdate = isdate || !lua_isnil(L, -1);
          date.wMinute = (WORD)lua_tonumber(L, -1);
          lua_pop(L, 1);
          lua_pushstring(L, "Second");
          lua_gettable(L, luaval);
          isdate = isdate || !lua_isnil(L, -1);
          date.wSecond = (WORD)lua_tonumber(L, -1);
          lua_pop(L, 1);
          lua_pushstring(L, "Milliseconds");
          lua_gettable(L, luaval);
          isdate = isdate || !lua_isnil(L, -1);
          date.wMilliseconds = (WORD)lua_tonumber(L, -1);
          lua_pop(L, 1);
          if(isdate) {
            varg.vt = VT_DATE;
            SystemTimeToVariantTime(&date, &varg.date);
          } else safearray_lua2com(L, luaval, varg, VT_VARIANT);
        }
      }
    }
      break;
    }

    break;

  case LUA_TBOOLEAN:
    varg.vt = VT_BOOL;
    varg.boolVal = lua_toboolean(L, luaval) ? VARIANT_TRUE : VARIANT_FALSE;
    break;


  case LUA_TUSERDATA:
    if(isIUnknown(L, luaval)) // is an IUnknown?
    {
      varg.vt = VT_UNKNOWN;
      varg.punkVal = (IUnknown *)*(void **)lua_touserdata(L, luaval);
      varg.punkVal->AddRef();
    }
    break;

  case LUA_TNIL:
    varg.vt = VT_EMPTY;
    break;

  case LUA_TNONE:
  default:
    TYPECONV_ERROR("No Lua value to convert.");
    break;
  }
  LUASTACK_DOCLEAN(L, 0);
}

/**
 Puts value from Lua stack into VARIANTARG.
 NOTE: uses VARTYPE funcdesc->elemdescFunc.tdesc.vt
 (IMPROVE: don't need to pass entire FUNCDESC.  Not using other parameters of TYPEDESC?)
*/
bool tLuaCOMTypeHandler::setRetval(lua_State* L,
                                   const FUNCDESC * funcdesc,
                                   stkIndex luaval,
                                   VARIANTARG * pvarg_retval)
{
  VARIANT varg;
  VariantInit(&varg);

  if(funcdesc->elemdescFunc.tdesc.vt != VT_VOID)
  {
    lua2com(L, luaval, varg, funcdesc->elemdescFunc.tdesc.vt);

    initByRefParam(pvarg_retval, VT_VARIANT);
    toByRefParam(varg, pvarg_retval);
  }
  else
    return false;

  return true;
}


/**
 Pushes onto the Lua stack any "out" parameters stored in the `dispparams` list.
 Returns the number of values pushed.
 The optional `pfuncdesc` (may be nullptr) helps interpret whether parameters
 are "out" parameters.
*/
int tLuaCOMTypeHandler::pushOutValues(lua_State* L, const DISPPARAMS& dispparams, const FUNCDESC* pfuncdesc)
{
  CHECKPRECOND(!pfuncdesc || dispparams.cArgs == pfuncdesc->cParams);

  // Iterate over parameters.
  int num_pushed_values = 0;
  for(unsigned i = 0; i < dispparams.cArgs; i++)
  {
    VARIANTARG& varg = dispparams.rgvarg[dispparams.cArgs - i - 1];
    bool isout = (pfuncdesc &&
      ((pfuncdesc->lprgelemdescParam[i].paramdesc.wParamFlags & PARAMFLAG_FOUT) != 0)) ||
       ((varg.vt & VT_BYREF) != 0);
    if (isout)
    {
      com2lua(L, varg);
      num_pushed_values++;
    }
  }

  return num_pushed_values;
}


/**
  Destroys DISPPARAMS list.
*/
void tLuaCOMTypeHandler::releaseVariants(DISPPARAMS *pDispParams)
{
  VARIANTARG* &vargs = pDispParams->rgvarg;
  if (vargs != NULL)
  {
    // Release memory.
    for (unsigned int i = 0; i < pDispParams->cArgs; i++)
    {
      releaseVariant(&vargs[i]);
    }
    delete [] vargs;
    
    // Optional zeroing of memory (not necessary).
    vargs = NULL;
    pDispParams->cArgs = 0;
    // pDispParams->rgdispidNamedArgs = NULL; (this memory is not currently owned by object).
    // pDispParams->cNamedArgs = 0;
  }
}



/**
 Fills DISPPARAMS structure from values on Lua stack (`params`).
 Optional `pfuncdesc` helps define proper conversions.
 invkind if the type of function call the parameters are for.
*/
void tLuaCOMTypeHandler::fillDispParams(lua_State* L,
                                        DISPPARAMS& rDispParams,
                                        FUNCDESC * pfuncdesc,
                                        tLuaObjList params,
                                        int invkind)
{
  // initializes DISPPARAMS
  rDispParams.cArgs = 0;
  rDispParams.cNamedArgs = 0;
  rDispParams.rgvarg = NULL;
  rDispParams.rgdispidNamedArgs = NULL;

  // if we know that this function does not received
  // parameters, stop here
  if (pfuncdesc && pfuncdesc->cParams == 0)
    return;

  static DISPID dispidNamed = DISPID_PROPERTYPUT;  // effectively const

  // references to simplify code
  unsigned int& r_cArgs = rDispParams.cArgs;
  VARIANTARG* &r_rgvarg  = rDispParams.rgvarg;

  // propertyput particular case
  if(invkind == DISPATCH_PROPERTYPUT ||
     invkind == DISPATCH_PROPERTYPUTREF)
  {
    rDispParams.cNamedArgs = 1;
    rDispParams.rgdispidNamedArgs = &dispidNamed;
  }

  r_cArgs = 0; // starts empty
  long lua_args = 0;
  long max_idl_params = 0;


  // first attemp to determine the size of DISPPARAMS
  unsigned int max_params = 0;

  if(pfuncdesc)
    max_params = pfuncdesc->cParams;
  else
    max_params = params.getNumParams();

  // if this function can receive a variable
  // number of arguments, then the last
  // formal parameter is a safearray of these
  // arguments and must be treated separately.
  // Following the documentation, we should pass
  // a safearray containing the remaining parameters.
  // As this didn't work with MSHTML.HTMLDocument.WriteLn,
  // we choose to do the same as VBScript: pass all
  // the remaining parameters separately.
  // The code to do this the "documented" way
  // is commented some lines of code below.
  if(pfuncdesc && pfuncdesc->cParamsOpt == -1)
  {
    assert(max_params >= 1);

    // The maximum number of parameters now is
    // bounded by the size of the parameter list,
    // if it is larger than the formal parameter
    // list
    if(params.getNumParams() > pfuncdesc->cParams)
      max_params = params.getNumParams();
    else
      max_params = pfuncdesc->cParams;

    // We ignore the last parameter (the safearray
    // of variants), as we won't use it
    max_idl_params = pfuncdesc->cParams - 1;
  }
  else
    max_idl_params = max_params;

  // creates array of VARIANTs
  r_rgvarg = new VARIANTARG[max_params];

  // Iterate the array lprgelemdescParam, getting parameters
  // from the Lua table.

  VARIANT defaultValue;
  VariantInit(&defaultValue);

  try
  {
    for (unsigned short i = 0; i < max_params; i++)
    {
      // default values
      bool byref      = true;
      bool hasdefault = false;
      bool array      = false;
      bool optin      = false;
      VARTYPE type = VT_VARIANT;
      VARTYPE array_type = VT_EMPTY;

      VariantInit(&r_rgvarg[r_cArgs]);

      // processing that makes sense when there is type info available
      if(pfuncdesc && i < max_idl_params)
      {
        PARAMDESC paramdesc = pfuncdesc->lprgelemdescParam[i].paramdesc;
        const TYPEDESC tdesc =
          processTYPEDESC(m_typeinfo, pfuncdesc->lprgelemdescParam[i].tdesc);

        // stores type of the expected value
        type = tdesc.vt & ~VT_BYREF;

        // tests whether an array is expected,
        // storing type of array elements if so.
        if(tdesc.vt & VT_ARRAY)
        {
          array = true;
          array_type = type & ~VT_ARRAY;
        }

        // ignores out parameters
        if(!(paramdesc.wParamFlags &  PARAMFLAG_FIN) &&
            (paramdesc.wParamFlags != PARAMFLAG_NONE) &&
            !(paramdesc.wParamFlags & PARAMFLAG_FOPT))
        {
          // If server expects a safearray, creates an
          // empty one to avoid problems
          if(array)
          {
            r_rgvarg[r_cArgs].vt = array_type | VT_ARRAY | VT_BYREF;
            r_rgvarg[r_cArgs].pparray =
              (SAFEARRAY**) CoTaskMemAlloc(sizeof(SAFEARRAY*));
            *r_rgvarg[r_cArgs].pparray = NULL;
          } else {
            initByRefParam(&r_rgvarg[r_cArgs], type);
          }

          r_cArgs++;
          continue;
        }
        else if(paramdesc.wParamFlags & PARAMFLAG_FOUT)
        {
          // assumes that it is an in/out parameter
          byref = true;
        }
        else if(tdesc.vt & VT_BYREF) // tdesc overrides paramflags
          byref = true;
        else // in param
          byref = false;

        // detects if optional output parameter
        optin = (paramdesc.wParamFlags & PARAMFLAG_FOPT) != 0 &&
                (paramdesc.wParamFlags & PARAMFLAG_FIN) != 0;

        // deals with default values (if any)
        if(paramdesc.wParamFlags & PARAMFLAG_FHASDEFAULT)
        {
          hasdefault = true;
          VariantCopy(
            &defaultValue,
            &paramdesc.pparamdescex->varDefaultValue);
        }
      }

      // gets value from Lua
      stkIndex val = params.getparam(lua_args);

      // Converts to VARIANT
      VARIANT var;
      VariantInit(&var);

      if(val != 0 && lua_type(L, val) != LUA_TNONE && !lua_isnil(L, val))
      {
        if(array)
          safearray_lua2com(L, val, var, array_type);
        else
          lua2com(L, val, var, type);
      }
      else if(hasdefault)
      {
        VariantCopy(&var, &defaultValue);
        VariantClear(&defaultValue);
      }
      else if(optin)
      {
        // assumes that a parameter is expected but has not been found
        var.vt = VT_ERROR;
        var.scode = DISP_E_PARAMNOTFOUND;
        byref = false;
      }
      // else if(optout) do nothing (handled below instead)

      if(!byref)
      {
        VariantCopy(&r_rgvarg[i], &var);
        VariantClear(&var);
      }
      else // byref (note: optout => byref)
      {
        initByRefParam(&r_rgvarg[i], type, true);
        toByRefParam(var, &r_rgvarg[i]);
      }

      r_cArgs++;
      lua_args++;
    }

    #if 0
    // deals with vararg functions following
    // vararg documentation
    if(pfuncdesc && pfuncdesc->cParamsOpt == -1)
    {
      safearray_lua2com(
        params.getparam(lua_args),
        r_rgvarg[r_cArgs],
        VT_VARIANT,
        true);

      r_cArgs++;
    }
    #endif
  }
  catch(class tLuaCOMException& e)
  {
    UNUSED(e);

    delete[] r_rgvarg;
    r_rgvarg = NULL;
    throw;
  }

  // inverts parameters order to match
  // what is expected by Automation
  if(r_cArgs > 0)
  {
    VARIANTARG temp;

    for(unsigned short i = 0; i < r_cArgs/2; i++)
    {
      temp = r_rgvarg[i];
      r_rgvarg[i] = r_rgvarg[r_cArgs - i - 1];
      r_rgvarg[r_cArgs - i - 1] = temp;
    }
  }

  return;
}


/**
  Pushes onto Lua stack representations of non-"out" parameters of DISPPARAMS.
  This can be used to marshall arguments of a COM method into a Lua function.
*/
void tLuaCOMTypeHandler::pushLuaArgs(lua_State* L,
                                     DISPPARAMS* pDispParams,
                                     const ELEMDESC* pElemDesc)
{
  HRESULT hr = S_OK;

  const unsigned int first_named_param =
    pDispParams->cArgs - pDispParams->cNamedArgs;

  VARIANT var;
  VariantInit(&var);

  for(unsigned int arg = 0; arg < pDispParams->cArgs; arg++)
  {
    const USHORT& wParamFlags = pElemDesc[arg].paramdesc.wParamFlags;
    const TYPEDESC tdesc = processTYPEDESC(m_typeinfo, pElemDesc[arg].tdesc);

    // if parameter is not an out param, convert it to Lua
    if(wParamFlags & PARAMFLAG_FIN || wParamFlags == PARAMFLAG_NONE )
    {

      // First we have to find the right index in the
      // rgvarg array, as we must deal with positional and
      // named parameters

      int index = -1;
      if(arg < first_named_param) // still inside positional parameters
        index = pDispParams->cArgs - arg - 1;
      else // now we are dealing with named parameters
      {
        // tries to find a named param for the current position
        for(unsigned int i = 0; i < pDispParams->cNamedArgs; i++)
        {
          if(pDispParams->rgdispidNamedArgs[i] == arg)
          {
            index = i;
            break;
          }
        }

        if(index == -1) // no corresponding named param found
        {
          lua_pushnil(L);
          continue;
        }
      }

      VARIANTARG& varg = pDispParams->rgvarg[index];

      // we assume that empty parameters with paramflags NONE
      // are out params
      if(varg.vt == VT_EMPTY && wParamFlags == PARAMFLAG_NONE)
        continue;


      if(varg.vt == VT_ERROR || varg.vt == VT_EMPTY)
      {
        // parameter has been omitted
        lua_pushnil(L);
        continue;
      }

      // removes indirections
      hr = VariantCopyInd(&var, &varg);
      CHK_COM_CODE(hr);

      // some type checks
      if((tdesc.vt & ~(VT_BYREF)) != VT_VARIANT &&
            V_ISARRAY(&tdesc) != V_ISARRAY(&var))
        CHK_COM_CODE(DISP_E_TYPEMISMATCH);
      else if(!V_ISARRAY(&var))
      {
        // coerces value to the expected scalar type
        Coerce(var, var, (tdesc.vt & ~VT_BYREF));
      }

      com2lua(L, var, (tdesc.vt & ~VT_BYREF) == VT_VARIANT);
      VariantClear(&var);
    }
  }
}


/**
  Copies representations of Lua values starting at stack index `outvalue`
  into only "out" parameters of DISPARAMS.
  FUNCDESC must be provided to assist in interpreting parameters.
*/
void tLuaCOMTypeHandler::setOutValues(lua_State* L,
                                      FUNCDESC * pFuncDesc,
                                      DISPPARAMS * pDispParams,
                                      stkIndex outvalue
                                      )
{
  CHECKPRECOND(outvalue > 0);

  const unsigned int num_args = pDispParams->cArgs;
  const unsigned int first_named_param = pDispParams->cArgs - pDispParams->cNamedArgs;
  unsigned int arg = 0;

  VARIANT var;
  VariantInit(&var);

  // Search return values of out parameters
  for(arg = 0; arg < num_args; arg++)
  {
    // checks whether there are more return values to map
    // to out parameters
    if(lua_type(L, outvalue) == LUA_TNONE)
      break;

    // alias
    const TYPEDESC tdesc =
      processTYPEDESC(m_typeinfo, pFuncDesc->lprgelemdescParam[arg].tdesc);

    // tests whether this parameters is an out parameter
    if(V_ISBYREF(&tdesc))
    {
      // tries to find the right position in the rgvarg array,
      // when using named args
      unsigned int index = (unsigned int)-1;

      if(arg < first_named_param) // still inside positional parameters
        index = pDispParams->cArgs - arg - 1;
      else // now we are dealing with named parameters
      {
        // tries to find a named param for the current position
        for(unsigned int i = 0; i < pDispParams->cNamedArgs; i++)
        {
          if(pDispParams->rgdispidNamedArgs[i] == arg)
          {
            index = i;
            break;
          }
        }

        if(index == -1)
        {
          // no corresponding named param found, so we must skip this one
          outvalue++;
          continue;
        }
      }

      // alias
      VARIANTARG& varg_orig = pDispParams->rgvarg[index];

      // allocates if pure out value
      USHORT paramFlags = pFuncDesc->lprgelemdescParam[arg].paramdesc.wParamFlags;
      if(!(paramFlags &  PARAMFLAG_FIN) &&
          (paramFlags != PARAMFLAG_NONE) &&
          !(paramFlags & PARAMFLAG_FOPT))
        initByRefParam(&varg_orig, tdesc.vt & ~VT_BYREF);

      // does indirection in the case of VARIANTs
      if(varg_orig.vt == (VT_VARIANT | VT_BYREF))
      {
        VARIANTARG& varg = *varg_orig.pvarVal;

        if(varg.vt & VT_ARRAY)
          safearray_lua2com(L, outvalue, var, varg.vt & ~(VT_BYREF | VT_ARRAY));
        else
          lua2com(L, outvalue, var, (varg.vt & ~VT_BYREF));

        if(varg.vt & VT_BYREF)
          toByRefParam(var, &varg);
        else
          VariantCopy(&varg, &var);
      }
      else
      {
        VARIANTARG& varg = varg_orig;

        if(varg.vt == VT_ERROR || varg.vt == VT_EMPTY)
          initByRefParam(&varg, tdesc.vt & ~VT_BYREF);

        if(varg.vt & VT_ARRAY)
          safearray_lua2com(L, outvalue, var, varg.vt & ~(VT_BYREF | VT_ARRAY));
        else
          lua2com(L, outvalue, var, (varg.vt & ~VT_BYREF));

        toByRefParam(var, &varg);
      }

      outvalue++;
    }
  }
}


//
// Conversion of SAFEARRAYs to/from Lua tables.
//

// auxillary functions for safearray_lua2com




SAFEARRAYBOUND* tLuaCOMTypeHandler::getRightOrderedBounds(
    SAFEARRAYBOUND *bounds,
    unsigned long num_dimensions)
{
  SAFEARRAYBOUND* new_bounds = new SAFEARRAYBOUND[num_dimensions];

  for(unsigned long i = 0; i < num_dimensions; i++) {
    new_bounds[i] = bounds[num_dimensions - i - 1];
  }

  return new_bounds;
}


/**
  Puts copy of VARIANT coerced to type `vt` to location `indices` in SAFEARRAY.
*/
void tLuaCOMTypeHandler::put_in_array(SAFEARRAY* safearray,
                         VARIANT var,
                         long* indices,
                         VARTYPE vt
                         )
{
  HRESULT hr = S_OK;

  // converts to the right type
  Coerce(var, var, vt);

  // does a copy to avoid problems
  VARIANT var_value;
  VariantInit(&var_value);
  VariantCopy(&var_value, &var);

  switch(vt)
  {
  case VT_I2:
    hr = SafeArrayPutElement(safearray, indices, &var_value.iVal);
    break;

  case VT_I4:
    hr = SafeArrayPutElement(safearray, indices, &var_value.lVal);
    break;

  case VT_R4:
    hr = SafeArrayPutElement(safearray, indices, &var_value.fltVal);
    break;

  case VT_R8:
    hr = SafeArrayPutElement(safearray, indices, &var_value.dblVal);
    break;

  case VT_CY:
    hr = SafeArrayPutElement(safearray, indices, &var_value.cyVal);
    break;

  case VT_DATE:
    hr = SafeArrayPutElement(safearray, indices, &var_value.date);
    break;

  case VT_BSTR:
    // BSTR already a pointer
    hr = SafeArrayPutElement(safearray, indices, var_value.bstrVal);
    break;

  case VT_DISPATCH:
    // IDispatch already is a pointer (no indirection need)
    hr = SafeArrayPutElement(safearray, indices, var_value.pdispVal);
    VariantClear(&var_value);
    break;

  case VT_ERROR:
    hr = SafeArrayPutElement(safearray, indices, &var_value.scode);
    break;

  case VT_BOOL:
    hr = SafeArrayPutElement(safearray, indices, &var_value.boolVal);
    break;

  case VT_VARIANT:
    hr = SafeArrayPutElement(safearray, indices, &var_value);
    break;

  case VT_UNKNOWN:
    // see IDispatch
    hr = SafeArrayPutElement(safearray, indices, var_value.punkVal);
    VariantClear(&var_value);
    break;

  case VT_DECIMAL:
    hr = SafeArrayPutElement(safearray, indices, &var_value.decVal);
    break;

  case VT_UI1:
    hr = SafeArrayPutElement(safearray, indices, &var_value.bVal);
    break;

  case VT_INT:
    hr = SafeArrayPutElement(safearray, indices, &var_value.intVal);
    break;

  case VT_ARRAY:
    TYPECONV_ERROR("SAFEARRAY of SAFEARRAYS not allowed");
    break;

  case VT_I1:
  case VT_UI2:
  case VT_UI4:
  case VT_UINT:
  default:
    TYPECONV_ERROR("Type not compatible with automation.");
    break;
  }

  CHK_COM_CODE(hr);
}

stkIndex tLuaCOMTypeHandler::get_from_array(lua_State* L,
                                            SAFEARRAY* safearray,
                                            long *indices,
                                            const VARTYPE& vt
                                            )
{
  VARIANTARG varg;
  void *pv = NULL;
  if(vt == VT_VARIANT)
  {
    pv = &varg;
  }
  else
  {
    VariantInit(&varg);
    varg.vt = vt;

    // e' uma union, tanto faz de quem pego o ponteiro
    pv = (void *) &varg.dblVal;
  }

  HRESULT hr = SafeArrayGetElement(safearray, indices, pv);

  if(FAILED(hr)) {
    if (vt == VT_VARIANT) {
      VariantClear(&varg);
    }
    LUACOM_EXCEPTION(INTERNAL_ERROR);
  }

  com2lua(L, varg);

  if (vt == VT_VARIANT) {
    VariantClear(&varg);
  }

  return lua_gettop(L);
}



bool tLuaCOMTypeHandler::inc_indices(long *indices,
                        SAFEARRAYBOUND *bounds,
                        unsigned long dimensions
                        )
{
  long j = dimensions - 1;

  indices[j]++;

  while(
    (indices[j] >= (long) bounds[j].cElements + bounds[j].lLbound) &&
    (j >= 0)
    )
  {
    indices[j] = bounds[j].lLbound;
    j--;
    if(j < 0 && indices[0] == (long)bounds[0].lLbound) {
      return false;
    }
    indices[j]++;
  }
  return true;
}


/**
 Converts Lua table to SAFEARRAY.
*/
void tLuaCOMTypeHandler::safearray_lua2com(lua_State* L,
                                           stkIndex luaval,
                                           VARIANTARG& varg,
                                           VARTYPE vt,
                                           bool from_stack)
{
  LUASTACK_SET(L);

  HRESULT hr = S_OK;

  tLuaVector luavector;

  long stack_bottom = lua_gettop(L);

  if(!from_stack)
  {
    // when trying to convert a string to a safearray
    // uses a specific method
    if(lua_type(L, luaval) == LUA_TSTRING)
    {
      string2safearray(
        lua_tostring(L, luaval),
        lua_strlen(L, luaval),
        varg
        );
      return;
    }

    // creates a luavector based on the table passed
    luavector.InitVectorFromTable(L, luaval);
  }
  else // trying to create an array from the stack
  {
    luavector.InitVectorFromStack(L, luaval);
  }

  long stack_top = lua_gettop(L);

  if(luavector.getLength() == 0) {
    // returns an empty vector
    varg.vt = vt | VT_ARRAY;
    varg.parray = SafeArrayCreateVector(vt, 0, 0);
    varg.parray = NULL;

    LUASTACK_CLEAN(L, 0);
    return;
  }

  // create variables
  unsigned long i = 0;
  const unsigned long dimensions = luavector.get_Dimensions();
  SAFEARRAYBOUND *bounds = new SAFEARRAYBOUND[dimensions];
  SAFEARRAY *safearray = NULL;

  VARIANTARG var_value;
  VariantInit(&var_value);

  // initialize dimensions
  for(i = 0; i < dimensions; i++)
  {
    bounds[dimensions - i - 1].lLbound = 0;
    bounds[dimensions - i - 1].cElements = luavector.get_Nth_Dimension(dimensions - i);
  }


  // create array
  safearray = SafeArrayCreate(vt, dimensions, bounds);

  long *indices = NULL;

  try
  {
    CHECK(safearray, INTERNAL_ERROR);

    // initialize indices
    indices = new long[dimensions];

    for(i = 0; i < dimensions; i++)
      indices[i] = bounds[i].lLbound;

    unsigned long dimension = indices[0];
    // copy elements one-by-one
    do
    {
      // obtain value
      luaval = luavector.getindex(indices, dimensions, bounds);

      // convert
      lua2com(L, luaval, var_value, vt);

      // put in array
      put_in_array(safearray, var_value, indices, vt);

      VariantClear(&var_value);
    }
    while(inc_indices(indices, bounds, dimensions)); // incrementa indices
  }
  catch(class tLuaCOMException&)
  {
    delete[] bounds;
    delete[] indices;
    SafeArrayDestroy(safearray);

    while(stack_top > stack_bottom)
    {
      lua_remove(L, stack_top);
      stack_top--;
    }

    throw;
  }


  // fill varg
  varg.vt = vt | VT_ARRAY;
  varg.parray = safearray;


  delete[] bounds;
  delete[] indices;

  while(stack_top > stack_bottom)
  {
    lua_remove(L, stack_top);
    stack_top--;
  }

  LUASTACK_CLEAN(L, 0);

  return;
}

/**
 Converts array of bytes to SAFEARRAY (of VT_UI1).
 Array is memory block [str, str+len).
*/
void tLuaCOMTypeHandler::string2safearray(const char* str, size_t len, VARIANTARG& varg)
{
  HRESULT hr = S_OK;

  // create array
  SAFEARRAY *safearray = SafeArrayCreateVector(VT_UI1, 0, (ULONG)len);
  CHECK(safearray, INTERNAL_ERROR);

  void * buffer = NULL;
  hr = SafeArrayAccessData(safearray,&buffer);
  if(FAILED(hr))
    LUACOM_EXCEPTION(INTERNAL_ERROR);
  if (buffer != NULL)
     memcpy(buffer,str,len);
  SafeArrayUnaccessData(safearray);

  // fill varg
  varg.vt = VT_UI1 | VT_ARRAY;
  varg.parray = safearray;
}

/**
 Converts SAFEARRAY (of VT_UI1) to array of bytes.
 Assumes SAFEARRAY is one-dimensional.
*/
void tLuaCOMTypeHandler::safearray2string(lua_State* L, VARIANTARG & varg)
{
  CHECKPRECOND(varg.vt & (VT_ARRAY | VT_UI1));
  CHECKPRECOND(varg.parray->cDims == 1);

  HRESULT hr = S_OK;

  long size = varg.parray->rgsabound[0].cElements;

  void * buffer = NULL;
  hr = SafeArrayAccessData(varg.parray, &buffer);
  CHK_COM_CODE(hr);

  lua_pushlstring(L, (char*) buffer, size);

  SafeArrayUnaccessData(varg.parray);
}

/**
 Returns an array containing the number of elements in each dimension
 of a SAFEARRAY.
 Callee owns memory returned.
*/
long* tLuaCOMTypeHandler::dimensionsFromBounds(SAFEARRAYBOUND* bounds,
                                                long num_bounds
                                                )
{
  long* dimensions = new long[num_bounds];

  for(int i = 0; i < num_bounds; i++)
  {
    dimensions[i] = bounds[i].cElements;
  }

  return dimensions;
}



void tLuaCOMTypeHandler::safearray_com2lua(lua_State* L, VARIANTARG & varg)
{
  CHECK(varg.vt & VT_ARRAY, PARAMETER_OUT_OF_RANGE);

  long *indices           = NULL;
  SAFEARRAYBOUND* bounds  = NULL;

  try
  {
    SAFEARRAY* safearray = varg.parray;

    // check for NULL or empty array (is this enough?)
    // returns an empty table in both cases, for consistency (also eases client code)
    if(safearray == NULL) {
      lua_newtable(L);
      return;
    }

    // get dimensions
    const int num_dimensions = SafeArrayGetDim(safearray);
    // checks for empty array, must be done in the 'first' dimension
    if(safearray->rgsabound[num_dimensions - 1].cElements == 0) {
      lua_newtable(L);
      return;
    }

    bounds = getRightOrderedBounds
      (
      safearray->rgsabound,
      num_dimensions
      );


    tLuaVector luavector;

    {
      long *dimensions = dimensionsFromBounds(bounds, num_dimensions);

      try
      {
        luavector.InitVectorFromDimensions(dimensions, num_dimensions);
      }
      catch(class tLuaCOMException&)
      {
        delete[] dimensions;
        throw;
      }

      delete[] dimensions;
    }

    // initializes indices
    indices = new long[num_dimensions];

    for(int i = 0; i < num_dimensions; i++)
      indices[i] = bounds[i].lLbound;

    // gets array data type
    VARTYPE vt;
    HRESULT hr = SafeArrayGetVartype(safearray, &vt);
    if (FAILED(hr)) {
      delete[] bounds;
      delete[] indices;
      LUACOM_EXCEPTION(INTERNAL_ERROR);
    }

    // holds index to Lua objects
    stkIndex luaval = 0;

    // saves current stack position
    stkIndex stacktop = lua_gettop(L);

    // allocates enough stack room
    lua_checkstack(L, luavector.size()*2);

    // copy elements one-by-one
    do
    {
      // get from array
      luaval = get_from_array(L, safearray, indices, vt);

      luavector.setindex(L, luaval, indices, num_dimensions, bounds);
    }
    while(inc_indices(indices, bounds, num_dimensions)); // incrementa indices

    // tries to create Lua table on top of stack
    bool succeeded = luavector.CreateTable(L);

    // remove temporary objects
    stkIndex clean_until = lua_gettop(L);

    if(succeeded)
      clean_until--; // doesn't clean created table!

    while(clean_until > stacktop)
    {
      lua_remove(L, clean_until);
      clean_until--;
    }
  }
  catch(class tLuaCOMException&)
  {
    delete[] bounds;
    delete[] indices;
    throw;
  }

  delete[] bounds;
  delete[] indices;

  return;
}


tLuaCOM * tLuaCOMTypeHandler::from_lua(lua_State* L, int index)
{
  return (tLuaCOM *) LuaBeans::from_lua(L, index);
}

TYPEDESC tLuaCOMTypeHandler::processPTR(ITypeInfo* typeinfo,
                                        const TYPEDESC &tdesc)
{
  CHECKPRECOND(tdesc.vt == VT_PTR);

  TYPEDESC pointed_at;

  // continues indirection
  pointed_at.vt = tdesc.lptdesc->vt;
  pointed_at.lptdesc = tdesc.lptdesc->lptdesc;

  // removes aliases
  pointed_at = processAliases(typeinfo, pointed_at);

  // if the referenced type is userdefined, gets its definition
  bool userdef = false;

  if(pointed_at.vt == VT_USERDEFINED)
  {
    userdef = true;
    pointed_at = processUSERDEFINED(typeinfo, pointed_at);
  }

  if(userdef == true &&
     (pointed_at.vt == VT_DISPATCH || pointed_at.vt == VT_UNKNOWN))
  {
    // does nothing, because it's a VT_USERDEFINED TYPEDESC that
    // describes an interface that inherits from IDispatch.
    // Pointers (that is, single indirection) to IDispatch
    // are always VT_DISPATCH.
  }
  else if(pointed_at.vt == VT_PTR)
  {
    // continues indirection
    pointed_at = processPTR(typeinfo, pointed_at);

    // We arrive here if the TYPEDESC describes a
    // pointer to a pointer. This only happens
    // when we are refencing interfaces. Since
    // interfaces are always refenced as pointers,
    // it looks like a single indirection

    pointed_at.vt |= VT_BYREF;
  }
  else if(pointed_at.vt == VT_SAFEARRAY)
  {
    pointed_at = processSAFEARRAY(typeinfo, pointed_at);
    pointed_at.vt |= VT_BYREF;
  }
  else // other types under a VT_PTR are just BYREF
  {
    pointed_at.vt |= VT_BYREF;
  }

  return pointed_at;
}

TYPEDESC tLuaCOMTypeHandler::processUSERDEFINED(ITypeInfo* typeinfo,
                                                const TYPEDESC &tdesc)
{
  HRESULT hr = S_OK;
  ITypeInfo *userdef = NULL;
  TYPEATTR *typeattr = NULL;
  TYPEDESC newtdesc;

  newtdesc.vt = 0;

  hr = typeinfo->GetRefTypeInfo(tdesc.hreftype, &userdef);

  if(FAILED(hr))
    TYPECONV_ERROR("Could not understand user-defined type");

  hr = userdef->GetTypeAttr(&typeattr);

  if(FAILED(hr))
  {
    userdef->Release();
    TYPECONV_ERROR("Could not understand user-defined type");
  }

  switch(typeattr->typekind)
  {
  case TKIND_ENUM:
    newtdesc.vt = VT_INT;
    break;

  case TKIND_DISPATCH:
    newtdesc.vt = VT_DISPATCH;
    break;

  case TKIND_ALIAS:
    // shouldn't arrive here: aliases must be removed via
    // processAliases()
    INTERNAL_ERROR();
    break;

  case TKIND_INTERFACE:
    newtdesc.vt = VT_UNKNOWN;
    break;

  case TKIND_UNION:
    TYPECONV_ERROR("Union type not supported!");
    break;

  case TKIND_COCLASS:
    newtdesc.vt = VT_UNKNOWN;
    break;

  case TKIND_RECORD:
    TYPECONV_ERROR("Record type not supported!");
    break;

  case TKIND_MODULE:
  case TKIND_MAX:
    TYPECONV_ERROR("TKIND_MODULE and TKIND_MAX not supported!");
    break;

  default:
    TYPECONV_ERROR("Unknown TYPEKIND on VT_USERDEFINED TYPEDESC");
    break;
  }

  userdef->ReleaseTypeAttr(typeattr);
  userdef->Release();

  return newtdesc;
}


/**
  Clears a VARIANT, releasing first the memory allocated
*/
void tLuaCOMTypeHandler::releaseVariant(VARIANTARG *pvarg, bool release_memory)
{
  if(pvarg->vt & VT_BYREF && pvarg->byref != NULL)
  {
    switch(pvarg->vt & ~VT_BYREF)
    {
    case VT_BSTR:
      SysFreeString(*pvarg->pbstrVal);
      break;

    case VT_DISPATCH:
      COM_RELEASE(*pvarg->ppdispVal);
      break;

    case VT_UNKNOWN:
      COM_RELEASE(*pvarg->ppunkVal);
      break;

    case VT_VARIANT:
      // a variant cannot contain another BYREF
      // so we just clear with VariantClear
      VariantClear(pvarg->pvarVal);
      break;

    }

    if(pvarg->vt & VT_ARRAY && *pvarg->pparray)
      SafeArrayDestroy(*pvarg->pparray);

    if(release_memory)
    {
      CoTaskMemFree(pvarg->byref);  // after CoTaskMemAlloc
      //char *p = (char*)pvarg->byref;
      //delete [] p;
      VariantClear(pvarg);
      //pvarg->byref = NULL;
      //pvarg->vt = VT_EMPTY;
    }
  }
  else
    VariantClear(pvarg);
}


/**
  Dereferences typedef's in type descriptions
*/
TYPEDESC tLuaCOMTypeHandler::processAliases(ITypeInfo* typeinfo,
                                            const TYPEDESC &tdesc)
{
  // if it's not a userdefined type, does nothing
  if(tdesc.vt != VT_USERDEFINED)
    return tdesc;

  HRESULT hr = S_OK;
  
  ITypeInfo *userdef = NULL;
  hr = typeinfo->GetRefTypeInfo(tdesc.hreftype, &userdef);

  if(FAILED(hr))
    TYPECONV_ERROR("Could not understand user-defined type");

  TYPEATTR *typeattr = NULL;
  hr = userdef->GetTypeAttr(&typeattr);

  if(FAILED(hr))
  {
    userdef->Release();
    TYPECONV_ERROR("Could not understand user-defined type");
  }

  TYPEDESC newtdesc;
  newtdesc.vt = 0;
  if(typeattr->typekind == TKIND_ALIAS)
  {
    newtdesc = typeattr->tdescAlias;
    newtdesc = processAliases(typeinfo, newtdesc);
  }
  else
    newtdesc = tdesc;

  userdef->ReleaseTypeAttr(typeattr);
  userdef->Release();

  return newtdesc;
}


TYPEDESC tLuaCOMTypeHandler::processTYPEDESC(ITypeInfo* typeinfo,
                                             TYPEDESC tdesc)
{
  // removes aliases
  tdesc = processAliases(typeinfo, tdesc);

  bool done = false;

  switch(tdesc.vt)
  {
  case VT_USERDEFINED:
    tdesc = processUSERDEFINED(typeinfo, tdesc);
    break;

  case VT_PTR:
    tdesc = processPTR(typeinfo, tdesc);
    break;

  case VT_SAFEARRAY:
    tdesc = processSAFEARRAY(typeinfo, tdesc);
  }

  CHECKPOSCOND(tdesc.vt != VT_USERDEFINED && tdesc.vt != VT_PTR);

  return tdesc;
}


/**
  Checks whether the Lua value is of tag LuaCOM_IUnknown.
*/
bool tLuaCOMTypeHandler::isIUnknown(lua_State* L, stkIndex value)
{
  lua_pushvalue(L, value);

  bool result = luaCompat_isOfType(L, MODULENAME, LCOM_IUNKNOWN_TYPENAME) != 0;

  lua_pop(L, 1);

  return result;
}


void tLuaCOMTypeHandler::pushIUnknown(lua_State* L, IUnknown *punk)
{
  luaCompat_pushTypeByName(L, MODULENAME, LCOM_IUNKNOWN_TYPENAME);
  luaCompat_newTypedObject(L, punk);
}


void tLuaCOMTypeHandler::initByRefParam(VARIANTARG* pvarg, VARTYPE vt, bool alloc_memory)
{
  CHECKPRECOND(!(vt & VT_BYREF));
  VariantClear(pvarg);

  pvarg->vt = vt | VT_BYREF;

  if(alloc_memory) {
    const long size = VariantSize(vt);

    pvarg->byref = (void *) CoTaskMemAlloc(size);

    //pvarg->byref = (void *) new char[size];

    // Initializes the allocated memory
    if(vt == VT_VARIANT)
      VariantInit(pvarg->pvarVal);
    else
      memset(pvarg->byref, 0, size);
  } else {
    pvarg->byref = NULL;
  }
}

void tLuaCOMTypeHandler::toByRefParam(VARIANT &var_source, VARIANTARG* pvarg_dest)
{
  CHECKPARAM(pvarg_dest);
  CHECKPRECOND(!(var_source.vt & VT_BYREF));

  // if a VARIANT ByRef is expected, just copies the value
  // to the ByRef VARIANT
  if(pvarg_dest->vt == (VT_VARIANT | VT_BYREF))
  {
    // we do a hard copy, to avoid increasing the reference count
    // objects by the use of VariantCopy
    memcpy(pvarg_dest->pvarVal, &var_source, sizeof(VARIANT));
    return;
  }

  VARTYPE vt_dest = pvarg_dest->vt & ~VT_BYREF;

  // tries to convert the value to the type expected in varg_dest
  if(!(vt_dest & VT_ARRAY))
  {
    HRESULT hr =
      VariantChangeType(&var_source, &var_source, 0, vt_dest);

    CHK_COM_CODE(hr);
  }

  // clears the old contents
  releaseVariant(pvarg_dest, false);

  pvarg_dest->vt = vt_dest | VT_BYREF;

  // does a hard copy of the memory contents,
  memcpy(pvarg_dest->byref, &var_source.byref, VariantSize(vt_dest));
}

TYPEDESC tLuaCOMTypeHandler::processSAFEARRAY(ITypeInfo* typeinfo,
                                              TYPEDESC &tdesc)
{
  CHECKPRECOND(tdesc.vt == VT_SAFEARRAY);

  // continues indirection
  TYPEDESC pointed_at;
  pointed_at = *tdesc.lptdesc;
  pointed_at = processTYPEDESC(typeinfo, pointed_at);
  pointed_at.vt |= VT_ARRAY;

  return pointed_at;
}


/**
 Returns the size in bytes of a VARIANT.
*/
long tLuaCOMTypeHandler::VariantSize(VARTYPE vt)
{
  if(vt & VT_ARRAY)
  {
    return sizeof(SAFEARRAY *);
  }

  switch(vt)
  {
  case VT_I2: return 2;
  case VT_I4: return 4;
  case VT_R4: return 4;
  case VT_R8: return 8;
  case VT_CY: return sizeof(CURRENCY);
  case VT_DATE:     return sizeof(DATE);
  case VT_BSTR:     return sizeof(BSTR);
  case VT_DISPATCH: return sizeof(IDispatch*);
  case VT_ERROR:    return sizeof(SCODE);
  case VT_BOOL:     return sizeof(VARIANT_BOOL);
  case VT_VARIANT:  return sizeof(VARIANT);
  case VT_UNKNOWN:  return sizeof(IUnknown*);
  case VT_DECIMAL:  return 16;
  case VT_UI1: case VT_I1: return 1;
  case VT_UI2: return 2;
  case VT_UI4: return 4;
  case VT_INT: return sizeof(int);
  case VT_UINT: return sizeof(unsigned int);
  default: TYPECONV_ERROR("Unknown type");
  }
}


/**
  Converts VARIANT to type `vt`.  Reads from `src` and writes to `dest`.
*/
void tLuaCOMTypeHandler::Coerce(VARIANTARG &dest, VARIANTARG src, VARTYPE vt)
{
  if(vt == VT_VARIANT)
  {
    return;  // do nothing. We already have a VARIANT
  }

  HRESULT hr = VariantChangeType(&dest, &src, 0, vt);
  CHK_COM_CODE(hr);
}
