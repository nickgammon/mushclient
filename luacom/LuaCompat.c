/*
 * LuaCompat.c
 *
 *  Implementation of the class LuaCompat,
 *  which tries to hide almost all diferences
 *  between Lua versions
 *
 */


#include <assert.h>
#include <string.h>

#include "..\lua.h"
#include "..\lauxlib.h"

#ifdef COMPAT_5_1
#include "compat-5.1.h"
#endif

#include "LuaCompat.h"

#define UNUSED(x) (void)(x)

#define LUASTACK_SET(L) int __LuaAux_luastack_top_index = lua_gettop(L)

#ifdef NDEBUG
#define LUASTACK_CLEAN(L, n) lua_settop(L, __LuaAux_luastack_top_index + n)
#else
#define LUASTACK_CLEAN(L, n) assert((__LuaAux_luastack_top_index + n) == lua_gettop(L))
#endif

#define LUACOMPAT_ERRORMESSAGE "__luacompat_errormesage"


/*****************************
 * LUA 4 compatibility code
 *****************************/

#ifdef LUA4
#define __LUACOMPAT_OK

/* lua4 version of API */

void luaCompat_openlib(lua_State* L, const char* libname, const struct luaL_reg* funcs)
{ /* lua4 */
  LUASTACK_SET(L);

  char funcname[1000];

  lua_newtable(L);  /* create it */
  lua_pushvalue(L, -1);
  lua_setglobal(L, libname);  /* register it with given name */

  for (; funcs->name; funcs++)
  {
    int i;
    lua_pushstring(L, funcs->name);
    lua_pushcfunction(L, funcs->func);
    lua_settable(L, -3);

    funcname[0] = '\0';

    strncat(funcname, libname, 1000);
    strcat(funcname, "_");
    strncat(funcname, funcs->name, 1000 - strlen(libname) - strlen(funcs->name) - 2);
    lua_pushcfunction(L, funcs->func);
    lua_setglobal(L, funcname);
  }

  LUASTACK_CLEAN(L, 1);
}


void luaCompat_error(lua_State* L, const char* message)
{ /* lua4 */
  lua_error(L, message);
}


static int errorhandler(lua_State* L)
{
  lua_setglobal(L, LUACOMPAT_ERRORMESSAGE);

  return 0;
}

int luaCompat_call(lua_State* L, int nargs, int nresults, const char **pErrMsg)
{ /* lua4 */
  int result = 0;
  int user_func = 0;

  lua_getglobal(L, "_ERRORMESSAGE");
  lua_insert(L, 1);

  lua_register(L, "_ERRORMESSAGE", errorhandler);

  result = lua_call(L, nargs, nresults);

  if(result && pErrMsg)
  {
    lua_getglobal(L, LUACOMPAT_ERRORMESSAGE);
    *pErrMsg = lua_tostring(L, -1);
    lua_pop(L, 1);
  }

  lua_pushvalue(L, 1);
  lua_setglobal(L, "_ERRORMESSAGE");

  lua_remove(L, 1);

  return result;
}


void luaCompat_newLuaType(lua_State* L,
                           const char* module_name,
                           const char* type_name)
{ /* lua4 */
  LUASTACK_SET(L);

  int tag = lua_newtag(L);

  lua_pushnumber(L, tag);

  luaCompat_moduleSet(L, module_name, type_name);

  LUASTACK_CLEAN(L, 0);
}

void luaCompat_pushTypeByName(lua_State* L,
                               const char* module_name,
                               const char* type_name)
{ /* lua4 */
  LUASTACK_SET(L);

  luaCompat_moduleGet(L, module_name, type_name);

  if(lua_isnil(L, -1))
  {
    lua_pop(L, 1);
    luaCompat_newLuaType(L, module_name, type_name);
    luaCompat_moduleGet(L, module_name, type_name);    
  }

  LUASTACK_CLEAN(L, 1);
}


