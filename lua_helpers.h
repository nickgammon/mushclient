#include "lua.h"
#include "lauxlib.h"

void MakeTableItem (lua_State *L, const char * name, const CString & str);
void MakeTableItem (lua_State *L, const char * name, const string & str);
void MakeTableItem (lua_State *L, const char * name, const int n);
void MakeTableItem (lua_State *L, const char * name, const COleDateTime d);
void MakeTableItemBool (lua_State *L, const char * name, const bool b);
const bool optboolean (lua_State *L, const int narg, const int def);
bool FindLuaFunction (lua_State * L, const char * sName);
bool GetNestedFunction (lua_State * L, const char * sName, const bool bRaiseError = false);
int CallLuaWithTraceBack (lua_State *L, const int iArguments, const int iReturn);
