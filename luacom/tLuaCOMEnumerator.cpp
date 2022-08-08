/**
  tLuaCOMEnumerator.cpp: tLuaCOMEnumerator class.
*/

#include "tLuaCOMEnumerator.h"

#include "tUtil.h"
#include "tCOMUtil.h"
#include "tLuaCOMException.h"

#include "LuaAux.h"

#include "luacom_internal.h"
#include "LuaCompat.h"


// initialization

const char *tLuaCOMEnumerator::type_name = "__ENUMERATOR_LUACOM_TYPE"; 
const char *tLuaCOMEnumerator::pointer_type_name = "__ENUMERATOR_POINTER_LUACOM_TYPE"; 
// the address of each static is a unique memory location used as the key to the lua registry
// see http://www.lua.org/pil/27.3.1.html
const char tLuaCOMEnumerator::module_name_key = 'k';

#define ENUMERATOR_FIELD "__TLUACOMENUMERATOR__internal"


tLuaCOMEnumerator::tLuaCOMEnumerator(IEnumVARIANT* pEV_param)
{
  CHECKPRECOND(pEV_param);
  
  // stores enumerator
  pEV.Attach(pEV_param);
  pEV->AddRef();

  typehandler = new tLuaCOMTypeHandler(NULL);
}

tLuaCOMEnumerator::~tLuaCOMEnumerator()
{
  delete typehandler;
}

int tLuaCOMEnumerator::index(lua_State *L)
{
  // pushes connection point
  lua_pushstring(L, ENUMERATOR_FIELD);
  lua_gettable(L, -3);

  // pushes method name
  lua_pushvalue(L, 2);

  // pushes closure to call all sinks
  lua_pushcclosure(L, call_method, 2);

  return 1;
}

void tLuaCOMEnumerator::push(lua_State* L)
{
  LUASTACK_SET(L);

  tStringBuffer module_name(tUtil::RegistryGetString(L, module_name_key));
  LUASTACK_DOCLEAN(L, 0);

  // creates table
  lua_newtable(L);
  luaCompat_pushTypeByName(L, 
    module_name, 
    tLuaCOMEnumerator::type_name);

  lua_setmetatable(L, -2);

  lua_pushstring(L, ENUMERATOR_FIELD);

  // pushes typed pointer
  luaCompat_pushTypeByName(L, 
    module_name, 
    tLuaCOMEnumerator::pointer_type_name);

  luaCompat_newTypedObject(L, this);

  // stores in the table
  lua_settable(L, -3);

  LUASTACK_CLEAN(L, 1);
}

int tLuaCOMEnumerator::garbagecollect(lua_State *L)
{
  LUASTACK_SET(L);

  // gets the enumerator
  tLuaCOMEnumerator* enumerator = 
    (tLuaCOMEnumerator*)*(void **)lua_touserdata(L, -1);

  delete enumerator;

  LUASTACK_CLEAN(L, 0);

  return 0;
}

void tLuaCOMEnumerator::registerLuaType(lua_State *L, const char *module)
{
  LUASTACK_SET(L);

  tStringBuffer module_name(module);
  // store value for later use (used to be DLL-static)
  tUtil::RegistrySetString(L, module_name_key, module_name);
  LUASTACK_CLEAN(L, 0);

  luaCompat_newLuaType(L, 
    module_name, 
    tLuaCOMEnumerator::type_name);

  luaCompat_newLuaType(L, 
    module_name, 
    tLuaCOMEnumerator::pointer_type_name);

  luaCompat_pushTypeByName(L, 
    module_name, 
    tLuaCOMEnumerator::type_name);

  lua_pushcfunction(L, tLuaCOMEnumerator::index);
  lua_setfield(L, -2, "__index");

  lua_pop(L, 1);

  luaCompat_pushTypeByName(L, 
    module_name, 
    tLuaCOMEnumerator::pointer_type_name);

  lua_pushcfunction(L, tLuaCOMEnumerator::garbagecollect);
  lua_setfield(L, -2, "__gc");

  lua_pop(L, 1);
  
  LUASTACK_CLEAN(L, 0);
}


int tLuaCOMEnumerator::call_method(lua_State *L)
{
  /// positions of parameters
  
  // self param (not used, but explicited to ensure
  // consistency)
  const int self_param        = 1;

  // first user param 
  const int user_first_param  = 2;
  
  // last user param
  const int user_last_param   = lua_gettop(L);

  // upvalues
  const int enumerator_param  = lua_upvalueindex(1);
  const int method_param      = lua_upvalueindex(2);

  int num_params = 0;

  if(user_last_param < user_first_param)
    num_params = 0;
  else
    num_params = user_last_param - user_first_param + 1;

  // gets the enumerator
  tLuaCOMEnumerator* enumerator = 
    (tLuaCOMEnumerator*)*(void **)lua_touserdata(L, enumerator_param);

  // gets the method name
  tStringBuffer method_name(lua_tostring(L, method_param));

  // call method
  int retval = 0;
  try
  {
    retval = enumerator->callCOMmethod(L, method_name, user_first_param, num_params);
  }
  catch(class tLuaCOMException& e)
  {
    luacom_error(L, e.getMessage());

    return 0;
  }

  return retval;
}

int tLuaCOMEnumerator::callCOMmethod(lua_State* L, const char *name, int first_param, int num_params)
{
  HRESULT hr = S_OK;

  // Next method
  if(strcmp(name, "Next") == 0)
  {
    unsigned long num_elements = 1;
    if(num_params > 0)
    {
      num_elements = (unsigned long) lua_tonumber(L, first_param);
    }

    VARIANT* pVar = new VARIANT[num_elements];

    for(unsigned long counter = 0; counter <  num_elements; counter++)
      VariantInit(&pVar[counter]);

    ULONG fetched = 0;
    hr = pEV->Next(num_elements, pVar, &fetched);
    
    for(unsigned long counter = 0; counter < fetched; counter++)
    {
      typehandler->com2lua(L, pVar[counter]);
      typehandler->releaseVariant(&pVar[counter]);
    }

    for(unsigned long counter = 0; counter <  num_elements; counter++)
      VariantClear(&pVar[counter]);

    delete[] pVar;

    pVar = NULL;

    return fetched;
  }

  if(strcmp(name, "Reset") == 0)
  {
    hr = pEV->Reset();
    CHK_LCOM_ERR(hr == S_OK, "Unable to reset enumeration.");
    
    return 0;
  }

  if(strcmp(name, "Skip") == 0)
  {
    CHK_LCOM_ERR(num_params > 0, "Not enough parameters.");

    unsigned long num_elements = (unsigned long) lua_tonumber(L, first_param);

    hr = pEV->Skip(num_elements);

    lua_pushboolean(L, hr == S_OK);
    return 1;
  }

  if(strcmp(name, "Clone") == 0)
  {
    tCOMPtr<IEnumVARIANT> p_newEV;
    CHK_COM_CODE(pEV->Clone(&p_newEV));

    tLuaCOMEnumerator* enumerator = new tLuaCOMEnumerator(p_newEV);
    
    enumerator->push(L);
    return 1;
  }

  return 0;
}