int luaCompat_newTypedObject(lua_State* L, void* object)
{  /* lua4 */
  int newreference = 0;
  int tag = 0;

  LUASTACK_SET(L);

  luaL_checktype(L, -1, LUA_TNUMBER);

  tag = (int) lua_tonumber(L, -1);

  lua_pop(L, 1);

  /* pushes userdata */
  lua_pushusertag(L, object, LUA_ANYTAG);

  if(lua_tag(L, -1) != tag)
  {
    /* this is the first userdata with this value,
       so corrects the tag */
    lua_settag(L, tag);
    newreference = 1;
  }

  LUASTACK_CLEAN(L, 0);

  return newreference;
}

void luaCompat_setType(lua_State* L, int index)
{ /* lua4 */

  LUASTACK_SET(L);

  int tag = lua_tonumber(L, -1);

  lua_pushvalue(L, index);

  lua_settag(L, tag);

  lua_pop(L, 2);

  LUASTACK_CLEAN(L, -1);
}


void luaCompat_moduleSet(lua_State* L, const char* module, const char* key)
{ /* lua4 */
  LUASTACK_SET(L);

  lua_getglobal(L, module);

  lua_pushstring(L, key);
  lua_pushvalue(L, -3);
  lua_settable(L, -3);

  lua_pop(L, 2);

  LUASTACK_CLEAN(L, -1);
}

void luaCompat_moduleGet(lua_State* L, const char* module, const char* key)
{ /* lua4 */
  LUASTACK_SET(L);

  lua_getglobal(L, module);
  lua_pushstring(L, key);

  lua_gettable(L, -2);

  lua_remove(L, -2);

  LUASTACK_CLEAN(L, 1);
}


void* luaCompat_getTypedObject(lua_State* L, int index)
{ /* lua4 */
  return lua_touserdata(L, index);
}

int luaCompat_isOfType(lua_State* L, const char* module, const char* type)
{ /* lua4 */
  int result = 0;

  LUASTACK_SET(L);

  luaCompat_getType(L, -1);
  luaCompat_pushTypeByName(L, module, type);

  result = lua_equal(L, -1, -2);

  lua_pop(L, 2);

  LUASTACK_CLEAN(L,0);

  return result;
}

void luaCompat_getType(lua_State* L, int index)
{ /* lua4 */
  LUASTACK_SET(L);

  int tag = lua_tag(L, index);
  lua_pushnumber(L, tag);

  LUASTACK_CLEAN(L, 1);
}

long luaCompat_getType2(lua_State* L, int index)
{ /* lua4 */
  int tag = lua_tag(L, index);
  return tag;
}



void luaCompat_handleEqEvent(lua_State* L)
{ /* lua4 */
  LUASTACK_SET(L);

  /* lua4 does not have eq event */
  lua_pop(L, 1);

  LUASTACK_CLEAN(L, -1);
}



void luaCompat_handleGettableEvent(lua_State* L)
{ /* lua4 */
  LUASTACK_SET(L);

  int tag = lua_tonumber(L, -2);

  lua_settagmethod(L, tag, "gettable");

  LUASTACK_CLEAN(L, -1);
}

void luaCompat_handleSettableEvent(lua_State* L)
{ /* lua4 */
  LUASTACK_SET(L);

  int tag = lua_tonumber(L, -2);

  lua_settagmethod(L, tag, "settable");

  LUASTACK_CLEAN(L, -1);

}

void luaCompat_handleNoIndexEvent(lua_State* L)
{ /* lua4 */
  LUASTACK_SET(L);

  int tag = lua_tonumber(L, -2);

  lua_settagmethod(L, tag, "index");

  LUASTACK_CLEAN(L, -1);
}

void luaCompat_handleGCEvent(lua_State* L)
{ /* lua4 */
  LUASTACK_SET(L);

  int tag = lua_tonumber(L, -2);

  lua_settagmethod(L, tag, "gc");

  LUASTACK_CLEAN(L, -1);
}

