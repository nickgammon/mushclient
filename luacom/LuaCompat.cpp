/*
 * LuaCompat.c
 *
 *  Implementation of the class LuaCompat,
 *  which tries to hide almost all diferences
 *  between Lua versions
 *
 *  This file isn't as useful as it used to be since
 *  we no longer support Lua < 5.1.
 */


#include <assert.h>
#include <string.h>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

#include "LuaCompat.h"
#include "LuaAux.h"

#define UNUSED(x) (void)(x)

#if LUA_VERSION_NUM >= 502

#ifndef luaL_register

void luaL_register (lua_State *L, const char *libname, const luaL_Reg *l){
  if(libname) lua_newtable(L);

  luaL_setfuncs(L, l, 0);

  if(libname){
    lua_pushvalue(L, -1);
    lua_setglobal(L, libname);
  }
}

#endif

#endif


int luaCompat_call(lua_State* L, int nargs, int nresults)
{
	tStringBuffer err;
	return luaCompat_call(L, nargs, nresults, err);
}
int luaCompat_call(lua_State* L, int nargs, int nresults, tStringBuffer& ErrMsg)
{
  int result = lua_pcall(L, nargs, nresults, 0);

  if(result)
  {
    ErrMsg.copyToBuffer(lua_tostring(L, -1));

    lua_pop(L, 1);
  }

  return result;
}


void luaCompat_newLuaType(lua_State* L, const char* module, const char* type)
{
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
{
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
{
  LUASTACK_SET(L);

  luaL_checktype(L, -1, LUA_TTABLE);

  lua_boxpointer(L, object);

  lua_insert(L, -2);

  lua_setmetatable(L, -2);

  LUASTACK_CLEAN(L, 0);

  return 1;
}


int luaCompat_isOfType(lua_State* L, const char* module, const char* type)
{
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
{
  LUASTACK_SET(L);
  int result = lua_getmetatable(L, index);

  if(!result)
    lua_pushnil(L);

  LUASTACK_CLEAN(L, 1);
}

const void* luaCompat_getType2(lua_State* L, int index)
{
  const void* result = 0;

  LUASTACK_SET(L);

  if(!lua_getmetatable(L, index))
    lua_pushnil(L);

  result = lua_topointer(L, -1);
  lua_pop(L, 1);

  LUASTACK_CLEAN(L, 0);

  return result;
}

void luaCompat_moduleCreate(lua_State* L, const char* module)
{
  LUASTACK_SET(L);

  lua_getfield(L, LUA_REGISTRYINDEX, module);
  if(lua_isnil(L, -1))
  {
    lua_pop(L, 1);
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, module);
  }

  LUASTACK_CLEAN(L, 1);
}

void luaCompat_moduleSet(lua_State* L, const char* module, const char* key)
{
  LUASTACK_SET(L);

  lua_getfield(L, LUA_REGISTRYINDEX, module);
  lua_pushvalue(L, -2);
  lua_setfield(L, -2, key);
  lua_pop(L, 2);

  LUASTACK_CLEAN(L, -1);
}

void luaCompat_moduleGet(lua_State* L, const char* module, const char* key)
{
  LUASTACK_SET(L);

  lua_getfield(L, LUA_REGISTRYINDEX, module);
  lua_getfield(L, -1, key);
  lua_remove(L, -2);

  LUASTACK_CLEAN(L, 1);
}

void* luaCompat_getPointer(lua_State* L, int index)
{
  if(!lua_islightuserdata(L, index))
    return NULL;

  return lua_touserdata(L, index);
}

int luaCompat_checkTagToCom(lua_State *L, int luaval) 
{
  /* unused: int has; */

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
