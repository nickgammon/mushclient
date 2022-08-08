#include <stdlib.h>
#include <stdio.h>
#include <string.h>
extern "C"
{
#include <lua.h>
#include <lauxlib.h>
}
#include "LuaCompat.h"
#include "luabeans.h"


#include "tLuaCOMException.h"
#include "LuaAux.h"
#include "tUtil.h"

// the address of each static is a unique memory location used as the key to the lua registry
// see http://www.lua.org/pil/27.3.1.html
const char LuaBeans::tag_name_key = 't';
const char LuaBeans::udtag_name_key = 'u';
const char LuaBeans::module_name_key = 'm';

void LuaBeans::createBeans(lua_State *L,
                           const char* p_module_name,
                           const char* name)
{
  LUASTACK_SET(L);

  char lua_name[500];

  tStringBuffer module_name(p_module_name);
  sprintf(lua_name, "%s" ,name);
  tStringBuffer tag_name(lua_name);
  luaCompat_newLuaType(L, module_name, tag_name);

  sprintf(lua_name,"%s_UDTAG",name);
  tStringBuffer udtag_name(lua_name);
  luaCompat_newLuaType(L, module_name, udtag_name);

  // store values for later use (used to be DLL-statics)
  tUtil::RegistrySetString(L, module_name_key, module_name);
  tUtil::RegistrySetString(L, tag_name_key, tag_name);
  tUtil::RegistrySetString(L, udtag_name_key, udtag_name);

  LUASTACK_CLEAN(L, 0);
}


/*void LuaBeans::Clean()
{
  free(LuaBeans::tag_name);
  free(LuaBeans::udtag_name);
}*/

void LuaBeans::registerObjectEvents(lua_State* L, class Events& events)
{
  LUASTACK_SET(L);

  tStringBuffer module_name(tUtil::RegistryGetString(L, module_name_key));
  tStringBuffer tag_name(tUtil::RegistryGetString(L, tag_name_key));
  tStringBuffer udtag_name(tUtil::RegistryGetString(L, udtag_name_key));
  LUASTACK_DOCLEAN(L, 0);

  luaCompat_pushTypeByName(L, module_name, tag_name);

  if(events.settable)
  {
    lua_pushcfunction(L, events.settable);
    lua_setfield(L, -2, "__newindex");
  }

  if(events.index)
  {
    lua_pushcfunction(L, events.index);
    lua_setfield(L, -2, "__index");
  }

  if(events.call)
  {
    lua_pushcfunction(L, events.call);
    lua_setfield(L, -2, "__call");
  }

  if(events.gc)
  {
    lua_pushcfunction(L, events.gc);
    lua_setfield(L, -2, "__gc");
  }

  lua_pop(L, 1);

  LUASTACK_CLEAN(L, 0);
}

void LuaBeans::registerPointerEvents(lua_State* L, class Events& events)
{
  LUASTACK_SET(L);

  tStringBuffer module_name(tUtil::RegistryGetString(L, module_name_key));
  tStringBuffer tag_name(tUtil::RegistryGetString(L, tag_name_key));
  tStringBuffer udtag_name(tUtil::RegistryGetString(L, udtag_name_key));
  LUASTACK_DOCLEAN(L, 0);

  luaCompat_pushTypeByName(L, module_name, udtag_name);

  if(events.settable)
  {
    lua_pushcfunction(L, events.settable);
    lua_setfield(L, -2, "__newindex");
  }

  if(events.index)
  {
    lua_pushcfunction(L, events.index);
    lua_setfield(L, -2, "__index");
  }

  if(events.gc)
  {
    lua_pushcfunction(L, events.gc);
    lua_setfield(L, -2, "__gc");
  }

  lua_pop(L, 1);

  LUASTACK_CLEAN(L, 0);
}

void LuaBeans::push(lua_State* L, void* userdata )
{
  LUASTACK_SET(L);

  tStringBuffer module_name(tUtil::RegistryGetString(L, module_name_key));
  tStringBuffer tag_name(tUtil::RegistryGetString(L, tag_name_key));
  tStringBuffer udtag_name(tUtil::RegistryGetString(L, udtag_name_key));
  LUASTACK_DOCLEAN(L, 0);

  lua_newtable(L);

  lua_pushstring(L, "_USERDATA_REF_");

  luaCompat_pushTypeByName(L, module_name, udtag_name);
  luaCompat_newTypedObject(L, userdata);

  lua_settable(L, -3);

  luaCompat_pushTypeByName(L, module_name, tag_name);
  lua_setmetatable(L, -2);

  LUASTACK_CLEAN(L, 1);
}

void* LuaBeans::check_tag(lua_State* L, int index)
{
  void* userdata = from_lua(L, index);

  luaL_argcheck(L, (userdata!=NULL), index, "Object type is wrong");

  return userdata;
}

void* LuaBeans::from_lua(lua_State* L, int index)
{
  LUASTACK_SET(L);

  tStringBuffer module_name(tUtil::RegistryGetString(L, module_name_key));
  tStringBuffer tag_name(tUtil::RegistryGetString(L, tag_name_key));
  tStringBuffer udtag_name(tUtil::RegistryGetString(L, udtag_name_key));
  LUASTACK_DOCLEAN(L, 0);

  void *obj = NULL;

  lua_pushvalue(L, index);
  if (lua_istable(L, -1) && luaCompat_isOfType(L, module_name, tag_name))
  {
    lua_pushstring(L, "_USERDATA_REF_");
    lua_gettable(L, index);
    obj = *(void **)lua_touserdata(L, -1);
    lua_pop(L, 1);
  }
  lua_pop(L, 1);

  LUASTACK_CLEAN(L, 0);
  
  return obj;
}


/*lua_State* LuaBeans::getLuaState()
{
  return L;
}*/