void luaCompat_handleFuncCallEvent(lua_State* L)
{ /* lua4 */
  LUASTACK_SET(L);

  int tag = lua_tonumber(L, -2);

  lua_settagmethod(L, tag, "function");

  LUASTACK_CLEAN(L, -1);
}


int luaCompat_upvalueIndex(lua_State* L, int which, int num_upvalues)
{ /* lua4 */
  return lua_gettop(L) + which - num_upvalues;
}

int luaCompat_getNumParams(lua_State* L, int num_upvalues)
{ /* lua4 */
  return lua_gettop(L) - num_upvalues;
}

void luaCompat_moduleCreate(lua_State* L, const char* module)
{ /* lua4 */
  LUASTACK_SET(L);

  // tests whether module already exists
  lua_getglobal(L, module);

  if(lua_isnil(L, -1))
  {
    lua_pop(L, 1);

    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, module);
  }

  LUASTACK_CLEAN(L, 1);
}

void luaCompat_pushPointer(lua_State* L, void *pointer)
{ /* lua4 */
  lua_pushuserdata(L, pointer);
}

void* luaCompat_getPointer(lua_State* L, int index)
{ /* lua4 */
  return lua_touserdata(L, index);
}

void luaCompat_pushBool(lua_State* L, int value)
{ /* lua4 */
  LUASTACK_SET(L);

  if(value)
    lua_pushnumber(L, 1);
  else
    lua_pushnil(L);

  LUASTACK_CLEAN(L, 1);
}

void luaCompat_pushCBool(lua_State* L, int value)
{ /* lua4 */
  LUASTACK_SET(L);

  if(value)
    lua_pushnumber(L, 1);
  else
    lua_pushnumber(L, 0);

  LUASTACK_CLEAN(L, 1);
}

int luaCompat_toCBool(lua_State* L, int index)
{ /* lua4 */
  int value = lua_tonumber(L, index);

  return value;
}


void luaCompat_needStack(lua_State* L, long size)
{ /* lua4 */
  assert(lua_stackspace(L) >= size);
}


void luaCompat_getglobal(lua_State* L)
{ /* lua4 */
  lua_getglobal(L, lua_tostring(L, -1));
  lua_remove(L, -2);
}

void luaCompat_setglobal(lua_State* L)
{ /* lua4 */
  lua_setglobal(L, lua_tostring(L, -2));
}

int luaCompat_checkTagToCom(lua_State *L, int luaval) 
{ /* lua4 */
  int tag;

  if((tag = lua_tag(L, luaval)) == LUA_NOTAG) return 0;

  lua_gettagmethod(L, tag, "tocom");
  if(lua_isnil(L,-1)) {
    lua_pop(L,1);
	return 0;
  }

  return 1;
}


#endif /* lua4 */



















/********************************
 * LUA 5 section 
 ********************************/

#ifdef LUA5
#define __LUACOMPAT_OK

/* Lua 5 version of the API */

void luaCompat_openlib(lua_State* L, const char* libname, const struct luaL_reg* funcs)
{ /* lua5 */
  LUASTACK_SET(L);

#ifdef COMPAT_5_1
  luaL_module(L, libname, funcs, 0);
#else
  luaL_openlib(L, libname, funcs, 0);
#endif

  LUASTACK_CLEAN(L, 1);
}

void luaCompat_error(lua_State* L, const char* message)
{ /* lua5 */
  lua_pushstring(L, message);
  lua_error(L);
}

int luaCompat_call(lua_State* L, int nargs, int nresults, const char** pErrMsg)
{ /* lua5 */
  int result = lua_pcall(L, nargs, nresults, 0);

  if(result)
  {
    if(pErrMsg)
      *pErrMsg = lua_tostring(L, -1);

    lua_pop(L, 1);
  }

  return result;
}


void luaCompat_newLuaType(lua_State* L, const char* module, const char* type)
{ /* lua5 */
  LUASTACK_SET(L);

  lua_newtable(L);

  /* stores the typename in the Lua table, allowing some reflexivity */
  lua_pushstring(L, "type");
  lua_pushstring(L, type);
  lua_settable(L, -3);

  /* stores type in the module */
  luaCompat_moduleSet(L, module, type);

  LUASTACK_CLEAN(L, 0);
}

