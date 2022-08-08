#ifndef LUABEANS_H
#define LUABEANS_H

class LuaBeans
{
public:

  class Events
  {
  public:
    lua_CFunction settable;
    lua_CFunction index;
    lua_CFunction call;
    lua_CFunction gc;

    Events()
    {
      settable = index = call = gc = NULL;
    }
  };

  //lua_State* getLuaState(void);

   static void createBeans(
     lua_State *L,
     const char* module_name,
     const char* name);

   //static void Clean(void);

   static void registerObjectEvents(lua_State* L, class Events& events);
   static void registerPointerEvents(lua_State* L, class Events& events);
   static void  push(lua_State* L, void* userdata);
   static void* check_tag(lua_State* L, int index);
   static void* from_lua(lua_State* L, int index);

protected:
   static const char tag_name_key;
   static const char udtag_name_key;
   static const char module_name_key;
};
#endif