void luaCompat_pushTypeByName(lua_State* L,
                               const char* module_name,
                               const char* type_name)
{ /* lua5 */
  LUASTACK_SET(L);

  luaCompat_moduleGet(L, module_name, type_name);

  if(lua_isnil(L, -1))
  {
    lua_pop(L, 1);
    luaCompat_newLuaType(L, module_name, type_name);
    luaCompat_moduleGet(L, module_name, type_name);    
  }

  LUASTACK_CLEAN(L, 1);
}

#ifndef lua_boxpointer
#define lua_boxpointer(L,u) \
	(*(void **)(lua_newuserdata(L, sizeof(void *))) = (u))
#define lua_unboxpointer(L,i)	(*(void **)(lua_touserdata(L, i)))
#endif

int luaCompat_newTypedObject(lua_State* L, void* object)
{ /* lua5 */
  LUASTACK_SET(L);

  luaL_checktype(L, -1, LUA_TTABLE);

  lua_boxpointer(L, object);

  lua_insert(L, -2);

  lua_setmetatable(L, -2);

  LUASTACK_CLEAN(L, 0);

  return 1;
}

void luaCompat_setType(lua_State* L, int index)
{ /* lua5 */

  LUASTACK_SET(L);

  lua_setmetatable(L, index);    

  LUASTACK_CLEAN(L,-1);
}



void luaCompat_moduleSet(lua_State* L, const char* module, const char* key)
{ /* lua5 */
  LUASTACK_SET(L);

  lua_pushstring(L, module);
  lua_gettable(L, LUA_REGISTRYINDEX);

  lua_pushstring(L, key);
  lua_pushvalue(L, -3);
  lua_settable(L, -3);

  lua_pop(L, 2);

  LUASTACK_CLEAN(L, -1);
}

void luaCompat_moduleGet(lua_State* L, const char* module, const char* key)
{ /* lua5 */
  LUASTACK_SET(L);

  lua_pushstring(L, module);
  lua_gettable(L, LUA_REGISTRYINDEX);

  lua_pushstring(L, key);
  lua_gettable(L, -2);

  lua_remove(L, -2);

  LUASTACK_CLEAN(L, 1);
}


void* luaCompat_getTypedObject(lua_State* L, int index)
{ /* lua5 */
  void **pObj = (void **) lua_touserdata(L, index);

  void *Obj= *pObj;

  return Obj;
}


int luaCompat_isOfType(lua_State* L, const char* module, const char* type)
{ /* lua5 */
  int result = 0;
  LUASTACK_SET(L);

  luaCompat_getType(L, -1);
  luaCompat_pushTypeByName(L, module, type);

  result = (lua_equal(L, -1, -2) ? 1 : 0);

  lua_pop(L, 2);

  LUASTACK_CLEAN(L, 0);

  return result;
}

void luaCompat_getType(lua_State* L, int index)
{ /* lua5 */
  LUASTACK_SET(L);
  int result = lua_getmetatable(L, index);

  if(!result)
    lua_pushnil(L);

  LUASTACK_CLEAN(L, 1);
}

long luaCompat_getType2(lua_State* L, int index)
{ /* lua5 */
  long result = 0;

  LUASTACK_SET(L);

  if(!lua_getmetatable(L, index))
    lua_pushnil(L);

  result = (long) lua_topointer(L, -1);
  lua_pop(L, 1);

  LUASTACK_CLEAN(L, 0);

  return result;
}


void luaCompat_handleEqEvent(lua_State* L)
{ /* lua5 */
  LUASTACK_SET(L);

  lua_pushstring(L, "__eq");
  lua_insert(L, -2);

  lua_settable(L, -3);

  LUASTACK_CLEAN(L, -1);
}


void luaCompat_handleGettableEvent(lua_State* L)
{ /* lua5 */
  // there is no gettable_event in Lua5 with the semantics of
  // Lua4
}

void luaCompat_handleSettableEvent(lua_State* L)
{ /* lua5 */
  LUASTACK_SET(L);

  lua_pushstring(L, "__newindex");
  lua_insert(L, -2);

  lua_settable(L, -3);

  LUASTACK_CLEAN(L, -1);

}


void luaCompat_handleNoIndexEvent(lua_State* L)
{ /* lua5 */
  LUASTACK_SET(L);

  lua_pushstring(L, "__index");
  lua_insert(L, -2);

  lua_settable(L, -3);

  LUASTACK_CLEAN(L, -1);
}


void luaCompat_handleGCEvent(lua_State* L)
{ /* lua5 */
  LUASTACK_SET(L);

  lua_pushstring(L, "__gc");
  lua_insert(L, -2);

  lua_settable(L, -3);

  LUASTACK_CLEAN(L, -1);
}

void luaCompat_handleFuncCallEvent(lua_State* L)
{ /* lua5 */
  LUASTACK_SET(L);

  lua_pushstring(L, "__call");
  lua_insert(L, -2);

  lua_settable(L, -3);

  LUASTACK_CLEAN(L, -1);
}


int luaCompat_upvalueIndex(lua_State* L, int which, int num_upvalues)
{ /* lua5 */
  UNUSED(num_upvalues);

  return lua_upvalueindex(which);
}

int luaCompat_getNumParams(lua_State* L, int num_upvalues)
{ /* lua5 */
  UNUSED(num_upvalues);
  return lua_gettop(L);
}

void luaCompat_moduleCreate(lua_State* L, const char* module)
{ /* lua5 */
  LUASTACK_SET(L);

  lua_pushstring(L, module);
  lua_gettable(L, LUA_REGISTRYINDEX);

  if(lua_isnil(L, -1))
  {
    lua_pop(L, 1);
    lua_newtable(L);
    lua_pushstring(L, module);
    lua_pushvalue(L, -2);

    lua_settable(L, LUA_REGISTRYINDEX);
  }

  LUASTACK_CLEAN(L, 1);
}

void luaCompat_pushPointer(lua_State* L, void *pointer)
{ /* lua5 */
  lua_pushlightuserdata(L, pointer);
}

void* luaCompat_getPointer(lua_State* L, int index)
{ /* lua5 */
  if(!lua_islightuserdata(L, index))
    return NULL;

  return lua_touserdata(L, index);
}

void luaCompat_pushBool(lua_State* L, int value)
{ /* lua5 */
  LUASTACK_SET(L);

  lua_pushboolean(L, value);

  LUASTACK_CLEAN(L, 1);
}

void luaCompat_pushCBool(lua_State* L, int value)
{ /* lua5 */
  LUASTACK_SET(L);

  lua_pushboolean(L, value);

  LUASTACK_CLEAN(L, 1);
}

int luaCompat_toCBool(lua_State* L, int index)
{ /* lua5 */
  int value = lua_toboolean(L, index);

  return value;
}


void luaCompat_needStack(lua_State* L, long size)
{ /* lua5 */
  lua_checkstack(L, size);
}


void luaCompat_getglobal(lua_State* L)
{ /* lua5 */
  lua_gettable(L, LUA_GLOBALSINDEX);
}

void luaCompat_setglobal(lua_State* L)
{ /* lua5 */
  lua_settable(L, LUA_GLOBALSINDEX);
}


int luaCompat_checkTagToCom(lua_State *L, int luaval) 
{ /* lua5 */

  if(!lua_getmetatable(L, luaval)) return 0;

  lua_pushstring(L, "__tocom");
  lua_gettable(L, -2);
  if(lua_isnil(L,-1)) {
    lua_pop(L, 2);
	return 0;
  }

  lua_remove(L,-2);
  return 1;
}




#endif /* LUA5 */















#ifndef __LUACOMPAT_OK
#error Lua version not specified
#endif

