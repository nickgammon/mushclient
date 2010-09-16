#include "stdafx.h"
#include "..\mainfrm.h"
#include "..\MUSHclient.h"
#include "..\doc.h"
#include "..\scripting\errors.h"
#include "..\winplace.h"
#include "..\TextDocument.h"
#include "..\TextView.h"
#include "..\sendvw.h"
#include "..\dialogs\DebugLuaDlg.h"

const char mushclient_typename[] = "mushclient.world";

/*------------------------------------------------------------------------
   This whole file is basically "glue" routines to bind the Lua calls to the
   normal MUSHclient script calls. As far as possible the calls will be compatible
   with existing ones.
 
   A couple of exceptions will be things like GetAlias which return results 
   by reference, these are changed to return multiple results.

   Written: 20th November 2004 by Nick Gammon

   Multiple-world support
   ----------------------

   The functions will automatically check if the first argument is a userdatum, 
   and if so, check that it is a mushclient.world userdatum.

   This lets you direct commands to other worlds, like this:

   other = world.GetWorld ("my other world")

   world.Note (other, "hello there")
   world.AddMapperComment (other, "this is a comment")
   
  ------------------------------------------------------------------------ */

// same as luaL_checkudata but just returns NULL if not correct type

LUALIB_API void *isudata (lua_State *L, int ud, const char *tname) {
  void *p = lua_touserdata(L, ud);
  if (p != NULL) {  /* value is a userdata? */
    if (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
      lua_getfield(L, LUA_REGISTRYINDEX, tname);  /* get correct metatable */
      int bCorrectType = lua_rawequal(L, -1, -2);
      lua_pop(L, 2);  /* remove both metatables */
      if (bCorrectType) /* does it have the correct mt? */
        return p;
    }   // has metatable
  }  // is userdata
  return NULL;  /* to avoid warnings */
}


// checks user data is mushclient.doc, and returns pointer to world
static CMUSHclientDoc *doc (lua_State *L) 
  { 
// mushclient_typename is "mushclient.world"
CMUSHclientDoc **ud = (CMUSHclientDoc **) isudata (L, 1, mushclient_typename);

  // if first argument is a world userdatum, take that as our world

  if (ud)
    {
    CMUSHclientDoc * pWantedDoc = *ud;
    luaL_argcheck(L, ud != NULL, 1, "'world' expected");
    lua_remove (L, 1);    // remove the userdata, we don't need it now

    // for safety, check that world still exists
    for (POSITION docPos = App.m_pWorldDocTemplate->GetFirstDocPosition();
        docPos != NULL; )
      {
      CMUSHclientDoc * pDoc = (CMUSHclientDoc *) App.m_pWorldDocTemplate->GetNextDoc(docPos);

      if (pDoc == pWantedDoc)
        return pDoc;
      } // end of doing each document

    luaL_error (L, "world is no longer available");
    }

  // retrieve our state (world pointer)
  
  // retrieve the document pointer from the environment table
  lua_getfield (L, LUA_ENVIRONINDEX, DOCUMENT_STATE);  // get "mushclient.document" value
  CMUSHclientDoc * pDoc = (CMUSHclientDoc *) lua_touserdata (L, -1);  // convert to world pointer
  lua_pop (L, 1);  // pop document pointer

  return pDoc;
  }

// helper function for pushing results returned by normal MUSHclient methods
// (stored in a variant)

int pushVariant (lua_State *L, VARIANT & v)
  {
  switch (v.vt)
    {
    case VT_BSTR:
      lua_pushstring (L, CString (v.bstrVal));
      VariantClear (&v);
      break;
    case VT_I2:
      lua_pushnumber (L, v.iVal);
      break;
    case VT_I4:
      lua_pushnumber (L, v.lVal);
      break;
    case VT_R4:
      lua_pushnumber (L, v.fltVal);
      break;
    case VT_R8:
      lua_pushnumber (L, v.dblVal);
      break;
    case VT_BOOL:
      lua_pushboolean (L, v.boolVal);
      break;
    case VT_DATE:
      {
// maybe not, let the user format the date
//      COleDateTime d (v.date);
//      lua_pushstring (L, d.Format (0, 0));

//      lua_pushnumber (L, v.date);

      COleDateTime dt (v.date);   // convert to  COleDateTime

      SYSTEMTIME sysTime;

      dt.GetAsSystemTime (sysTime);  // now get as SYSTEMTIME
      CTime tm (sysTime);         // import into CTime

      lua_pushnumber (L, (lua_Number) tm.GetTime ());  // return as Lua-style system time

      break;
      }

    case (VT_ARRAY + VT_VARIANT): // array of variants
      {
      lua_newtable(L);                                                            

      SAFEARRAY * psa = v.parray;
      long lLBound, lUBound;

      SafeArrayGetLBound(psa, 1, &lLBound);
      SafeArrayGetUBound(psa, 1, &lUBound);

      VARIANT val;
      int extra = 0;
      if (lLBound == 0)
        extra = 1;
      for (int i = lLBound; i <= lUBound; i++)
        {
        long index[2];
        index [0] = i;
        index [1] = 0;
        SafeArrayGetElement(psa, index, &val);

        pushVariant (L, val);   // recurse to push the element value
        lua_rawseti(L, -2, i + extra);  // make 1-relative
        }   // end of doing each row
      VariantClear (&v);
      }   // end of VT_ARRAY
      break;

    default:
      lua_pushnil (L);
      break;
    }  // end of switch on type
  return 1;  // number of result fields
  } // end of pushVariant

// helper function for pushing results returned by normal MUSHclient methods
// (stored in a BSTR)

int pushBstr (lua_State *L, BSTR & str)
  {
  lua_pushstring (L, CString (str));
  SysFreeString (str);
  return 1;  // number of result fields
  }   // end of pushBstr

// helper function for things like print, note, send etc. to combine all arguments
// into a string

string concatArgs (lua_State *L, const char * delimiter = "", const int first = 1)
  {
  int n = lua_gettop(L);  /* number of arguments */
  int i;
  string sOutput;
  lua_getglobal(L, "tostring");
  for (i = first; i <= n; i++) 
    {
    const char *s;
    lua_pushvalue(L, -1);  /* function to be called */
    lua_pushvalue(L, i);   /* value to print */
    lua_call(L, 1, 1);
    s = lua_tostring(L, -1);  /* get result */
    if (s == NULL)
      luaL_error(L, "'tostring' must return a string to be concatenated");
        
    // add delimiter every time except first time through the loop
    if (i > first)
      sOutput += delimiter;
    sOutput += s;
    lua_pop(L, 1);  /* pop result */
    }

  lua_pop(L, 1);  /* pop tostring function */
  return sOutput;

  } // end of concatArgs

static void GetVariableListHelper (lua_State *L, CMUSHclientDoc *pDoc)
  {
  lua_newtable(L);                                                            

  for (POSITION pos = pDoc->GetVariableMap ().GetStartPosition(); pos; )
    {
    CString strVariableName;
    CVariable * variable_item;

    pDoc->GetVariableMap ().GetNextAssoc (pos, strVariableName, variable_item);

    lua_pushstring (L, strVariableName);
    lua_pushstring (L, variable_item->strContents);
    lua_rawset(L, -3);
    }      // end of looping through each Variable

  } // end of GetVariableListHelper

//----- My checking routines that do not witter on about "bad self" -----


LUALIB_API int my_argerror (lua_State *L, int narg, const char *extramsg) {
  lua_Debug ar;
  if (!lua_getstack(L, 0, &ar))  /* no stack frame? */
    return luaL_error(L, "bad argument #%d (%s)", narg, extramsg);
  lua_getinfo(L, "n", &ar);
  if (ar.name == NULL)
    ar.name = "?";
  return luaL_error(L, "bad argument #%d to " LUA_QS " (%s)",
                        narg, ar.name, extramsg);
}

LUALIB_API int my_typerror (lua_State *L, int narg, const char *tname) {
  const char *msg = lua_pushfstring(L, "%s expected, got %s",
                                    tname, luaL_typename(L, narg));
  return my_argerror(L, narg, msg);
}


static void my_tag_error (lua_State *L, int narg, int tag) {
  my_typerror(L, narg, lua_typename(L, tag));
}

LUALIB_API const char *my_checklstring (lua_State *L, int narg, size_t *len) {
  const char *s = lua_tolstring(L, narg, len);
  if (!s) my_tag_error(L, narg, LUA_TSTRING);
  return s;
}

LUALIB_API const char *my_optlstring (lua_State *L, int narg,
                                        const char *def, size_t *len) {
  if (lua_isnoneornil(L, narg)) {
    if (len)
      *len = (def ? strlen(def) : 0);
    return def;
  }
  else return my_checklstring(L, narg, len);
}

LUALIB_API lua_Number my_checknumber (lua_State *L, int narg) {
  lua_Number d = lua_tonumber(L, narg);
  if (d == 0 && !lua_isnumber(L, narg))  /* avoid extra test when d is not 0 */
    my_tag_error(L, narg, LUA_TNUMBER);
  return d;
}

LUALIB_API lua_Number my_optnumber (lua_State *L, int narg, lua_Number def) {
  return luaL_opt(L, my_checknumber, narg, def);
}

LUALIB_API const char *get_option_value (lua_State *L, int narg) {

  // if boolean convert to 0 or 1
  if (lua_isboolean (L, narg))
    return (lua_toboolean (L, narg) ? "1" : "0");
  else if (lua_isnil (L, narg))
    return "0";  // nil is considered false

  const char *s = lua_tolstring(L, narg, NULL);
  if (!s) 
    my_tag_error(L, narg, LUA_TSTRING);
  return s;
}

#define my_checkstring(L,n)	(my_checklstring(L, (n), NULL))
#define my_optstring(L,n,d)	(my_optlstring(L, (n), (d), NULL))

//-------------- end of special checks --------------------


const bool optboolean (lua_State *L, const int narg, const int def) 
  {
  // that argument not present, take default
  if (lua_gettop (L) < narg)
    return def;

  // nil will default to the default
  if (lua_isnil (L, narg))
    return def;

  // try to convert boolean
  if (lua_isboolean (L, narg))
    return lua_toboolean (L, narg);

  return my_checknumber (L, narg) != 0;
}

//----------------------------------------
//  world.Accelerator
//----------------------------------------
static int L_Accelerator (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->Accelerator (
        my_checkstring (L, 1),       // Key
        my_checkstring (L, 2)        // Send
        ));
  return 1;  // number of result fields
  } // end of L_Accelerator

//----------------------------------------
//  world.AcceleratorTo
//----------------------------------------
static int L_AcceleratorTo (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->AcceleratorTo (
        my_checkstring (L, 1),       // Key
        my_checkstring (L, 2),       // Send
        (short) my_checknumber (L, 3)        // SendTo
        ));
  return 1;  // number of result fields
  } // end of L_AcceleratorTo

//----------------------------------------
//  world.AcceleratorList
//----------------------------------------
static int L_AcceleratorList (lua_State *L)
  {
  VARIANT v = doc (L)->AcceleratorList ();
  return pushVariant (L, v);
  } // end of L_AcceleratorList

//----------------------------------------
//  world.Activate
//----------------------------------------
static int L_Activate (lua_State *L)
  {
  doc (L)->Activate ();
  return 0;  // number of result fields
  } // end of L_Activate

//----------------------------------------
//  world.ActivateClient
//----------------------------------------
static int L_ActivateClient (lua_State *L)
  {
  doc (L)->ActivateClient ();
  return 0;  // number of result fields
  } // end of L_ActivateClient


//----------------------------------------
//  world.ActivateNotepad
//----------------------------------------
static int L_ActivateNotepad (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushboolean (L, pDoc->ActivateNotepad (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_ActivateNotepad


//----------------------------------------
//  world.AddAlias
//----------------------------------------
static int L_AddAlias (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->AddAlias (
                  my_checkstring (L, 1),  // AliasName
                  my_checkstring (L, 2),  // MatchText  
                  my_checkstring (L, 3),  // ResponseText
                  (long) my_checknumber (L, 4),  // Flags
                  my_optstring (L, 5, "")   // ScriptName - optional
                  ));
  return 1;  // number of result fields
  } // end of L_AddAlias

//----------------------------------------
//  world.AddFont
//----------------------------------------
static int L_AddFont (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->AddFont (
                  my_checkstring (L, 1)  // PathName
                  ));
  return 1;  // number of result fields
  } // end of L_AddFont

//----------------------------------------
//  world.AddMapperComment
//----------------------------------------
static int L_AddMapperComment (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->AddMapperComment (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_AddMapperComment


//----------------------------------------
//  world.AddSpellCheckWord
//----------------------------------------
static int L_AddSpellCheckWord (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L,  pDoc->AddSpellCheckWord (
              my_checkstring (L, 1), // Original word
              my_checkstring (L, 2), // Action
              my_optstring (L, 3, "") // Replacement word - optional
              ));
  return 1;  // number of result fields
  } // end of L_AddSpellCheckWord

//----------------------------------------
//  world.AddTimer
//----------------------------------------
static int L_AddTimer (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->AddTimer (
                  my_checkstring (L, 1),  // TimerName
                  (short) my_checknumber (L, 2),  // Hour
                  (short) my_checknumber (L, 3),  // Minute
                  my_checknumber (L, 4),  // Second
                  my_checkstring (L, 5),  // ResponseText  
                  (long) my_checknumber (L, 6),  // Flags
                  my_optstring (L, 7, "")   // ScriptName - optional
                  ));
  return 1;  // number of result fields
  } // end of L_AddTimer


//----------------------------------------
//  world.AddToMapper
//----------------------------------------
static int L_AddToMapper (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L,  pDoc->AddToMapper (
              my_checkstring (L, 1), // Direction
              my_optstring (L, 2, "") // Reverse - optional
              ));
  return 1;  // number of result fields
  } // end of L_AddToMapper


//----------------------------------------
//  world.AddTrigger
//----------------------------------------
static int L_AddTrigger (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->AddTrigger (
                  my_checkstring (L, 1),  // TriggerName
                  my_checkstring (L, 2),  // MatchText
                  my_checkstring (L, 3),  // ResponseText
                  (long) my_checknumber (L, 4),  // Flags  
                  (short) my_checknumber (L, 5),  // Colour
                  (short) my_checknumber (L, 6),  // Wildcard
                  my_checkstring (L, 7),  // SoundFileName  
                  my_optstring (L, 8, "")   // ScriptName - optional
                  ));
  return 1;  // number of result fields
  } // end of L_AddTrigger


//----------------------------------------
//  world.AddTriggerEx
//----------------------------------------
static int L_AddTriggerEx (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->AddTriggerEx (
                  my_checkstring (L, 1),  // TriggerName
                  my_checkstring (L, 2),  // MatchText
                  my_checkstring (L, 3),  // ResponseText
                  (long) my_checknumber (L, 4),  // Flags  
                  (short) my_checknumber (L, 5),  // Colour
                  (short) my_checknumber (L, 6),  // Wildcard
                  my_checkstring (L, 7),  // SoundFileName  
                  my_checkstring (L, 8),  // ScriptName
                  (short) my_checknumber (L, 9),  // SendTo  
                  (short) my_optnumber (L, 10, DEFAULT_TRIGGER_SEQUENCE)   // Sequence  - optional
                  ));
  return 1;  // number of result fields
  } // end of L_AddTriggerEx


//----------------------------------------
//  world.AdjustColour
//----------------------------------------
static int L_AdjustColour (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->AdjustColour (
                  (long) my_checknumber (L, 1),    // Colour
                  (short) my_checknumber (L, 2)     // Method
                  ));
  return 1;  // number of result fields
  } // end of L_AdjustColour


//----------------------------------------
//  world.ANSI
//----------------------------------------
static int L_ANSI (lua_State *L)
  {
  doc (L);  // must do this first

  int n = lua_gettop(L);  /* number of arguments */
  int i;
  CString sOutput = "\x1B[";

  for (i= 1; i <= n; i++) 
    {
    sOutput += CFormat ("%i", (int) my_checknumber (L, i));
    if (i < n)
      sOutput += ";";
    }

  sOutput += "m";
  lua_pushstring (L, sOutput);
  return 1;
  } // end of L_ANSI


//----------------------------------------
//  world.AnsiNote
//----------------------------------------
static int L_AnsiNote (lua_State *L)
  {
  CMUSHclientDoc * pDoc = doc (L);  // must do this first

  pDoc->AnsiNote (concatArgs (L).c_str ());

//  lua_concat (L, lua_gettop (L));
//  pDoc->AnsiNote (my_checkstring (L, 1));
  return 0;  // number of result fields
  } // end of L_AnsiNote


//----------------------------------------
//  world.AppendToNotepad
//----------------------------------------
static int L_AppendToNotepad (lua_State *L)
  {
  CMUSHclientDoc * pDoc = doc (L);  // must do this first

  lua_pushboolean (L, pDoc->AppendToNotepad (
                    my_checkstring (L, 1), // title
                    concatArgs (L, "", 2).c_str ()  // contents
                    ));

//  lua_concat (L, lua_gettop (L) - 1);
//  lua_pushboolean (L, pDoc->AppendToNotepad (
//                    my_checkstring (L, 1), // title
//                    my_checkstring (L, 2)  // contents
//                    ));
  return 1;  // number of result fields
  } // end of L_AppendToNotepad


//----------------------------------------
//  world.ArrayClear
//----------------------------------------
static int L_ArrayClear (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->ArrayClear (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_ArrayClear


//----------------------------------------
//  world.ArrayCount
//----------------------------------------
static int L_ArrayCount (lua_State *L)
  {
  lua_pushnumber (L, doc (L)->ArrayCount ());
  return 1;  // number of result fields
  } // end of L_ArrayCount


//----------------------------------------
//  world.ArrayCreate
//----------------------------------------
static int L_ArrayCreate (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->ArrayCreate (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_ArrayCreate


//----------------------------------------
//  world.ArrayDelete
//----------------------------------------
static int L_ArrayDelete (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->ArrayDelete (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_ArrayDelete


//----------------------------------------
//  world.ArrayDeleteKey
//----------------------------------------
static int L_ArrayDeleteKey (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->ArrayDeleteKey (
                  my_checkstring (L, 1),  // Name
                  my_checkstring (L, 2)   // Key
                  ));
  return 1;  // number of result fields
  } // end of L_ArrayDeleteKey


//----------------------------------------
//  world.ArrayExists
//----------------------------------------
static int L_ArrayExists (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushboolean (L,  pDoc->ArrayExists (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_ArrayExists


//----------------------------------------
//  world.ArrayExport
//----------------------------------------
static int L_ArrayExport (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->ArrayExport (
            my_checkstring (L, 1),   // Name
            my_optstring (L, 2, ",") // Delimiter
            );
  return pushVariant (L, v);
  } // end of L_ArrayExport


//----------------------------------------
//  world.ArrayExportKeys
//----------------------------------------
static int L_ArrayExportKeys (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->ArrayExportKeys (
            my_checkstring (L, 1),   // Name
            my_optstring (L, 2, ",") // Delimiter
            );
  return pushVariant (L, v);
  } // end of L_ArrayExportKeys


//----------------------------------------
//  world.ArrayGet
//----------------------------------------
static int L_ArrayGet (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->ArrayGet (
            my_checkstring (L, 1),   // Name
            my_checkstring (L, 2)    // Key
            );
  return pushVariant (L, v);
  } // end of L_ArrayGet


//----------------------------------------
//  world.ArrayGetFirstKey
//----------------------------------------
static int L_ArrayGetFirstKey (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->ArrayGetFirstKey (my_checkstring (L, 1));
  return pushVariant (L, v);
  } // end of L_ArrayGetFirstKey


//----------------------------------------
//  world.ArrayGetLastKey
//----------------------------------------
static int L_ArrayGetLastKey (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->ArrayGetLastKey (my_checkstring (L, 1));
  return pushVariant (L, v);
  } // end of L_ArrayGetLastKey


//----------------------------------------
//  world.ArrayImport    
//  - extension - takes an optional table as 2nd argument
//----------------------------------------
static int L_ArrayImport (lua_State *L)
  {
  CMUSHclientDoc * pDoc = doc (L);  // must do this first

  string name = my_checkstring (L, 1);
  const int table = 2;    // 2nd argument may be table

  // extension here - accept table as 2nd argument
  if (lua_istable (L, table))
    {

    tStringMapOfMaps::iterator it = pDoc->GetArrayMap ().find (name);
                           
    // find table
    if (it == pDoc->GetArrayMap ().end ())
      {
      lua_pushnumber (L, eArrayDoesNotExist);
      return 1;
      }

    int iDuplicates = 0;

    // standard Lua table iteration
    for (lua_pushnil (L); lua_next (L, table) != 0; lua_pop (L, 1))
      {
      if (lua_type (L, -2) != LUA_TSTRING)
        luaL_error (L, "table must have string keys");

      // get key and value
      string sKey = lua_tostring (L, -2);
      string sValue = lua_tostring (L, -1);

      // insert into array
      pair<tStringToStringMap::iterator, bool> status = 
          it->second->insert (make_pair (sKey, sValue));

      // check if already there
      if (!status.second)
        {
        status.first->second = sValue;
        iDuplicates++;
        }

      } // end of looping through table

    if (iDuplicates)
      lua_pushnumber (L, eImportedWithDuplicates);
    else
      lua_pushnumber (L, eOK);
    return 1; // one result, which is error code

    }   // end of table for argument 2

  // not a table - normal string with delimiters
  lua_pushnumber (L, pDoc->ArrayImport (
                  name.c_str (),  // Name
                  my_checkstring (L, 2),  // Values
                  my_optstring (L, 3, ",")   // Delimiter
                  ));
  return 1;  // number of result fields
  } // end of L_ArrayImport


//----------------------------------------
//  world.ArrayKeyExists
//----------------------------------------
static int L_ArrayKeyExists (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushboolean (L,  pDoc->ArrayKeyExists (
            my_checkstring (L, 1),      // Name
            my_checkstring (L, 2)       // Key
            ));
  return 1;  // number of result fields
  } // end of L_ArrayKeyExists


//----------------------------------------
//  world.ArrayListAll
//----------------------------------------
static int L_ArrayListAll (lua_State *L)
  {
  VARIANT v = doc (L)->ArrayListAll ();
  return pushVariant (L, v);
  } // end of L_ArrayListAll


//----------------------------------------
//  world.ArrayListKeys
//----------------------------------------
static int L_ArrayListKeys (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->ArrayListKeys (my_checkstring (L, 1));
  return pushVariant (L, v);
  } // end of L_ArrayListKeys


//----------------------------------------
//  world.ArrayListValues
//----------------------------------------
static int L_ArrayListValues (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->ArrayListValues (my_checkstring (L, 1));
  return pushVariant (L, v);
  } // end of L_ArrayListValues

//----------------------------------------
//  world.ArrayList - Lua only
//----------------------------------------
static int L_ArrayList (lua_State *L)
  {
  CMUSHclientDoc * pDoc = doc (L);  // must do this first

  tStringMapOfMaps::iterator it = pDoc->GetArrayMap ().find (my_checkstring (L, 1));
  if (it == pDoc->GetArrayMap ().end ())
    return 0;        // not found, no results

  lua_newtable(L);                                                            
  for (tStringToStringMap::iterator i = it->second->begin (); 
       i != it->second->end ();
       i++)
       {
        lua_pushstring (L, i->first.c_str ());
        lua_pushstring (L, i->second.c_str ());
        lua_rawset(L, -3);
       }
  return 1;   // one table
  } // end of L_ArrayList


//----------------------------------------
//  world.ArraySet
//----------------------------------------
static int L_ArraySet (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->ArraySet (
                  my_checkstring (L, 1),  // Name
                  my_checkstring (L, 2),  // Key
                  my_checkstring (L, 3)   // Value
                  ));
  return 1;  // number of result fields
  } // end of L_ArraySet


//----------------------------------------
//  world.ArraySize
//----------------------------------------
static int L_ArraySize (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->ArraySize (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_ArraySize


//----------------------------------------
//  world.Base64Decode
//----------------------------------------
static int L_Base64Decode (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->Base64Decode (my_checkstring (L, 1));
  return pushVariant (L, v);
  } // end of L_Base64Decode


//----------------------------------------
//  world.Base64Encode
//----------------------------------------
static int L_Base64Encode (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->Base64Encode (
        my_checkstring (L, 1),  // Text
        optboolean (L, 2, 0)     // MultiLine
        );
  return pushVariant (L, v);
  } // end of L_Base64Encode

//----------------------------------------
//  world.BlendPixel
//----------------------------------------
static int L_BlendPixel (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->BlendPixel (
            (long) my_checknumber (L, 1),    // Blend
            (long) my_checknumber (L, 2),    // Base
            (short) my_checknumber (L, 3),    // Mode
            my_optnumber (L, 4, 1)    // Opacity
            ));
  return 1;  // number of result fields
  } // end of L_BlendPixel

//----------------------------------------
//  world.GetBoldColour
//----------------------------------------
static int L_GetBoldColour (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->GetBoldColour (
	        (short) my_checknumber (L, 1)
			));
  return 1;  // number of result fields
  } // end of GetBoldColour

//----------------------------------------
//  world.SetBoldColour
//----------------------------------------
static int L_SetBoldColour (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->SetBoldColour (
      (short) my_checknumber (L, 1),  // WhichColour
      (long) my_checknumber (L, 2)   // nNewValue
      );
  return 0;  // number of result fields
  } // end of SetBoldColour


//----------------------------------------
//  world.BroadcastPlugin
//----------------------------------------
static int L_BroadcastPlugin (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->BroadcastPlugin (
                  (long) my_checknumber (L, 1),  // PluginID
                  my_optstring   (L, 2, "")   // Argument - optional
                  ));
  return 1;  // number of result fields
  } // end of L_BroadcastPlugin


//----------------------------------------
//  world.CallPlugin
//----------------------------------------
static int L_CallPlugin (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);

  const char * sPluginID = my_checkstring (L, 1);
  const char * sRoutine = my_checkstring (L, 2);

  // remove plugin ID and function name
  // this is so, after the lua_pcall the stack should be empty
  // so, if the called function does a CallPlugin back to us, it won't matter
  // if we do a lua_settop (pL, 0) (below) to clear the stack

  lua_remove (L, 1);  // remove plugin ID
  lua_remove (L, 1);  // remove function name

  int i;    // for iterating through arguments / return values

  // preliminary checks ...

  // function name given?
  if (sRoutine [0] == 0)
    {
    lua_pushnumber (L, eNoSuchRoutine);
    lua_pushstring (L, "No function name supplied");
    return 2;     // eNoSuchRoutine, explanation
    }

  // plugin exists?
  CPlugin * pPlugin = pDoc->GetPlugin (sPluginID); 
  if (!pPlugin)
    {
    lua_pushnumber (L, eNoSuchPlugin);
    CString strError = TFormat ("Plugin ID (%s) is not installed",
                                sPluginID);
    lua_pushstring (L, strError);
    return 2;    // eNoSuchPlugin, explanation
    }

  // plugin is enabled?
  if (!pPlugin->m_bEnabled)
    {
    lua_pushnumber (L, ePluginDisabled);
    CString strError = TFormat ("Plugin '%s' (%s) disabled",
                      (LPCTSTR) pPlugin->m_strName, 
                      sPluginID);
    lua_pushstring (L, strError);
    return 2;     // ePluginDisabled, explanation

    }

  // plugin has a script engine?
  if (pPlugin->m_ScriptEngine == NULL)
    {
    lua_pushnumber (L, eNoSuchRoutine);
    CString strError = TFormat ("Scripting not enabled in plugin '%s' (%s)",
                                (LPCTSTR) pPlugin->m_strName, 
                                sPluginID);
    lua_pushstring (L, strError);
    return 2;     // eNoSuchRoutine, explanation

    }

  // new in 4.55 - Lua to Lua calls can handle multiple arguments and multiple return values

  // don't need to check *our* scripting language, we must be Lua, duh, or we wouldn't be here.

  if (pPlugin->m_ScriptEngine->IsLua ())
    {
    int n = lua_gettop(L);  // number of arguments in calling script (we removed plugin ID and function name already)

    lua_State *pL = pPlugin->m_ScriptEngine->L;  // plugin's Lua state

    // don't clear if we are calling ourselves
    if (pL != L)
      lua_settop (pL, 0);   // clear stack in target plugin

    // get wanted function onto stack
    if (!GetNestedFunction (pL, sRoutine, false))    // don't raise error
      {
      lua_pushnumber (L, eNoSuchRoutine);
      CString strError = TFormat ("No function '%s' in plugin '%s' (%s)",
                        sRoutine, 
                        (LPCTSTR) pPlugin->m_strName, 
                        sPluginID);
      lua_pushstring (L, strError);
      return 2;    // eNoSuchRoutine, explanation
      }
   
    // if we are calling ourselves, don't make a copy of everything
    if (pL == L)
      lua_insert (pL, 1);    // move function to be called as first item 
    else
      {   // calling a different plugin

      // copy all our arguments to destination script space
      // we can handle: nil, boolean, number, string
      // but NOT: table, function, userdata, thread

      // check we can push our arguments.
      // we need room for the function itself and at least room for the return value
      lua_checkstack (pL, n + 2);

      for (i = 1; i <= n; i++) 
        {
      
        switch (lua_type (L, i))
          {
          case LUA_TNIL:
            lua_pushnil (pL);
            break;

          case LUA_TBOOLEAN:
            lua_pushboolean (pL, lua_toboolean (L, i));
            break;

          case LUA_TNUMBER:
            lua_pushnumber (pL, lua_tonumber (L, i));
            break;

          case LUA_TSTRING:
            {
            size_t len;
            const char * s = lua_tolstring (L, i, &len);
            lua_pushlstring (pL, s, len);
            }
            break;

          // not one of those? can't handle it
          default:
            lua_settop (pL, 0);     // clear target plugin's stack to remove whatever we pushed onto it
            lua_pushnumber (L, eBadParameter);
            CString strError = TFormat ("Cannot pass argument #%i (%s type) to CallPlugin",
                                        i + 2,  // add two because we deleted plugin ID and function name
                                        luaL_typename (L, i));
            lua_pushstring (L, strError);
            return 2;    // eBadParameter, explanation

          } // end of switch on type of argument

        } // end of for each argument
      }   // end of not calling ourselves

    unsigned short iOldStyle = pDoc->m_iNoteStyle;
    pDoc->m_iNoteStyle = NORMAL;    // back to default style

    CString strOldCallingPluginID = pPlugin->m_strCallingPluginID;

    pPlugin->m_strCallingPluginID.Empty ();
    
    if (pDoc->m_CurrentPlugin)
      pPlugin->m_strCallingPluginID = pDoc->m_CurrentPlugin->m_strID;

    // do this so plugin can find its own state (eg. with GetPluginID)
    CPlugin * pSavedPlugin = pDoc->m_CurrentPlugin; 
    pDoc->m_CurrentPlugin = pPlugin;  
    
    // now call the routine in the plugin

    if (CallLuaWithTraceBack (pL, n, LUA_MULTRET))   // true on error
      {

      // here for execution error in plugin function ...

      CString strType = TFormat ("Plugin %s", (LPCTSTR) pPlugin->m_strName); 
      CString strReason = TFormat ("Executing plugin %s sub %s", 
                                   (LPCTSTR) pPlugin->m_strName,
                                   sRoutine ); 

      // grab the Lua error from the stack before we clear it
      CString strLuaError (lua_tostring(pL, -1));

      // this will display the error, and the error context
      LuaError (pL, "Run-time error", sRoutine, strType, strReason, pDoc);

      // back to who *we* are (had to wait until after LuaError)
      pDoc->m_CurrentPlugin = pSavedPlugin;
      pDoc->m_iNoteStyle = iOldStyle;

      lua_settop (pL, 0);     // clean stack up

      // the error code for the caller (result value 1)
      lua_pushnumber (L, eErrorCallingPluginRoutine);

      // a nice error message (result value 2)
      CString strError = TFormat ("Runtime error in function '%s', plugin '%s' (%s)",
                                  sRoutine,
                                  (LPCTSTR) pPlugin->m_strName, sPluginID);
      
      lua_pushstring (L, strError);

      // what the exact Lua error message was (result value 3)
      lua_pushstring (L, strLuaError);

      pPlugin->m_strCallingPluginID = strOldCallingPluginID;

      return 3;  // ie. eErrorCallingPluginRoutine, explanation, Lua error message
      }

    // back to who *we* are (if no error)
    pDoc->m_CurrentPlugin = pSavedPlugin;
    pDoc->m_iNoteStyle = iOldStyle;
    pPlugin->m_strCallingPluginID = strOldCallingPluginID;

    int ret_n = lua_gettop(pL);  // number of returned values (might be zero)

    lua_pushnumber (L, eOK);   // expected behaviour prior to 4.55 (just a single value)

    // if we are calling ourselves, don't make a copy of everything
    if (pL == L)
      {
      lua_insert (L, 1);    // put return code as first item pushing others up
      return 1 + ret_n;     // eOK plus all returned values
      }

    lua_checkstack (L, ret_n + 1);  // check we can push eOK plus all the return results

    // copy return results back to original script space
    // we can handle: nil, boolean, number, string
    // but NOT: table, function, userdata, thread

    for (i = 1; i <= ret_n; i++) 
      {
      
      switch (lua_type (pL, i))
        {
        case LUA_TNIL:
          lua_pushnil (L);
          break;

        case LUA_TBOOLEAN:
          lua_pushboolean (L, lua_toboolean (pL, i));
          break;

        case LUA_TNUMBER:
          lua_pushnumber (L, lua_tonumber (pL, i));
          break;

        case LUA_TSTRING:
          {
          size_t len;
          const char * s = lua_tolstring (pL, i, &len);
          lua_pushlstring (L, s, len);
          }
          break;

        // not one of those? can't handle it
        default:
          lua_pushnumber (L, eErrorCallingPluginRoutine);
          CString strError = CFormat ("Cannot handle return value #%i (%s type) from function '%s' in plugin '%s' (%s)",
                                      i, 
                                      luaL_typename (pL, i), 
                                      sRoutine,
                                      (LPCTSTR) pPlugin->m_strName, 
                                      sPluginID);
          lua_pushstring (L, strError);
          lua_settop (pL, 0);     // clean stack in plugin
          return 2;   // eErrorCallingPluginRoutine, explanation

        } // end of switch on type of argument

      } // end of for each argument

      return ret_n + 1;  // eOK plus all returned values
    }  // end if Lua calling Lua

  // ------------- end stuff added for version 4.55 -------------------

  // old fashioned way ...
  lua_pushnumber (L, pDoc->CallPlugin (
                  sPluginID,  // PluginID     - was argument 1 earlier on
                  sRoutine,   // Routine      - was argument 2 earlier on
                  my_optstring   (L, 1, "")   // Argument - optional (originally argument 3)
                  ));
  return 1;  // number of result fields
  } // end of L_CallPlugin

//----------------------------------------
//  world.ChangeDir
//----------------------------------------
static int L_ChangeDir (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->ChangeDir (
                  my_checkstring (L, 1)   // Directory
                  ));
  return 1;  // number of result fields
  } // end of L_ChangeDir


//----------------------------------------
//  world.ChatAcceptCalls
//----------------------------------------
static int L_ChatAcceptCalls (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->ChatAcceptCalls (
	              (short) my_optnumber (L, 1, DEFAULT_CHAT_PORT)
				  ));
  return 1;  // number of result fields
  } // end of L_ChatAcceptCalls


//----------------------------------------
//  world.ChatCall
//----------------------------------------
static int L_ChatCall (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->ChatCall (
                  my_checkstring (L, 1),  // Server
                  (long) my_optnumber (L, 2, DEFAULT_CHAT_PORT)
                  ));
  return 1;  // number of result fields
  } // end of L_ChatCall


//----------------------------------------
//  world.ChatCallzChat
//----------------------------------------
static int L_ChatCallzChat (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->ChatCallzChat (
                  my_checkstring (L, 1),  // Server
                  (long) my_optnumber (L, 2, DEFAULT_CHAT_PORT)
                  ));
  return 1;  // number of result fields
  } // end of L_ChatCallzChat


//----------------------------------------
//  world.ChatDisconnect
//----------------------------------------
static int L_ChatDisconnect (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->ChatDisconnect (
	              (long) my_checknumber (L, 1)
				  ));
  return 1;  // number of result fields
  } // end of L_ChatDisconnect


//----------------------------------------
//  world.ChatDisconnectAll
//----------------------------------------
static int L_ChatDisconnectAll (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->ChatDisconnectAll ());
  return 1;  // number of result fields
  } // end of L_ChatDisconnectAll


//----------------------------------------
//  world.ChatEverybody
//----------------------------------------
static int L_ChatEverybody (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->ChatEverybody (
                  my_checkstring (L, 1),  // Message
                  optboolean (L, 2, 0)  // Emote
                  ));
  return 1;  // number of result fields
  } // end of L_ChatEverybody


//----------------------------------------
//  world.ChatGetID
//----------------------------------------
static int L_ChatGetID (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->ChatGetID (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_ChatGetID


//----------------------------------------
//  world.ChatGroup
//----------------------------------------
static int L_ChatGroup (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->ChatGroup (
                  my_checkstring (L, 1),  // Group
                  my_checkstring (L, 2),  // Message
                  optboolean (L, 3, 0)  // Emote
                  ));
  return 1;  // number of result fields
  } // end of L_ChatGroup


//----------------------------------------
//  world.ChatID
//----------------------------------------
static int L_ChatID (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->ChatID (
                  (long)my_checknumber (L, 1),  // ID
                  my_checkstring (L, 2),  // Message
                  optboolean (L, 3, 0)  // Emote
                  ));
  return 1;  // number of result fields
  } // end of L_ChatID


//----------------------------------------
//  world.ChatMessage
//----------------------------------------
static int L_ChatMessage (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->ChatMessage (
                  (long) my_checknumber (L, 1),  // ID
                  (short) my_checknumber (L, 2),  // Message code
                  my_optstring (L, 3, "")  // Text - optional
                  ));
  return 1;  // number of result fields
  } // end of L_ChatMessage


//----------------------------------------
//  world.ChatNameChange
//----------------------------------------
static int L_ChatNameChange (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->ChatNameChange (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_ChatNameChange


//----------------------------------------
//  world.ChatNote
//----------------------------------------
static int L_ChatNote (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->ChatNote (
          (short) my_checknumber (L, 1),  // NoteType
          my_optstring (L, 2, "")   // Message - optional
          );
  return 0;  // number of result fields
  } // end of L_ChatNote


//----------------------------------------
//  world.ChatPasteEverybody
//----------------------------------------
static int L_ChatPasteEverybody (lua_State *L)
  {
  lua_pushnumber (L, doc (L)->ChatPasteEverybody ());
  return 1;  // number of result fields
  } // end of L_ChatPasteEverybody


//----------------------------------------
//  world.ChatPasteText
//----------------------------------------
static int L_ChatPasteText (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->ChatPasteText (
	              (long) my_checknumber (L, 1)
				  ));
  return 1;  // number of result fields
  } // end of L_ChatPasteText


//----------------------------------------
//  world.ChatPeekConnections
//----------------------------------------
static int L_ChatPeekConnections (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->ChatPeekConnections (
	              (long) my_checknumber (L, 1)
				  ));
  return 1;  // number of result fields
  } // end of L_ChatPeekConnections


//----------------------------------------
//  world.ChatPersonal
//----------------------------------------
static int L_ChatPersonal (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->ChatPersonal (
                  my_checkstring (L, 1),  // Who
                  my_checkstring (L, 2),  // Message
                  optboolean (L, 3, 0)  // Emote
                  ));
  return 1;  // number of result fields
  } // end of L_ChatPersonal


//----------------------------------------
//  world.ChatPing
//----------------------------------------
static int L_ChatPing (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->ChatPing (
	              (long) my_checknumber (L, 1)
				  ));
  return 1;  // number of result fields
  } // end of L_ChatPing


//----------------------------------------
//  world.ChatRequestConnections
//----------------------------------------
static int L_ChatRequestConnections (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->ChatRequestConnections (
	              (long) my_checknumber (L, 1)));
  return 1;  // number of result fields
  } // end of L_ChatRequestConnections


//----------------------------------------
//  world.ChatSendFile
//----------------------------------------
static int L_ChatSendFile (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->ChatSendFile (
                  (long) my_checknumber (L, 1),  // ID
                  my_optstring (L, 2, "")   // FileName
                  ));
  return 1;  // number of result fields
  } // end of L_ChatSendFile


//----------------------------------------
//  world.ChatStopAcceptingCalls
//----------------------------------------
static int L_ChatStopAcceptingCalls (lua_State *L)
  {
  doc (L)->ChatStopAcceptingCalls ();
  return 0;  // number of result fields
  } // end of L_ChatStopAcceptingCalls


//----------------------------------------
//  world.ChatStopFileTransfer
//----------------------------------------
static int L_ChatStopFileTransfer (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->ChatStopFileTransfer (
	              (long) my_checknumber (L, 1)
				  ));
  return 1;  // number of result fields
  } // end of L_ChatStopFileTransfer

//----------------------------------------
//  world.CloseLog
//----------------------------------------
static int L_CloseLog (lua_State *L)
  {
  lua_pushnumber (L, doc (L)->CloseLog ());
  return 1;  // number of result fields
  } // end of L_CloseLog


//----------------------------------------
//  world.CloseNotepad
//----------------------------------------
static int L_CloseNotepad (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->CloseNotepad (
                  my_checkstring (L, 1),  // Title
                  optboolean (L, 2, 0)  // QuerySave
                  ));
  return 1;  // number of result fields
  } // end of L_CloseNotepad


//----------------------------------------
//  world.ColourNameToRGB
//----------------------------------------
static int L_ColourNameToRGB (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->ColourNameToRGB (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_ColourNameToRGB


//----------------------------------------
//  world.ColourNote
// extension - takes groups of 3 arguments 
//----------------------------------------
static int L_ColourNote (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  int n = lua_gettop(L);  /* number of arguments */
  int i;
  for (i=1; i<=n; i += 3) 
    {
    if ((i + 2) < n)    // not end of line yet
      pDoc->ColourTell (
            my_checkstring (L, i),       // TextColour
            my_checkstring (L, i + 1),   // BackgroundColour
            my_checkstring (L, i + 2)    // Text
                          );
    else
      pDoc->ColourNote (
            my_checkstring (L, i),       // TextColour
            my_checkstring (L, i + 1),   // BackgroundColour
            my_checkstring (L, i + 2)    // Text
                          );
    }
  return 0;  // number of result fields
  } // end of L_ColourNote


//----------------------------------------
//  world.ColourTell
// extension - takes groups of 3 arguments 
//----------------------------------------
static int L_ColourTell (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  int n = lua_gettop(L);  /* number of arguments */
  int i;
  for (i=1; i<=n; i += 3) 
    pDoc->ColourTell (
          my_checkstring (L, i),       // TextColour
          my_checkstring (L, i + 1),   // BackgroundColour
          my_checkstring (L, i + 2)    // Text
                      );
  return 0;  // number of result fields
  } // end of L_ColourTell


//----------------------------------------
//  world.Connect
//----------------------------------------
static int L_Connect (lua_State *L)
  {
  lua_pushnumber (L, doc (L)->Connect ());
  return 1;  // number of result fields
  } // end of L_Connect


//----------------------------------------
//  world.CreateGUID
//----------------------------------------
static int L_CreateGUID (lua_State *L)
  {
  BSTR str = doc (L)->CreateGUID ();
  return pushBstr (L, str);  // number of result fields
  } // end of L_CreateGUID


//----------------------------------------
//  world.SetCursor
//----------------------------------------
static int L_SetCursor (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->SetCursor (
            (long) my_optnumber (L, 1, 0)    // Cursor
            ));
  return 1;  // number of result fields
  } // end of L_SetCursor

//----------------------------------------
//  world.GetCustomColourBackground
//----------------------------------------
static int L_GetCustomColourBackground (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->GetCustomColourBackground (
	              (short) my_checknumber (L, 1)
				  ));
  return 1;  // number of result fields
  } // end of L_GetCustomColourBackground

//----------------------------------------
//  world.SetCustomColourBackground
//----------------------------------------
static int L_SetCustomColourBackground (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->SetCustomColourBackground (
      (short) my_checknumber (L, 1),  // WhichColour
      (long) my_checknumber (L, 2)   // nNewValue
      );
  return 0;  // number of result fields
  } // end of L_SetCustomColourBackground


//----------------------------------------
//  world.GetCustomColourText
//----------------------------------------
static int L_GetCustomColourText (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->GetCustomColourText (
	              (short) my_checknumber (L, 1)
				  ));
  return 1;  // number of result fields
  } // end of L_GetCustomColourText

//----------------------------------------
//  world.SetCustomColourText
//----------------------------------------
static int L_SetCustomColourText (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->SetCustomColourText (
      (short) my_checknumber (L, 1),  // WhichColour
      (long) my_checknumber (L, 2)    // nNewValue
      );
  return 0;  // number of result fields
  } // end of L_SetCustomColourText


//----------------------------------------
//  world.DatabaseOpen
//----------------------------------------
static int L_DatabaseOpen (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->DatabaseOpen (
      my_checkstring (L, 1),  // Name
      my_checkstring (L, 2),  // FileName
      (long) my_optnumber (L, 3, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE)  // Flags
      ));
  return 1;  // number of result fields
  } // end of L_DatabaseOpen

//----------------------------------------
//  world.DatabaseClose
//----------------------------------------
static int L_DatabaseClose (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->DatabaseClose (
      my_checkstring (L, 1)  // Name
      ));
  return 1;  // number of result fields
  } // end of L_DatabaseClose

//----------------------------------------
//  world.DatabasePrepare
//----------------------------------------
static int L_DatabasePrepare (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->DatabasePrepare (
      my_checkstring (L, 1),  // Name
      my_checkstring (L, 2)   // Sql
      ));
  return 1;  // number of result fields
  } // end of L_DatabasePrepare

//----------------------------------------
//  world.DatabaseFinalize
//----------------------------------------
static int L_DatabaseFinalize (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->DatabaseFinalize (
      my_checkstring (L, 1)  // Name
      ));
  return 1;  // number of result fields
  } // end of L_DatabaseFinalize

//----------------------------------------
//  world.DatabaseReset
//----------------------------------------
static int L_DatabaseReset (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->DatabaseReset (
      my_checkstring (L, 1)  // Name
      ));
  return 1;  // number of result fields
  } // end of L_DatabaseReset

//----------------------------------------
//  world.DatabaseColumns
//----------------------------------------
static int L_DatabaseColumns (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->DatabaseColumns (
      my_checkstring (L, 1)  // Name
      ));
  return 1;  // number of result fields
  } // end of L_DatabaseColumns

//----------------------------------------
//  world.DatabaseStep
//----------------------------------------
static int L_DatabaseStep (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->DatabaseStep (
      my_checkstring (L, 1)  // Name
      ));
  return 1;  // number of result fields
  } // end of L_DatabaseStep

//----------------------------------------
//  world.DatabaseError
//----------------------------------------
static int L_DatabaseError (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  BSTR str = pDoc->DatabaseError (my_checkstring (L, 1));
  return pushBstr (L, str);  // number of result fields
  } // end of L_DatabaseError

//----------------------------------------
//  world.DatabaseColumnName
//----------------------------------------
static int L_DatabaseColumnName (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  BSTR str = pDoc->DatabaseColumnName (
      my_checkstring (L, 1),         // Name
      (long) my_checknumber (L, 2)   // Column
      );
  return pushBstr (L, str);  // number of result fields
  } // end of L_DatabaseColumnName

//----------------------------------------
//  world.DatabaseColumnText
//----------------------------------------
static int L_DatabaseColumnText (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  BSTR str = pDoc->DatabaseColumnText (
      my_checkstring (L, 1),         // Name
      (long) my_checknumber (L, 2)   // Column
      );
  return pushBstr (L, str);  // number of result fields
  } // end of L_DatabaseColumnText

//----------------------------------------
//  world.DatabaseColumnValue
//----------------------------------------
static int L_DatabaseColumnValue (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->DatabaseColumnValue (
      my_checkstring (L, 1),         // Name
      (long) my_checknumber (L, 2)   // Column
      );
  return pushVariant (L, v);  // number of result fields
  } // end of L_DatabaseColumnValue

//----------------------------------------
//  world.DatabaseColumnType
//----------------------------------------
static int L_DatabaseColumnType (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->DatabaseColumnType (
      my_checkstring (L, 1),         // Name
      (long) my_checknumber (L, 2)   // Column
      ));
  return 1;  // number of result fields
  } // end of L_DatabaseColumnType

//----------------------------------------
//  world.DatabaseTotalChanges
//----------------------------------------
static int L_DatabaseTotalChanges (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->DatabaseTotalChanges (
      my_checkstring (L, 1)  // Name
      ));
  return 1;  // number of result fields
  } // end of L_DatabaseTotalChanges

//----------------------------------------
//  world.DatabaseChanges
//----------------------------------------
static int L_DatabaseChanges (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->DatabaseChanges (
      my_checkstring (L, 1)  // Name
      ));
  return 1;  // number of result fields
  } // end of L_DatabaseChanges


//----------------------------------------
//  world.DatabaseLastInsertRowid
//----------------------------------------
static int L_DatabaseLastInsertRowid (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  BSTR str = pDoc->DatabaseLastInsertRowid (
      my_checkstring (L, 1)  // Name
      );
  return pushBstr (L, str);  // number of result fields
  } // end of L_DatabaseLastInsertRowid

//----------------------------------------
//  world.DatabaseList
//----------------------------------------
static int L_DatabaseList (lua_State *L)
  {
  VARIANT v = doc (L)->DatabaseList ();
  return pushVariant (L, v);  // number of result fields
  } // end of L_DatabaseList

//----------------------------------------
//  world.DatabaseInfo
//----------------------------------------
static int L_DatabaseInfo (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->DatabaseInfo (
      my_checkstring (L, 1),           // Name
      (long) my_checknumber (L, 2)     // InfoType
    );

  return pushVariant (L, v);
  } // end of L_DatabaseInfo

//----------------------------------------
//  world.DatabaseExec
//----------------------------------------
static int L_DatabaseExec (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->DatabaseExec (
      my_checkstring (L, 1),  // Name
      my_checkstring (L, 2)   // Sql
      ));
  return 1;  // number of result fields
  } // end of L_DatabaseExec

//----------------------------------------
//  world.DatabaseColumnNames
//----------------------------------------
static int L_DatabaseColumnNames (lua_State *L)
  {
  VARIANT v = doc (L)->DatabaseColumnNames (my_checkstring (L, 1));
  return pushVariant (L, v);  // number of result fields
  } // end of L_DatabaseColumnNames

//----------------------------------------
//  world.DatabaseColumnValues
//----------------------------------------
static int L_DatabaseColumnValues (lua_State *L)
  {
  VARIANT v = doc (L)->DatabaseColumnValues (my_checkstring (L, 1));
  return pushVariant (L, v);  // number of result fields
  } // end of L_DatabaseColumnValues


//----------------------------------------
//  world.Debug
//----------------------------------------
static int L_Debug (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->Debug (my_optstring (L, 1, ""));
  return pushVariant (L, v);  // number of result fields
  } // end of L_Debug


//----------------------------------------
//  world.DeleteAlias
//----------------------------------------
static int L_DeleteAlias (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->DeleteAlias (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_DeleteAlias


//----------------------------------------
//  world.DeleteAliasGroup
//----------------------------------------
static int L_DeleteAliasGroup (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->DeleteAliasGroup (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_DeleteAliasGroup


//----------------------------------------
//  world.DeleteAllMapItems
//----------------------------------------
static int L_DeleteAllMapItems (lua_State *L)
  {
  lua_pushnumber (L, doc (L)->DeleteAllMapItems ());
  return 1;  // number of result fields
  } // end of L_DeleteAllMapItems


//----------------------------------------
//  world.DeleteCommandHistory
//----------------------------------------
static int L_DeleteCommandHistory (lua_State *L)
  {
  doc (L)->DeleteCommandHistory ();
  return 0;  // number of result fields
  } // end of L_DeleteCommandHistory


//----------------------------------------
//  world.DeleteGroup
//----------------------------------------
static int L_DeleteGroup (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->DeleteGroup (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_DeleteGroup


//----------------------------------------
//  world.DeleteLastMapItem
//----------------------------------------
static int L_DeleteLastMapItem (lua_State *L)
  {
  lua_pushnumber (L, doc (L)->DeleteLastMapItem ());
  return 1;  // number of result fields
  } // end of L_DeleteLastMapItem

//----------------------------------------
//  world.DeleteLines
//----------------------------------------
static int L_DeleteLines (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->DeleteLines ((long) my_checknumber (L, 1));
  return 0;  // number of result fields
  } // end of L_DeleteLines

//----------------------------------------
//  world.DeleteOutput
//----------------------------------------
static int L_DeleteOutput (lua_State *L)
  {
  doc (L)->DeleteOutput ();
  return 0;  // number of result fields
  } // end of L_DeleteOutput


//----------------------------------------
//  world.DeleteTemporaryAliases
//----------------------------------------
static int L_DeleteTemporaryAliases (lua_State *L)
  {
  lua_pushnumber (L, doc (L)->DeleteTemporaryAliases ());
  return 1;  // number of result fields
  } // end of L_DeleteTemporaryAliases


//----------------------------------------
//  world.DeleteTemporaryTimers
//----------------------------------------
static int L_DeleteTemporaryTimers (lua_State *L)
  {
  lua_pushnumber (L, doc (L)->DeleteTemporaryTimers ());
  return 1;  // number of result fields
  } // end of L_DeleteTemporaryTimers


//----------------------------------------
//  world.DeleteTemporaryTriggers
//----------------------------------------
static int L_DeleteTemporaryTriggers (lua_State *L)
  {
  lua_pushnumber (L, doc (L)->DeleteTemporaryTriggers ());
  return 1;  // number of result fields
  } // end of L_DeleteTemporaryTriggers


//----------------------------------------
//  world.DeleteTimer
//----------------------------------------
static int L_DeleteTimer (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->DeleteTimer (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_DeleteTimer


//----------------------------------------
//  world.DeleteTimerGroup
//----------------------------------------
static int L_DeleteTimerGroup (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->DeleteTimerGroup (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_DeleteTimerGroup


//----------------------------------------
//  world.DeleteTrigger
//----------------------------------------
static int L_DeleteTrigger (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->DeleteTrigger (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_DeleteTrigger


//----------------------------------------
//  world.DeleteTriggerGroup
//----------------------------------------
static int L_DeleteTriggerGroup (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->DeleteTriggerGroup (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_DeleteTriggerGroup


//----------------------------------------
//  world.DeleteVariable
//----------------------------------------
static int L_DeleteVariable (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->DeleteVariable (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_DeleteVariable


//----------------------------------------
//  world.DiscardQueue
//----------------------------------------
static int L_DiscardQueue (lua_State *L)
  {
  lua_pushnumber (L, doc (L)->DiscardQueue ());
  return 1;  // number of result fields
  } // end of L_DiscardQueue


//----------------------------------------
//  world.Disconnect
//----------------------------------------
static int L_Disconnect (lua_State *L)
  {
  lua_pushnumber (L, doc (L)->Disconnect ());
  return 1;  // number of result fields
  } // end of L_Disconnect


//----------------------------------------
//  world.DoAfter
//----------------------------------------
static int L_DoAfter (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->DoAfter (
                  my_checknumber (L, 1), // Seconds
                  my_checkstring (L, 2)  // SendText
                  ));
  return 1;  // number of result fields
  } // end of L_DoAfter


//----------------------------------------
//  world.DoAfterNote
//----------------------------------------
static int L_DoAfterNote (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->DoAfterNote (
                  my_checknumber (L, 1), // Seconds
                  my_checkstring (L, 2)  // NoteText
                  ));
  return 1;  // number of result fields
  } // end of L_DoAfterNote


//----------------------------------------
//  world.DoAfterSpecial
//----------------------------------------
static int L_DoAfterSpecial (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->DoAfterSpecial (
                  my_checknumber (L, 1),  // Seconds
                  my_checkstring (L, 2),  // NoteText
                  (short) my_checknumber (L, 3)   // SendTo
                  ));
  return 1;  // number of result fields
  } // end of L_DoAfterSpecial


//----------------------------------------
//  world.DoAfterSpeedWalk
//----------------------------------------
static int L_DoAfterSpeedWalk (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->DoAfterSpeedWalk (
                  my_checknumber (L, 1), // Seconds
                  my_checkstring (L, 2)  // SendText
                  ));
  return 1;  // number of result fields
  } // end of L_DoAfterSpeedWalk


//----------------------------------------
//  world.DoCommand
//----------------------------------------
static int L_DoCommand (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->DoCommand (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_DoCommand


//----------------------------------------
//  world.GetEchoInput
//----------------------------------------
static int L_GetEchoInput (lua_State *L)
  {
  lua_pushboolean (L, doc (L)->GetEchoInput ());
  return 1;  // number of result fields
  } // end of L_GetEchoInput

//----------------------------------------
//  world.SetEchoInput
//----------------------------------------
static int L_SetEchoInput (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->SetEchoInput (optboolean (L, 1, 1));  // defaults to yes
  return 0;  // number of result fields
  } // end of L_SetEchoInput

//----------------------------------------
//  world.EditDistance
//----------------------------------------
static int L_EditDistance (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->EditDistance (
      my_checkstring (L, 1),   // source
      my_checkstring (L, 2)    // target
      ));
  return 1;  // number of result fields
  } // end of L_EditDistance


//----------------------------------------
//  world.EnableAlias
//----------------------------------------
static int L_EnableAlias (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->EnableAlias (
      my_checkstring (L, 1),    // alias
      optboolean (L, 2, 1)    // enabled flag, defaults to true
      ));
  return 1;  // number of result fields
  } // end of L_EnableAlias


//----------------------------------------
//  world.EnableAliasGroup
//----------------------------------------
static int L_EnableAliasGroup (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->EnableAliasGroup (
      my_checkstring (L, 1),    // Group
      optboolean (L, 2, 1)    // enabled flag, defaults to true
      ));
  return 1;  // number of result fields
  } // end of L_EnableAliasGroup


//----------------------------------------
//  world.EnableGroup
//----------------------------------------
static int L_EnableGroup (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->EnableGroup (
      my_checkstring (L, 1),    // Group
      optboolean (L, 2, 1)    // enabled flag, defaults to true
      ));
  return 1;  // number of result fields
  } // end of L_EnableGroup


//----------------------------------------
//  world.EnableMapping
//----------------------------------------
static int L_EnableMapping (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->EnableMapping (optboolean (L, 1, 1));
  return 0;  // number of result fields
  } // end of L_EnableMapping


//----------------------------------------
//  world.EnablePlugin
//----------------------------------------
static int L_EnablePlugin (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->EnablePlugin (
      my_checkstring (L, 1),    // Plugin ID
      optboolean (L, 2, 1)    // enabled flag, defaults to true
      ));
  return 1;  // number of result fields
  } // end of L_EnablePlugin


//----------------------------------------
//  world.EnableTimer
//----------------------------------------
static int L_EnableTimer (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->EnableTimer (
      my_checkstring (L, 1),    // Timer
      optboolean (L, 2, 1)    // enabled flag, defaults to true
      ));
  return 1;  // number of result fields
  } // end of L_EnableTimer


//----------------------------------------
//  world.EnableTimerGroup
//----------------------------------------
static int L_EnableTimerGroup (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->EnableTimerGroup (
      my_checkstring (L, 1),    // Group
      optboolean (L, 2, 1)    // enabled flag, defaults to true
      ));
  return 1;  // number of result fields
  } // end of L_EnableTimerGroup


//----------------------------------------
//  world.EnableTrigger
//----------------------------------------
static int L_EnableTrigger (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->EnableTrigger (
      my_checkstring (L, 1),    // Trigger
      optboolean (L, 2, 1)    // enabled flag, defaults to true
      ));
  return 1;  // number of result fields
  } // end of L_EnableTrigger


//----------------------------------------
//  world.EnableTriggerGroup
//----------------------------------------
static int L_EnableTriggerGroup (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->EnableTriggerGroup (
      my_checkstring (L, 1),    // Group
      optboolean (L, 2, 1)    // enabled flag, defaults to true
      ));
  return 1;  // number of result fields
  } // end of L_EnableTriggerGroup


//----------------------------------------
//  world.ErrorDesc
//----------------------------------------
static int L_ErrorDesc (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  BSTR str = pDoc->ErrorDesc ((long) my_checknumber (L, 1));
  return pushBstr (L, str);  // number of result fields
  } // end of L_ErrorDesc

//----------------------------------------
//  world.EvaluateSpeedwalk
//----------------------------------------
static int L_EvaluateSpeedwalk (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  // this is a helper function that returns a CString, not a BSTR
  lua_pushstring (L, pDoc->DoEvaluateSpeedwalk (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_EvaluateSpeedwalk


//----------------------------------------
//  world.Execute
//----------------------------------------
static int L_Execute (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->Execute (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_Execute


//----------------------------------------
//  world.ExportXML
//----------------------------------------
static int L_ExportXML (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  BSTR str = pDoc->ExportXML (
                  (short) my_checknumber (L, 1), // Type
                  my_checkstring (L, 2)          // Name
                  );
  return pushBstr (L, str);  // number of result fields
  } // end of L_ExportXML


//----------------------------------------
//  world.FilterPixel
//----------------------------------------
static int L_FilterPixel (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->FilterPixel (
            (long) my_checknumber (L, 1),     // Pixel
            (short) my_checknumber (L, 2),    // Operation
            my_checknumber (L, 3)             // Options
            ));
  return 1;  // number of result fields
  } // end of L_FilterPixel

//----------------------------------------
//  world.FixupEscapeSequences
//----------------------------------------
static int L_FixupEscapeSequences (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  BSTR str = pDoc->FixupEscapeSequences (
                  my_checkstring (L, 1)  // Source
                  );
  return pushBstr (L, str);  // number of result fields
  } // end of L_FixupEscapeSequences


//----------------------------------------
//  world.FixupHTML
//----------------------------------------
static int L_FixupHTML (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  BSTR str = pDoc->FixupHTML (
                  my_checkstring (L, 1)  // StringToConvert
                  );
  return pushBstr (L, str);  // number of result fields
  } // end of L_FixupHTML

//----------------------------------------
//  world.FlashIcon
//----------------------------------------
static int L_FlashIcon (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->FlashIcon ();
  return 0;  // number of result fields
  } // end of L_FlashIcon

//----------------------------------------
//  world.FlushLog
//----------------------------------------
static int L_FlushLog (lua_State *L)
  {
  lua_pushnumber (L, doc (L)->FlushLog ());
  return 1;  // number of result fields
  } // end of L_FlushLog


//----------------------------------------
//  world.GenerateName
//----------------------------------------
static int L_GenerateName (lua_State *L)
  {
  VARIANT v = doc (L)->GenerateName ();
  return pushVariant (L, v);  // number of result fields
  } // end of L_GenerateName


//----------------------------------------
//  world.GetAlias
//
// Unlike the normal version this takes the alias name and
//  returns 5 fields: result, match, response, flags, script name
//----------------------------------------
static int L_GetAlias (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);

  VARIANT MatchText, 
          ResponseText, 
          Parameter, 
          Flags, 
          ScriptName;

  long result = pDoc->GetAlias(my_checkstring (L, 1),  // name
                                  &MatchText, 
                                  &ResponseText, 
                                  &Parameter,    // not used these days
                                  &Flags, 
                                  &ScriptName); 
  lua_pushnumber (L, result);         // 1
  pushVariant (L, MatchText);         // 2
  pushVariant (L, ResponseText);      // 3
  pushVariant (L, Flags);             // 4
  pushVariant (L, ScriptName);        // 5

  return 5;  // number of result fields
  } // end of L_GetAlias


//----------------------------------------
//  world.GetAliasInfo
//----------------------------------------
static int L_GetAliasInfo (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->GetAliasInfo (
      my_checkstring (L, 1),          // Name
      (short) my_checknumber (L, 2)   // Type
    );
  return pushVariant (L, v);  // number of result fields
  } // end of L_GetAliasInfo


//----------------------------------------
//  world.GetAliasList
//----------------------------------------
static int L_GetAliasList (lua_State *L)
  {
  VARIANT v = doc (L)->GetAliasList ();
  return pushVariant (L, v);  // number of result fields
  } // end of L_GetAliasList


//----------------------------------------
//  world.GetAliasOption
//----------------------------------------
static int L_GetAliasOption (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->GetAliasOption (
      my_checkstring (L, 1),    // Name
      my_checkstring (L, 2)     // Option name
    );
  return pushVariant (L, v);  // number of result fields
  } // end of L_GetAliasOption


//----------------------------------------
//  world.GetAliasWildcard
//----------------------------------------
static int L_GetAliasWildcard (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->GetAliasWildcard (
      my_checkstring (L, 1),    // Name
      my_checkstring (L, 2)     // Wildcard name
    );
  return pushVariant (L, v);  // number of result fields
  } // end of L_GetAliasWildcard


//----------------------------------------
//  world.GetAlphaOption
//----------------------------------------
static int L_GetAlphaOption (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->GetAlphaOption (
      my_checkstring (L, 1) // Name
    );
  return pushVariant (L, v);  // number of result fields
  } // end of L_GetAlphaOption


//----------------------------------------
//  world.GetAlphaOptionList
//----------------------------------------
static int L_GetAlphaOptionList (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->GetAlphaOptionList ();
  return pushVariant (L, v);  // number of result fields
  } // end of L_GetAlphaOptionList


//----------------------------------------
//  world.GetChatInfo
//----------------------------------------
static int L_GetChatInfo (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->GetChatInfo (
      (long) my_checknumber (L, 1),     // ChatID 
      (short) my_checknumber (L, 2)     // InfoType
    );
  return pushVariant (L, v);  // number of result fields
  } // end of L_GetChatInfo


//----------------------------------------
//  world.GetChatList
//----------------------------------------
static int L_GetChatList (lua_State *L)
  {
  VARIANT v = doc (L)->GetChatList ();
  return pushVariant (L, v);  // number of result fields
  } // end of L_GetChatList


//----------------------------------------
//  world.GetChatOption
//----------------------------------------
static int L_GetChatOption (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->GetChatOption (
      (long) my_checknumber (L, 1),    // ChatID 
      my_checkstring (L, 2)            // OptionName
    );
  return pushVariant (L, v);  // number of result fields
  } // end of L_GetChatOption


//----------------------------------------
//  world.GetClipboard
//----------------------------------------
static int L_GetClipboard (lua_State *L)
  {
  BSTR str = doc (L)->GetClipboard ();
  return pushBstr (L, str);  // number of result fields
  } // end of L_GetClipboard


//----------------------------------------
//  world.GetCommand
//----------------------------------------
static int L_GetCommand (lua_State *L)
  {
  BSTR str = doc (L)->GetCommand ();
  return pushBstr (L, str);  // number of result fields
  } // end of L_GetCommand


//----------------------------------------
//  world.GetCommandList
//----------------------------------------
static int L_GetCommandList (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->GetCommandList (
      (long) my_checknumber (L, 1) // Count
    );
  return pushVariant (L, v);  // number of result fields
  } // end of L_GetCommandList


//----------------------------------------
//  world.GetConnectDuration
//----------------------------------------
static int L_GetConnectDuration (lua_State *L)
  {
  lua_pushnumber (L, doc (L)->GetConnectDuration ());
  return 1;  // number of result fields
  } // end of L_GetConnectDuration


//----------------------------------------
//  world.GetCurrentValue
//----------------------------------------
static int L_GetCurrentValue (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->GetCurrentValue (
            my_checkstring (L, 1)   // OptionName
            );
  return pushVariant (L, v);
  } // end of L_GetCurrentValue


//----------------------------------------
//  world.GetCustomColourName
//----------------------------------------
static int L_GetCustomColourName (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);

  BSTR str = pDoc->GetCustomColourName (
      (short) my_checknumber (L, 1)  // WhichColour
      );
  return pushBstr (L, str);  // number of result fields
  } // end of L_GetCustomColourName

//----------------------------------------
//  world.GetDefaultValue
//----------------------------------------
static int L_GetDefaultValue (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->GetDefaultValue (
            my_checkstring (L, 1)   // OptionName
            );
  return pushVariant (L, v);
  } // end of L_GetDefaultValue

//----------------------------------------
//  world.GetDeviceCaps
//----------------------------------------
static int L_GetDeviceCaps (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->GetDeviceCaps ((long) my_checknumber (L, 1)));
  return 1;  // number of result fields
  } // end of L_GetDeviceCaps

//----------------------------------------
//  world.GetEntity
//----------------------------------------
static int L_GetEntity (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  BSTR str = pDoc->GetEntity (my_checkstring (L, 1));
  return pushBstr (L, str);  // number of result fields
  } // end of L_GetEntity

//----------------------------------------
//  world.GetXMLEntity
//----------------------------------------
static int L_GetXMLEntity (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  BSTR str = pDoc->GetXMLEntity (my_checkstring (L, 1));
  return pushBstr (L, str);  // number of result fields
  } // end of L_GetXMLEntity

//----------------------------------------
//  world.GetFrame
//----------------------------------------
static int L_GetFrame (lua_State *L)
  {
  lua_pushlightuserdata (L, (void *) doc (L)->GetFrame ());
  return 1;  // number of result fields
  } // end of L_GetFrame

//----------------------------------------
//  world.GetGlobalOption
//----------------------------------------
static int L_GetGlobalOption (lua_State *L)
  {
  VARIANT v = doc (L)->GetGlobalOption (my_checkstring (L, 1));
  return pushVariant (L, v);
  } // end of L_GetGlobalOption


//----------------------------------------
//  world.GetGlobalOptionList
//----------------------------------------
static int L_GetGlobalOptionList (lua_State *L)
  {
  VARIANT v = doc (L)->GetGlobalOptionList ();
  return pushVariant (L, v);
  } // end of L_GetGlobalOptionList

//----------------------------------------
//  world.GetHostAddress
//----------------------------------------
static int L_GetHostAddress (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->GetHostAddress (
            my_checkstring (L, 1)   // HostName
            );
  return pushVariant (L, v);
  } // end of L_GetHostAddress


//----------------------------------------
//  world.GetHostName
//----------------------------------------
static int L_GetHostName (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  BSTR str = pDoc->GetHostName (my_checkstring (L, 1));
  return pushBstr (L, str);  // number of result fields
  } // end of L_GetHostName


//----------------------------------------
//  world.GetInfo
//----------------------------------------
static int L_GetInfo (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->GetInfo (
            (long) my_checknumber (L, 1)   // InfoType
            );
  return pushVariant (L, v);
  } // end of L_GetInfo


//----------------------------------------
//  world.GetInternalCommandsList
//----------------------------------------
static int L_GetInternalCommandsList (lua_State *L)
  {
  VARIANT v = doc (L)->GetInternalCommandsList ();
  return pushVariant (L, v);
  } // end of L_GetInternalCommandsList


//----------------------------------------
//  world.GetLineCount
//----------------------------------------
static int L_GetLineCount (lua_State *L)
  {
  lua_pushnumber (L, doc (L)->GetLineCount ());
  return 1;  // number of result fields
  } // end of L_GetLineCount


//----------------------------------------
//  world.GetLineInfo
// extension - for info type 0 or omitted, returns a table
//----------------------------------------
static int L_GetLineInfo (lua_State *L)
  {
  CMUSHclientDoc * pDoc = doc (L);  // must do this first
  int iLine = (int) my_checknumber (L, 1);
  int iType = (int) my_optnumber (L, 2, 0);
  
  if (iType == 0)
    {
    // check line exists
    if (iLine <= 0 || iLine > pDoc->m_LineList.GetCount ())
      return 0;     // no result for no line

  // get pointer to line in question

    CLine * pLine = pDoc->m_LineList.GetAt (pDoc->GetLinePosition (iLine - 1));

    lua_newtable(L);                                                            
    MakeTableItem     (L, "text",     CString (pLine->text, pLine->len)); // 1
    MakeTableItem     (L, "length",   pLine->len); // 2
    MakeTableItemBool (L, "newline",  pLine->hard_return); // 3
    MakeTableItemBool (L, "note",     (pLine->flags & COMMENT) != 0); // 4
    MakeTableItemBool (L, "user",     (pLine->flags & USER_INPUT) != 0); // 5
    MakeTableItemBool (L, "log",      (pLine->flags & LOG_LINE) != 0); // 6
    MakeTableItemBool (L, "bookmark", (pLine->flags & BOOKMARK) != 0); // 7
    MakeTableItemBool (L, "hr",       (pLine->flags & HORIZ_RULE) != 0); // 8
    MakeTableItem     (L, "time",     (int) pLine->m_theTime.GetTime ()); // 9a
    MakeTableItem     (L, "timestr",  COleDateTime (pLine->m_theTime.GetTime ())); // 9b
    MakeTableItem     (L, "line",     pLine->m_nLineNumber); // 10
    MakeTableItem     (L, "styles",   pLine->styleList.GetCount ()); // 11

    // high-performance timer
    double ticks = (double) pLine->m_lineHighPerformanceTime.QuadPart / (double) App.m_iCounterFrequency;
    MakeTableItem (L, "ticks", ticks);

    return 1;   // one table
    }     // end of returning a table

  // normal behaviour
  VARIANT v = pDoc->GetLineInfo (iLine, iType);
  return pushVariant (L, v);
  } // end of L_GetLineInfo


//----------------------------------------
//  world.GetLinesInBufferCount
//----------------------------------------
static int L_GetLinesInBufferCount (lua_State *L)
  {
  lua_pushnumber (L, doc (L)->GetLinesInBufferCount ());
  return 1;  // number of result fields
  } // end of L_GetLinesInBufferCount


//----------------------------------------
//  world.GetLoadedValue
//----------------------------------------
static int L_GetLoadedValue (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->GetLoadedValue (
            my_checkstring (L, 1)   // OptionName
            );
  return pushVariant (L, v);
  } // end of L_GetLoadedValue

static void luaWindowPositionHelper (lua_State *L, const RECT & r)
  {
  lua_newtable(L);                                                            
  MakeTableItem     (L, "left",   r.left);
  MakeTableItem     (L, "top",    r.top);
  MakeTableItem     (L, "width",  r.right - r.left);
  MakeTableItem     (L, "height", r.bottom - r.top);
  } // end of luaWindowPositionHelper

//----------------------------------------
//  world.GetMainWindowPosition  
// - extension, return table rather than string
//----------------------------------------
static int L_GetMainWindowPosition (lua_State *L)
  {
  CWindowPlacement wp;
  Frame.GetWindowPlacement(&wp);  

  luaWindowPositionHelper (L, wp.rcNormalPosition);
  return 1;  // number of result fields
  } // end of L_GetMainWindowPosition

//----------------------------------------
//  world.GetNotepadWindowPosition  
// - extension, return table rather than string
//----------------------------------------
static int L_GetNotepadWindowPosition (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);

  CTextDocument * pTextDoc = pDoc->FindNotepad (my_checkstring (L, 1));

  if (pTextDoc)
    {
    CWindowPlacement wp;

    // get the view
    POSITION pos=pTextDoc->GetFirstViewPosition();

    if (pos)
      {
      CView* pView = pTextDoc->GetNextView(pos);

      if (pView->IsKindOf(RUNTIME_CLASS(CTextView)))
        {
        CTextView* pmyView = (CTextView*)pView;
        pmyView->GetParentFrame ()->GetWindowPlacement(&wp);
        luaWindowPositionHelper (L, wp.rcNormalPosition);
        return 1;  // number of result fields
        } // end of having the right type of view
      }   // end of having a view
    } // end of having an existing notepad document

  return 0;   // no results, notepad not found
  } // end of GetNotepadWindowPosition

//----------------------------------------
//  world.GetWorldWindowPosition  
// - extension, return table rather than string
//----------------------------------------
static int L_GetWorldWindowPosition (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  int which = (int) my_optnumber (L, 1, 1);  // Which
  bool screen = optboolean (L, 2, 0);  // client or screen
  int count = 0;

  CWindowPlacement wp;

  for(POSITION pos=pDoc->GetFirstViewPosition();pos!=NULL;)
    {
    CView* pView = pDoc->GetNextView(pos);

    if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
      {
      CSendView* pmyView = (CSendView*)pView;

      count++;
      if (count != which)
        continue;      // wrong one

//        pmyView->GetParentFrame ()->GetClientRect (&wp.rcNormalPosition);
      pmyView->GetParentFrame ()->GetWindowPlacement(&wp); 

      if (screen)
        {
        pmyView->GetParentFrame ()->ClientToScreen (&wp.rcNormalPosition);
        }
      luaWindowPositionHelper (L, wp.rcNormalPosition);
      return 1;  // number of result fields

      }	
    }

  return 0; // oops, that one not found (returns nil)

  } // end of L_GetWorldWindowPosition


//----------------------------------------
//  world.MakeRegularExpression
//----------------------------------------
static int L_MakeRegularExpression (lua_State *L)
  {
  doc (L);
  // uses helper routine to avoid using BSTR
  lua_pushstring (L, ConvertToRegularExpression(my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_MakeRegularExpression

//----------------------------------------
//  world.GetMapColour
//----------------------------------------
static int L_GetMapColour (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->GetMapColour (
	              (long) my_checknumber (L, 1)
				  ));
  return 1;  // number of result fields
  } // end of L_GetMapColour

//----------------------------------------
//  world.GetMappingCount
//----------------------------------------
static int L_GetMappingCount (lua_State *L)
  {
  lua_pushnumber (L, doc (L)->GetMappingCount ());
  return 1;  // number of result fields
  } // end of L_GetMappingCount


//----------------------------------------
//  world.GetMappingItem
//----------------------------------------
static int L_GetMappingItem (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->GetMappingItem (
            (long) my_checknumber (L, 1)   // Item
            );
  return pushVariant (L, v);
  } // end of L_GetMappingItem


//----------------------------------------
//  world.GetMappingString
//----------------------------------------
static int L_GetMappingString (lua_State *L)
  {
  VARIANT v = doc (L)->GetMappingString ();
  return pushVariant (L, v);
  } // end of L_GetMappingString


//----------------------------------------
//  world.GetNotepadLength
//----------------------------------------
static int L_GetNotepadLength (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->GetNotepadLength (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_GetNotepadLength

//----------------------------------------
//  world.GetNotepadList
//----------------------------------------
static int L_GetNotepadList (lua_State *L)
  {
  VARIANT v = doc (L)->GetNotepadList (optboolean (L, 1, 0));
  return pushVariant (L, v);
  } // end of L_GetNotepadList

//----------------------------------------
//  world.GetNotepadText
//----------------------------------------
static int L_GetNotepadText (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  BSTR str = pDoc->GetNotepadText (my_checkstring (L, 1));
  return pushBstr (L, str);  // number of result fields
  } // end of L_GetNotepadText


//----------------------------------------
//  world.GetNotes
//----------------------------------------
static int L_GetNotes (lua_State *L)
  {
  BSTR str = doc (L)->GetNotes ();
  return pushBstr (L, str);  // number of result fields
  } // end of L_GetNotes

//----------------------------------------
//  world.GetNoteStyle
//----------------------------------------
static int L_GetNoteStyle (lua_State *L)
  {
  lua_pushnumber (L, doc (L)->GetNoteStyle ());
  return 1;  // number of result fields
  } // end of L_GetNoteStyle


//----------------------------------------
//  world.GetOption
//----------------------------------------
static int L_GetOption (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->GetOption (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_GetOption


//----------------------------------------
//  world.GetOptionList
//----------------------------------------
static int L_GetOptionList (lua_State *L)
  {
  VARIANT v = doc (L)->GetOptionList ();
  return pushVariant (L, v);
  } // end of L_GetOptionList

//----------------------------------------
//  world.PickColour
//----------------------------------------
static int L_PickColour (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->PickColour (
	              (long) my_checknumber (L, 1)
				  ));
  return 1;  // number of result fields
  } // end of L_PickColour


//----------------------------------------
//  world.GetPluginAliasInfo
//----------------------------------------
static int L_GetPluginAliasInfo (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->GetPluginAliasInfo (
      my_checkstring (L, 1),    // PluginID
      my_checkstring (L, 2),    // Name
      (short) my_checknumber (L, 3)     // Type
    );
  return pushVariant (L, v);  // number of result fields
  } // end of L_GetPluginAliasInfo


//----------------------------------------
//  world.GetPluginAliasList
//----------------------------------------
static int L_GetPluginAliasList (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->GetPluginAliasList (
      my_checkstring (L, 1)    // PluginID
    );
  return pushVariant (L, v);  // number of result fields
  } // end of L_GetPluginAliasList


//----------------------------------------
//  world.GetPluginAliasOption
//----------------------------------------
static int L_GetPluginAliasOption (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->GetPluginAliasOption (
      my_checkstring (L, 1),    // PluginID
      my_checkstring (L, 2),    // Alias Name
      my_checkstring (L, 3)     // Option Name
    );
  return pushVariant (L, v);  // number of result fields
  } // end of L_GetPluginAliasOption



//----------------------------------------
//  world.GetPluginID
//----------------------------------------
static int L_GetPluginID (lua_State *L)
  {
  BSTR str = doc (L)->GetPluginID ();
  return pushBstr (L, str);  // number of result fields
  } // end of L_GetPluginID


//----------------------------------------
//  world.GetPluginInfo
//----------------------------------------
static int L_GetPluginInfo (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->GetPluginInfo (
      my_checkstring (L, 1),            // PluginID
      (short) my_checknumber (L, 2)     // Type
    );
  return pushVariant (L, v);  // number of result fields
  } // end of L_GetPluginInfo


//----------------------------------------
//  world.GetPluginList
//----------------------------------------
static int L_GetPluginList (lua_State *L)
  {
  VARIANT v = doc (L)->GetPluginList ();
  return pushVariant (L, v);
  } // end of L_GetPluginList


//----------------------------------------
//  world.GetPluginName
//----------------------------------------
static int L_GetPluginName (lua_State *L)
  {
  BSTR str = doc (L)->GetPluginName ();
  return pushBstr (L, str);  // number of result fields
  } // end of L_GetPluginName


//----------------------------------------
//  world.GetPluginTimerInfo
//----------------------------------------
static int L_GetPluginTimerInfo (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->GetPluginTimerInfo (
      my_checkstring (L, 1),            // PluginID
      my_checkstring (L, 2),            // Name
      (short) my_checknumber (L, 3)     // Type
    );
  return pushVariant (L, v);  // number of result fields
  } // end of L_GetPluginTimerInfo


//----------------------------------------
//  world.GetPluginTimerList
//----------------------------------------
static int L_GetPluginTimerList (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->GetPluginTimerList (
      my_checkstring (L, 1)    // PluginID
    );
  return pushVariant (L, v);  // number of result fields
  } // end of L_GetPluginTimerList


//----------------------------------------
//  world.GetPluginTimerOption
//----------------------------------------
static int L_GetPluginTimerOption (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->GetPluginTimerOption (
      my_checkstring (L, 1),    // PluginID
      my_checkstring (L, 2),    // Timer Name
      my_checkstring (L, 3)     // Option Name
    );
  return pushVariant (L, v);  // number of result fields
  } // end of L_GetPluginTimerOption


//----------------------------------------
//  world.GetPluginTriggerInfo
//----------------------------------------
static int L_GetPluginTriggerInfo (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->GetPluginTriggerInfo (
      my_checkstring (L, 1),            // PluginID
      my_checkstring (L, 2),            // Name
      (short) my_checknumber (L, 3)     // Type
    );
  return pushVariant (L, v);  // number of result fields
  } // end of L_GetPluginTriggerInfo


//----------------------------------------
//  world.GetPluginTriggerList
//----------------------------------------
static int L_GetPluginTriggerList (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->GetPluginTriggerList (
      my_checkstring (L, 1)    // PluginID
    );
  return pushVariant (L, v);  // number of result fields
  } // end of L_GetPluginTriggerList


//----------------------------------------
//  world.GetPluginTriggerOption
//----------------------------------------
static int L_GetPluginTriggerOption (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->GetPluginTriggerOption (
      my_checkstring (L, 1),    // PluginID
      my_checkstring (L, 2),    // Trigger Name
      my_checkstring (L, 3)     // Option Name
    );
  return pushVariant (L, v);  // number of result fields
  } // end of L_GetPluginTriggerOption


//----------------------------------------
//  world.GetPluginVariable
//----------------------------------------
static int L_GetPluginVariable (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->GetPluginVariable (
      my_checkstring (L, 1),   // PluginID
      my_checkstring (L, 2)    // VariableName
    );
  return pushVariant (L, v);  // number of result fields
  } // end of L_GetPluginVariable


//----------------------------------------
//  world.GetPluginVariableList
//----------------------------------------
static int L_GetPluginVariableList (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);

  // plugin name
  const char * PluginID = my_checkstring (L, 1);

  // lookup plugin
  CPlugin * pPlugin = NULL;  
  if (strlen (PluginID) > 0)  
    {                         
    pPlugin = pDoc->GetPlugin (PluginID); 
    if (!pPlugin)             
	    return 0;   // no results - non-empty plugin not found     
    }                         
  // save current plugin
  CPlugin * pOldPlugin = pDoc->m_CurrentPlugin;  
  pDoc->m_CurrentPlugin = pPlugin;               
                      
  // now get the variable list 
  GetVariableListHelper (L, pDoc);                   

  // restore current plugin
  pDoc->m_CurrentPlugin = pOldPlugin;            

  return 1;     // one result (one table)

  } // end of L_GetPluginVariableList


//----------------------------------------
//  world.GetQueue
//----------------------------------------
static int L_GetQueue (lua_State *L)
  {
  VARIANT v = doc (L)->GetQueue ();
  return pushVariant (L, v);
  } // end of L_GetQueue


//----------------------------------------
//  world.GetReceivedBytes
//----------------------------------------
static int L_GetReceivedBytes (lua_State *L)
  {
  lua_pushnumber (L, (lua_Number) (doc (L)->m_nBytesIn) );
  return 1;  // number of result fields
  } // end of L_GetReceivedBytes


//----------------------------------------
//  world.GetRecentLines
//----------------------------------------
static int L_GetRecentLines (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  BSTR str = pDoc->GetRecentLines ((long) my_checknumber (L, 1));
  return pushBstr (L, str);  // number of result fields
  } // end of L_GetRecentLines


//----------------------------------------
//  world.GetScriptTime
//----------------------------------------
static int L_GetScriptTime (lua_State *L)
  {
  lua_pushnumber (L, doc (L)->GetScriptTime ());
  return 1;  // number of result fields
  } // end of L_GetScriptTime


//----------------------------------------
//  world.GetSelectionEndColumn
//----------------------------------------
static int L_GetSelectionEndColumn (lua_State *L)
  {
  lua_pushnumber (L, doc (L)->GetSelectionEndColumn ());
  return 1;  // number of result fields
  } // end of L_GetSelectionEndColumn


//----------------------------------------
//  world.GetSelectionEndLine
//----------------------------------------
static int L_GetSelectionEndLine (lua_State *L)
  {
  lua_pushnumber (L, doc (L)->GetSelectionEndLine ());
  return 1;  // number of result fields
  } // end of L_GetSelectionEndLine


//----------------------------------------
//  world.GetSelectionStartColumn
//----------------------------------------
static int L_GetSelectionStartColumn (lua_State *L)
  {
  lua_pushnumber (L, doc (L)->GetSelectionStartColumn ());
  return 1;  // number of result fields
  } // end of L_GetSelectionStartColumn


//----------------------------------------
//  world.GetSelectionStartLine
//----------------------------------------
static int L_GetSelectionStartLine (lua_State *L)
  {
  lua_pushnumber (L, doc (L)->GetSelectionStartLine ());
  return 1;  // number of result fields
  } // end of L_GetSelectionStartLine


//----------------------------------------
//  world.GetSentBytes
//----------------------------------------
static int L_GetSentBytes (lua_State *L)
  {
  lua_pushnumber (L, (lua_Number) (doc (L)->m_nBytesOut) );
  return 1;  // number of result fields
  } // end of L_GetSentBytes


//----------------------------------------
//  world.GetSoundStatus
//----------------------------------------
static int L_GetSoundStatus (lua_State *L)
  {
  lua_pushnumber (L, doc (L)->GetSoundStatus (
	              (short) my_checknumber (L, 1)
				  ));
  return 1;  // number of result fields
  } // end of L_GetSoundStatus


static bool DoStyle (lua_State *L, 
                      CMUSHclientDoc *pDoc, 
                      CLine * pLine, 
                      int iStyleNumber,
                      CString & strText)
  {

  // check style exists
  if (iStyleNumber <= 0 || iStyleNumber > pLine->styleList.GetCount ())
    return true;   // error, style doesn't exist

  CStyle * pStyle = NULL;
  POSITION pos;
  int iCol = 0;
  int iCount = 1;

  // search for it sequentially so we know the column number, 
  // so we can get its text
  for (pos = pLine->styleList.GetHeadPosition(); pos; iCount++)
    {
    pStyle = pLine->styleList.GetNext (pos);
    if (iCount == iStyleNumber)
      break;  // found right one
    
    if (!pos)
      return true;   // error, style doesn't exist

    iCol += pStyle->iLength; // new column

    } // end of looping looking for it

    int iAction = 0;
    if (pStyle)
      switch (pStyle->iFlags & ACTIONTYPE)
        {
        case ACTION_NONE:       iAction = 0; break;
        case ACTION_SEND:       iAction = 1; break;
          case ACTION_HYPERLINK:  iAction = 2; break;
        case ACTION_PROMPT:     iAction = 3; break;
        } // end of switch


    COLORREF colour1,
             colour2;

    pDoc->GetStyleRGB (pStyle, colour1, colour2);
    CAction * pAction = pStyle->pAction;

//  1: text of style
//  2: length of style run
//  3: starting column of style
//  4: action type - 0=none, 1=send to mud, 2=hyperlink, 3=prompt
//  5: action   (eg. what to send)
//  6: hint     (what to show)
//  7: variable (variable to set)
//  8: true if bold
//  9: true if underlined
// 10: true if blinking
// 11: true if inverse
// 12: true if changed by trigger from original
// 13: true if start of a tag (action is tag name)
// 14: foreground (text) colour in RGB
// 15: background colour in RGB

  lua_newtable(L);                                                            
  MakeTableItem     (L, "text",     strText.Mid (iCol, pStyle->iLength)); // 1
  MakeTableItem     (L, "length",   pStyle->iLength); // 2
  MakeTableItem     (L, "column",   iCol + 1); // 3
  MakeTableItem     (L, "actiontype", iAction); // 4
  MakeTableItem     (L, "action",   pAction ? pAction->m_strAction : ""); // 5
  MakeTableItem     (L, "hint",     pAction ? pAction->m_strHint : ""); // 6
  MakeTableItem     (L, "variable", pAction ? pAction->m_strVariable : ""); // 7
  MakeTableItemBool (L, "bold",     (pStyle->iFlags & HILITE) != 0); // 8
  MakeTableItemBool (L, "ul",       (pStyle->iFlags & UNDERLINE) != 0); // 9
  MakeTableItemBool (L, "blink",    (pStyle->iFlags & BLINK) != 0); // 10
  MakeTableItemBool (L, "inverse",  (pStyle->iFlags & INVERSE) != 0); // 11
  MakeTableItemBool (L, "changed",  (pStyle->iFlags & CHANGED) != 0); // 12
  MakeTableItemBool (L, "starttag", (pStyle->iFlags & START_TAG) != 0); // 13
  MakeTableItem     (L, "textcolour", colour1); // 14
  MakeTableItem     (L, "backcolour", colour2); // 15

  return false;   // OK return
  } // end of DoStyle

//----------------------------------------
//  world.GetStyleInfo
// - extension will return a table of types, or table of tables
//----------------------------------------
static int L_GetStyleInfo (lua_State *L)
  {

  CMUSHclientDoc * pDoc = doc (L);  // must do this first
  int iLine = (int) my_checknumber (L, 1);            // LineNumber  
  int iStyleNumber = (int) my_optnumber (L, 2, 0);    // StyleNumber 
  int iType = (int) my_optnumber (L, 3, 0);           // InfoType    

  CLine * pLine = NULL;
  CString strText;

  if (iStyleNumber == 0 || iType == 0)  // extension
    {
    // check line exists
    if (iLine <= 0 || iLine > pDoc->m_LineList.GetCount ())
      return 0;     // no result for no line

    // get pointer to line in question

    pLine = pDoc->m_LineList.GetAt (pDoc->GetLinePosition (iLine - 1));
    strText = CString (pLine->text, pLine->len);

    }

  // if style is zero, make a table per style
  if (iStyleNumber == 0)  // do all styles
    {
    lua_newtable(L);    // table has one entry per style                                                         
    for (iStyleNumber = 1; iStyleNumber <= pLine->styleList.GetCount (); iStyleNumber++)
      {
      if (iType == 0)   // all types wanted
        DoStyle (L, pDoc, pLine, iStyleNumber, strText);
      else
        {   // a single type, use our usual routine to get it
        VARIANT v = pDoc->GetStyleInfo (iLine, iStyleNumber, iType); 
        pushVariant (L, v);
        }

      lua_rawseti(L, -2, iStyleNumber);  // put individual style table into line table
      }  // for each style
    return 1;   // one table
    }

  // only one style, however they want all types for it
  if (iType == 0)
    {
    if (DoStyle (L, pDoc, pLine, iStyleNumber, strText))
      return 0;   // error, no table returned
    return 1;   // one table
    }

  // here for usual behaviour
  VARIANT v = pDoc->GetStyleInfo (iLine, iStyleNumber, iType); 
  return pushVariant (L, v);
  } // end of L_GetStyleInfo

//----------------------------------------
//  world.GetSysColor
//----------------------------------------
static int L_GetSysColor (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->GetSysColor (
	              (long) my_checknumber (L, 1)
				  ));
  return 1;  // number of result fields
  } // end of L_GetSysColor

//----------------------------------------
//  world.GetSystemMetrics
//----------------------------------------
static int L_GetSystemMetrics (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->GetSystemMetrics (
	              (long) my_checknumber (L, 1)
				  ));
  return 1;  // number of result fields
  } // end of L_GetSystemMetrics

//----------------------------------------
//  world.GetTimer
//----------------------------------------
static int L_GetTimer (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);

  VARIANT Hour, 
          Minute, 
          Second, 
          ResponseText, 
          Flags, 
          ScriptName;

  long result = pDoc->GetTimer(my_checkstring (L, 1),  // name
                                  &Hour, 
                                  &Minute, 
                                  &Second, 
                                  &ResponseText, 
                                  &Flags, 
                                  &ScriptName); 

  lua_pushnumber (L, result);       // 1
  pushVariant (L, Hour);            // 2
  pushVariant (L, Minute);          // 3
  pushVariant (L, Second);          // 4
  pushVariant (L, ResponseText);    // 5
  pushVariant (L, Flags);           // 6
  pushVariant (L, ScriptName);      // 7

  return 7;  // number of result fields
  } // end of L_GetTimer


//----------------------------------------
//  world.GetTimerInfo
//----------------------------------------
static int L_GetTimerInfo (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->GetTimerInfo (
      my_checkstring (L, 1),            // TimerName
      (short) my_checknumber (L, 2)     // Type
    );
  return pushVariant (L, v);  // number of result fields
  } // end of L_GetTimerInfo


//----------------------------------------
//  world.GetTimerList
//----------------------------------------
static int L_GetTimerList (lua_State *L)
  {
  VARIANT v = doc (L)->GetTimerList ();
  return pushVariant (L, v);  // number of result fields
  } // end of L_GetTimerList


//----------------------------------------
//  world.GetTimerOption
//----------------------------------------
static int L_GetTimerOption (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->GetTimerOption (
      my_checkstring (L, 1),    // TimerName
      my_checkstring (L, 2)     // Option name
    );
  return pushVariant (L, v);  // number of result fields
  } // end of L_GetTimerOption


//----------------------------------------
//  world.GetTrigger
//----------------------------------------
static int L_GetTrigger (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);

  VARIANT MatchText, 
          ResponseText, 
          Flags, 
          Colour, 
          Wildcard, 
          SoundFileName, 
          ScriptName;

  long result = pDoc->GetTrigger(my_checkstring (L, 1),  // name
                                  &MatchText, 
                                  &ResponseText, 
                                  &Flags, 
                                  &Colour, 
                                  &Wildcard, 
                                  &SoundFileName, 
                                  &ScriptName); 

  lua_pushnumber (L, result);      // 1
  pushVariant (L, MatchText);      // 2
  pushVariant (L, ResponseText);   // 3
  pushVariant (L, Flags);          // 4
  pushVariant (L, Colour);         // 5
  pushVariant (L, Wildcard);       // 6
  pushVariant (L, SoundFileName);  // 7
  pushVariant (L, ScriptName);     // 8

  return 8;  // number of result fields
  } // end of L_GetTrigger


//----------------------------------------
//  world.GetTriggerInfo
//----------------------------------------
static int L_GetTriggerInfo (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->GetTriggerInfo (
      my_checkstring (L, 1),            // TriggerName
      (short) my_checknumber (L, 2)     // Type
    );
  return pushVariant (L, v);  // number of result fields
  } // end of L_GetTriggerInfo


//----------------------------------------
//  world.GetTriggerList
//----------------------------------------
static int L_GetTriggerList (lua_State *L)
  {
  VARIANT v = doc (L)->GetTriggerList ();
  return pushVariant (L, v);  // number of result fields
  } // end of L_GetTriggerList


//----------------------------------------
//  world.GetTriggerOption
//----------------------------------------
static int L_GetTriggerOption (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->GetTriggerOption (
      my_checkstring (L, 1),    // TriggerName
      my_checkstring (L, 2)     // Option name
    );
  return pushVariant (L, v);  // number of result fields
  } // end of L_GetTriggerOption


//----------------------------------------
//  world.GetTriggerWildcard
//----------------------------------------
static int L_GetTriggerWildcard (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->GetTriggerWildcard (
      my_checkstring (L, 1),    // Name
      my_checkstring (L, 2)     // Wildcard name
    );
  return pushVariant (L, v);  // number of result fields
  } // end of L_GetTriggerWildcard


//----------------------------------------
//  world.GetUdpPort
//----------------------------------------
static int L_GetUdpPort (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->GetUdpPort (
      (long) my_checknumber (L, 1),    // First
      (long) my_checknumber (L, 2)     // Last
    ));
  return 1;  // number of result fields
  } // end of L_GetUdpPort


//----------------------------------------
//  world.GetUniqueID
//----------------------------------------
static int L_GetUniqueID (lua_State *L)
  {
  BSTR str = doc (L)->GetUniqueID ();
  return pushBstr (L, str);  // number of result fields
  } // end of L_GetUniqueID


//----------------------------------------
//  world.GetUniqueNumber
//----------------------------------------
static int L_GetUniqueNumber (lua_State *L)
  {
  lua_pushnumber (L, doc (L)->GetUniqueNumber ());
  return 1;  // number of result fields
  } // end of L_GetUniqueNumber


//----------------------------------------
//  world.GetVariable
//----------------------------------------
static int L_GetVariable (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->GetVariable (
      my_checkstring (L, 1)    // VariableName
    );
  return pushVariant (L, v);  // number of result fields 
  } // end of L_GetVariable


//----------------------------------------
//  world.GetVariableList
// extension - returns variable name *and* value
//----------------------------------------
static int L_GetVariableList (lua_State *L)
  {
  GetVariableListHelper (L, doc (L));
  return 1;  // number of result fields  (one table)
  } // end of L_GetVariableList


//----------------------------------------
//  world.GetWorld
//----------------------------------------
static int L_GetWorld (lua_State *L)
  {
  CString strName = my_checkstring (L, 1);

  for (POSITION docPos = App.m_pWorldDocTemplate->GetFirstDocPosition();
      docPos != NULL; )
    {
    CMUSHclientDoc * pDoc = (CMUSHclientDoc *) App.m_pWorldDocTemplate->GetNextDoc(docPos);
  
    if (pDoc->m_mush_name.CompareNoCase (strName) == 0)
      {
      // our "world" is a userdatum which is a pointer (to a pointer) to the world
      CMUSHclientDoc **ud = (CMUSHclientDoc **)lua_newuserdata(L, sizeof (CMUSHclientDoc *));
      luaL_getmetatable(L, mushclient_typename);
      lua_setmetatable(L, -2);
      *ud = pDoc;    // store pointer to this world in the userdata
      return 1;

      } // end of world found
    } // end of doing each document

  return 0;  // number of result fields
  } // end of L_GetWorld


//----------------------------------------
//  world.GetWorldById
//----------------------------------------
static int L_GetWorldById (lua_State *L)
  {
  CString WorldID = my_checkstring (L, 1);

  for (POSITION docPos = App.m_pWorldDocTemplate->GetFirstDocPosition();
      docPos != NULL; )
    {
    CMUSHclientDoc * pDoc = (CMUSHclientDoc *) App.m_pWorldDocTemplate->GetNextDoc(docPos);
  
    if (pDoc->m_strWorldID.CompareNoCase (WorldID) == 0)
      {
      // our "world" is a userdatum which is a pointer (to a pointer) to the world
      CMUSHclientDoc **ud = (CMUSHclientDoc **)lua_newuserdata(L, sizeof (CMUSHclientDoc *));
      luaL_getmetatable(L, mushclient_typename);
      lua_setmetatable(L, -2);
      *ud = pDoc;    // store pointer to this world in the userdata
      return 1;

      } // end of world found
    } // end of doing each document

  return 0;  // number of result fields
  } // end of L_GetWorldById


//----------------------------------------
//  world.GetWorldID
//----------------------------------------
static int L_GetWorldID (lua_State *L)
  {
  BSTR str = doc (L)->GetWorldID ();
  return pushBstr (L, str);  // number of result fields
  } // end of L_GetWorldID

//----------------------------------------
//  world.GetWorldIdList
//----------------------------------------
static int L_GetWorldIdList (lua_State *L)
  {
  VARIANT v = doc (L)->GetWorldIdList ();
  return pushVariant (L, v);  // number of result fields
  } // end of L_GetWorldIdList


//----------------------------------------
//  world.GetWorldList
//----------------------------------------
static int L_GetWorldList (lua_State *L)
  {
  VARIANT v = doc (L)->GetWorldList ();
  return pushVariant (L, v);  // number of result fields
  } // end of L_GetWorldList


//----------------------------------------
//  world.Hash
//----------------------------------------
static int L_Hash (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  BSTR str = pDoc->Hash (my_checkstring (L, 1));
  return pushBstr (L, str);  // number of result fields
  } // end of L_Hash

//----------------------------------------
//  world.Help
//----------------------------------------
static int L_Help (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->Help (my_optstring (L, 1, ""));
  return 0;  // number of result fields
  } // end of L_Help

//----------------------------------------
//  world.Hyperlink
//----------------------------------------
static int L_Hyperlink (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->Hyperlink (
                      my_checkstring (L, 1),  // Action
                      my_checkstring (L, 2),  // Text
                      my_checkstring (L, 3),  // Hint
                      my_checkstring (L, 4),  // TextColour
                      my_checkstring (L, 5),  // BackColour
                      optboolean (L, 6, 0)   // URL  - optional
                      );
  return 0;  // number of result fields
  } // end of L_Hyperlink


//----------------------------------------
//  world.ImportXML
//----------------------------------------
static int L_ImportXML (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->ImportXML (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_ImportXML


//----------------------------------------
//  world.Info
//----------------------------------------
static int L_Info (lua_State *L)
  {
  CMUSHclientDoc * pDoc = doc (L);  // must do this first
  pDoc->Info (concatArgs (L).c_str ());
//  lua_concat (L, lua_gettop (L));
//  pDoc->Info (my_checkstring (L, 1));
  return 0;  // number of result fields
  } // end of L_Info


//----------------------------------------
//  world.InfoBackground
//----------------------------------------
static int L_InfoBackground (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->InfoBackground (my_checkstring (L, 1));
  return 0;  // number of result fields
  } // end of L_InfoBackground


//----------------------------------------
//  world.InfoClear
//----------------------------------------
static int L_InfoClear (lua_State *L)
  {
  doc (L)->InfoClear ();
  return 0;  // number of result fields
  } // end of L_InfoClear


//----------------------------------------
//  world.InfoColour
//----------------------------------------
static int L_InfoColour (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->InfoColour (my_checkstring (L, 1));
  return 0;  // number of result fields
  } // end of L_InfoColour


//----------------------------------------
//  world.InfoFont
//----------------------------------------
static int L_InfoFont (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->InfoFont (
                    my_checkstring (L, 1),    // FontName
                    (short) my_checknumber (L, 2),    // Size
                    (short) my_checknumber (L, 3)     // Style
                    );
  return 0;  // number of result fields
  } // end of L_InfoFont


//----------------------------------------
//  world.IsAlias
//----------------------------------------
static int L_IsAlias (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->IsAlias (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_IsAlias


//----------------------------------------
//  world.IsConnected
//----------------------------------------
static int L_IsConnected (lua_State *L)
  {
  lua_pushboolean (L, doc (L)->IsConnected ());
  return 1;  // number of result fields
  } // end of L_IsConnected


//----------------------------------------
//  world.IsLogOpen
//----------------------------------------
static int L_IsLogOpen (lua_State *L)
  {
  lua_pushboolean (L, doc (L)->IsLogOpen ());
  return 1;  // number of result fields
  } // end of L_IsLogOpen


//----------------------------------------
//  world.IsPluginInstalled
//----------------------------------------
static int L_IsPluginInstalled (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushboolean (L, pDoc->IsPluginInstalled (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_IsPluginInstalled


//----------------------------------------
//  world.IsTimer
//----------------------------------------
static int L_IsTimer (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->IsTimer (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_IsTimer


//----------------------------------------
//  world.IsTrigger
//----------------------------------------
static int L_IsTrigger (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->IsTrigger (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_IsTrigger


//----------------------------------------
//  world.LoadPlugin
//----------------------------------------
static int L_LoadPlugin (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->LoadPlugin (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_LoadPlugin


//----------------------------------------
//  world.GetLogInput
//----------------------------------------
static int L_GetLogInput (lua_State *L)
  {
  lua_pushboolean (L, doc (L)->m_log_input);
  return 1;  // number of result fields
  } // end of L_GetLogInput

//----------------------------------------
//  world.SetLogInput
//----------------------------------------
static int L_SetLogInput (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->m_log_input = optboolean (L, 1, 1) != 0;
  return 0;  // number of result fields
  } // end of L_SetLogInput


//----------------------------------------
//  world.GetLogNotes
//----------------------------------------
static int L_GetLogNotes (lua_State *L)
  {
  lua_pushboolean (L, doc (L)->m_bLogNotes);
  return 1;  // number of result fields
  } // end of L_GetLogNotes

//----------------------------------------
//  world.SetLogNotes
//----------------------------------------
static int L_SetLogNotes (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->m_bLogNotes = optboolean (L, 1, 1) != 0;
  return 0;  // number of result fields
  } // end of L_SetLogNotes


//----------------------------------------
//  world.GetLogOutput
//----------------------------------------
static int L_GetLogOutput (lua_State *L)
  {
  lua_pushboolean (L, doc (L)->m_bLogOutput);
  return 1;  // number of result fields
  } // end of L_GetLogOutput

//----------------------------------------
//  world.SetLogOutput
//----------------------------------------
static int L_SetLogOutput (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->m_bLogOutput = optboolean (L, 1, 1) != 0;
  return 0;  // number of result fields
  } // end of L_SetLogOutput

//----------------------------------------
//  world.LogSend
//----------------------------------------
static int L_LogSend (lua_State *L)
  {
  CMUSHclientDoc * pDoc = doc (L);  // must do this first
  lua_pushnumber (L, pDoc->LogSend (concatArgs (L).c_str ()));

//  lua_concat (L, lua_gettop (L));
//  lua_pushnumber (L, pDoc->LogSend (my_checkstring (L, 1)));

  return 1;  // number of result fields
  } // end of L_LogSend

//----------------------------------------
//  world.MapColour
//----------------------------------------
static int L_MapColour (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->MapColour (
      (long) my_checknumber (L, 1),  // Original
      (long) my_checknumber (L, 2)   // Replacement
      );
  return 0;  // number of result fields
  } // end of L_MapColour

//----------------------------------------
//  world.MapColourList
//----------------------------------------
static int L_MapColourList (lua_State *L)
  {
  CMUSHclientDoc * pDoc = doc (L);  // must do this first
  lua_newtable(L);                                                            

  map<COLORREF, COLORREF>::const_iterator it;

  for (it = pDoc->m_ColourTranslationMap.begin (); 
       it != pDoc->m_ColourTranslationMap.end (); it++)
    {
    lua_pushnumber (L, it->first);
    lua_pushnumber (L, it->second);
    lua_rawset(L, -3);
    }      // end of looping through each colour

  return 1;  // 1 table
  } // end of L_MapColourList

//----------------------------------------
//  world.GetMapping
//----------------------------------------
static int L_GetMapping (lua_State *L)
  {
  lua_pushboolean (L, doc (L)->m_bMapping);
  return 1;  // number of result fields
  } // end of L_GetMapping

//----------------------------------------
//  world.SetMapping
//----------------------------------------
static int L_SetMapping (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->m_bMapping = optboolean (L, 1, 1) != 0;
  return 0;  // number of result fields
  } // end of L_SetMapping

//----------------------------------------
//  world.Metaphone
//----------------------------------------
static int L_Metaphone (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  BSTR str = pDoc->Metaphone (my_checkstring (L, 1),
                              (short) my_optnumber (L, 2, 4));
  return pushBstr (L, str);
  } // end of L_Metaphone

//----------------------------------------
//  world.GetNormalColour
//----------------------------------------
static int L_GetNormalColour (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->GetNormalColour (
	              (short) my_checknumber (L, 1)
				  ));
  return 1;  // number of result fields
  } // end of GetNormalColour

//----------------------------------------
//  world.SetNormalColour
//----------------------------------------
static int L_SetNormalColour (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->SetNormalColour (
      (short) my_checknumber (L, 1),  // WhichColour
      (long) my_checknumber (L, 2)    // nNewValue
      );
  return 0;  // number of result fields
  } // end of SetNormalColour

//----------------------------------------
//  world.MoveMainWindow
//----------------------------------------
static int L_MoveMainWindow (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->MoveMainWindow (
      (long) my_checknumber (L, 1),  // Left
      (long) my_checknumber (L, 2),  // Top
      (long) my_checknumber (L, 3),  // Width
      (long) my_checknumber (L, 4)   // Height
      );
  return 0;  // number of result fields
  } // end of L_MoveMainWindow

//----------------------------------------
//  world.MoveWorldWindow
//----------------------------------------
static int L_MoveWorldWindow (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->MoveWorldWindowX (
      (long) my_checknumber (L, 1),  // Left
      (long) my_checknumber (L, 2),  // Top
      (long) my_checknumber (L, 3),  // Width
      (long) my_checknumber (L, 4),   // Height
      (short) my_optnumber (L, 5, 1)  // Which
      );
  return 0;  // number of result fields
  } // end of L_MoveWorldWindow

//----------------------------------------
//  world.MoveNotepadWindow
//----------------------------------------
static int L_MoveNotepadWindow (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushboolean (L, pDoc->MoveNotepadWindow (
      my_checkstring (L, 1),  // title
      (long) my_checknumber (L, 2),  // Left
      (long) my_checknumber (L, 3),  // Top
      (long) my_checknumber (L, 4),  // Width
      (long) my_checknumber (L, 5)   // Height
      ));
  return 1;  // number of result fields
  } // end of L_MoveNotepadWindow

//----------------------------------------
//  world.MtSrand
//----------------------------------------
static int L_MtSrand (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);

  const int table = 1;

// Lua extension - allow for table of seeds

  if (lua_istable (L, table))
    {
    vector <unsigned long> v;

    // standard Lua table iteration
    for (lua_pushnil (L); lua_next (L, table) != 0; lua_pop (L, 1))
      {
      if (!lua_isnumber (L, -1))
        luaL_error (L, "MtSrand table must consist of numbers");

      v.push_back ((unsigned long) lua_tonumber (L, -1));
      } // end of extracting vector of keys

    if (v.size () == 0)
      luaL_error (L, "MtSrand table must not be empty");

    init_by_array (&v [0], v.size ());
    }
  else
    pDoc->MtSrand ((long) my_checknumber (L, 1));
  return 0;  // number of result fields
  } // end of L_MtSrand

//----------------------------------------
//  world.MtRand
//----------------------------------------
static int L_MtRand (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->MtRand ());
  return 1;  // number of result fields
  } // end of L_MtRand

//----------------------------------------
//  world.Note
//----------------------------------------
static int L_Note (lua_State *L)
  {
  CMUSHclientDoc * pDoc = doc (L);  // must do this first
  pDoc->Note (concatArgs (L).c_str ());

//  lua_concat (L, lua_gettop (L));
//  pDoc->Note (my_checkstring (L, 1));

  return 0;  // number of result fields
  } // end of L_Note

//----------------------------------------
//  world.NoteHr
//----------------------------------------
static int L_NoteHr (lua_State *L)
  {
  CMUSHclientDoc * pDoc = doc (L);  // must do this first
  pDoc->NoteHr ();
  return 0;  // number of result fields
  } // end of L_NoteHr


//----------------------------------------
//  world.GetNoteColour
//----------------------------------------
static int L_GetNoteColour (lua_State *L)
  {
  lua_pushnumber (L, doc (L)->GetNoteColour ());
  return 1;  // number of result fields
  } // end of GetNoteColour

//----------------------------------------
//  world.SetNoteColour
//----------------------------------------
static int L_SetNoteColour (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->SetNoteColour (
      (short) my_checknumber (L, 1)   // nNewValue
      );
  return 0;  // number of result fields
  } // end of SetNoteColour


//----------------------------------------
//  world.GetNoteColourBack
//----------------------------------------
static int L_GetNoteColourBack (lua_State *L)
  {
  lua_pushnumber (L, doc (L)->GetNoteColourBack ());
  return 1;  // number of result fields
  } // end of GetNoteColourBack

//----------------------------------------
//  world.SetNoteColourBack
//----------------------------------------
static int L_SetNoteColourBack (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->SetNoteColourBack (
      (long) my_checknumber (L, 1)   // nNewValue
      );
  return 0;  // number of result fields
  } // end of SetNoteColourBack


//----------------------------------------
//  world.GetNoteColourFore
//----------------------------------------
static int L_GetNoteColourFore (lua_State *L)
  {
  lua_pushnumber (L, doc (L)->GetNoteColourFore ());
  return 1;  // number of result fields
  } // end of GetNoteColourFore

//----------------------------------------
//  world.SetNoteColourFore
//----------------------------------------
static int L_SetNoteColourFore (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->SetNoteColourFore (
      (long) my_checknumber (L, 1)   // nNewValue
      );
  return 0;  // number of result fields
  } // end of SetNoteColourFore


//----------------------------------------
//  world.NoteColourName
//----------------------------------------
static int L_NoteColourName (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->NoteColourName (
      my_checkstring (L, 1),    // Foreground
      my_checkstring (L, 2)     // Background
      );
  return 0;  // number of result fields
  } // end of L_NoteColourName


//----------------------------------------
//  world.NoteColourRGB
//----------------------------------------
static int L_NoteColourRGB (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->NoteColourRGB (
      (long) my_checknumber (L, 1),    // Foreground
      (long) my_checknumber (L, 2)     // Background
      );
  return 0;  // number of result fields
  } // end of L_NoteColourRGB

//----------------------------------------
//  world.NotepadColour
//----------------------------------------
static int L_NotepadColour (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->NotepadColour (
        my_checkstring (L, 1),   // Title
        my_checkstring (L, 2),   // TextColour
        my_checkstring (L, 3)    // BackgroundColour
                      ));
  return 1;  // number of result fields
  } // end of L_NotepadColour

//----------------------------------------
//  world.NotepadFont
//----------------------------------------
static int L_NotepadFont (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->NotepadFont (
        my_checkstring (L, 1),           // Title
        my_checkstring (L, 2),           // FontName
        (short) my_checknumber (L, 3),   // Size
        (short) my_checknumber (L, 4),   // Style
        (short) my_optnumber (L, 5, 0)   // Charset
                      ));
  return 1;  // number of result fields
  } // end of L_NotepadFont


//----------------------------------------
//  world.NotepadReadOnly
//----------------------------------------
static int L_NotepadReadOnly (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->NotepadReadOnly (
        my_checkstring (L, 1),   // Title
        optboolean (L, 2, 1)    // true or false
                      ));
  return 1;  // number of result fields
  } // end of L_NotepadReadOnly


//----------------------------------------
//  world.NotepadSaveMethod
//----------------------------------------
static int L_NotepadSaveMethod (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->NotepadSaveMethod (
        my_checkstring (L, 1),           // Title
        (short) my_checknumber (L, 2)    // Method (0-2)
                      ));
  return 1;  // number of result fields
  } // end of L_NotepadSaveMethod

//----------------------------------------
//  world.NoteStyle
//----------------------------------------
static int L_NoteStyle (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->NoteStyle (
      (short) my_checknumber (L, 1)    // Style
      );
  return 0;  // number of result fields
  } // end of L_NoteStyle


//----------------------------------------
//  world.Open
//----------------------------------------
static int L_Open (lua_State *L)
  {
  doc (L);

  CDocument * pnewDoc = App.OpenDocumentFile (my_checkstring (L, 1));

  lua_pushboolean (L, pnewDoc != NULL);    // true means opened OK, false otherwise

  return 1;  // number of result fields
  } // end of L_Open

//----------------------------------------
//  world.OpenBrowser
//----------------------------------------
static int L_OpenBrowser (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);

  lua_pushnumber (L, pDoc->OpenBrowser (
      my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_OpenBrowser

//----------------------------------------
//  world.OpenLog
//----------------------------------------
static int L_OpenLog (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->OpenLog (
      my_checkstring (L, 1),      // LogFileName
      optboolean (L, 2, 0)      // Append - optional
      ));
  return 1;  // number of result fields
  } // end of L_OpenLog


//----------------------------------------
//  world.PasteCommand
//----------------------------------------
static int L_PasteCommand (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  BSTR str = pDoc->PasteCommand (my_checkstring (L, 1));
  return pushBstr (L, str);
  } // end of L_PasteCommand

//----------------------------------------
//  world.Pause
//----------------------------------------
static int L_Pause (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->Pause (optboolean (L, 1, 1));
  return 0;  // number of result fields
  } // end of L_Pause

//----------------------------------------
//  world.PlaySound
//----------------------------------------
static int L_PlaySound (lua_State *L)
  {
  CMUSHclientDoc * pDoc = doc (L);  // must do this first
  lua_pushnumber (L, pDoc->PlaySound (
                  (short) my_checknumber (L, 1),  // Buffer
                  my_optstring (L, 2, ""),   // Filename
                  optboolean (L, 3, 0),    // Repeat
                  my_optnumber (L, 4, 0),  // Volume
                  my_optnumber (L, 5, 0))  // Pan
                  );
  return 1;  // number of result fields
  } // end of L_PlaySound

//----------------------------------------
//  world.PlaySoundMemory
//----------------------------------------
static int L_PlaySoundMemory (lua_State *L)
  {
  CMUSHclientDoc * pDoc = doc (L);  // must do this first
  LPCTSTR sBuffer;
  size_t  iBufferLength;
  sBuffer = luaL_checklstring (L, 2, &iBufferLength);
  lua_pushnumber (L, pDoc->PlaySoundHelper (
                  (short) my_checknumber (L, 1),  // Buffer
                  NULL,                   // Filename
                  optboolean (L, 3, 0),   // Repeat
                  my_optnumber (L, 4, 0), // Volume
                  my_optnumber (L, 5, 0), // Pan
                  sBuffer,                // MemoryBuffer
                  iBufferLength           // MemLen
                  )  
                  );
  return 1;  // number of result fields
  } // end of L_PlaySoundMemory


//----------------------------------------
//  world.PluginSupports
//----------------------------------------
static int L_PluginSupports (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->PluginSupports (
      my_checkstring (L, 1),      // PluginID
      my_checkstring (L, 2)       // Routine
      ));
  return 1;  // number of result fields
  } // end of L_PluginSupports


//----------------------------------------
//  world.PushCommand
//----------------------------------------
static int L_PushCommand (lua_State *L)
  {
  BSTR str = doc (L)->PushCommand ();
  return pushBstr (L, str);
  } // end of L_PushCommand


//----------------------------------------
//  world.Queue
//----------------------------------------
static int L_Queue (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->Queue (
      my_checkstring (L, 1),      // Message
      optboolean (L, 2, 1)      // Echo - optional
      ));
  return 1;  // number of result fields
  } // end of L_Queue


//----------------------------------------
//  world.ReadNamesFile
//----------------------------------------
static int L_ReadNamesFile (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->ReadNamesFile (
      my_checkstring (L, 1)      // FileName
      ));
  return 1;  // number of result fields
  } // end of L_ReadNamesFile


//----------------------------------------
//  world.Redraw
//----------------------------------------
static int L_Redraw (lua_State *L)
  {
  doc (L)->Redraw ();
  return 0;  // number of result fields
  } // end of L_Redraw

//----------------------------------------
//  world.ReloadPlugin
//----------------------------------------
static int L_ReloadPlugin (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->ReloadPlugin (
      my_checkstring (L, 1)      // PluginID
      ));
  return 1;  // number of result fields
  } // end of L_ReloadPlugin


//----------------------------------------
//  world.RemoveBacktracks
//----------------------------------------
static int L_RemoveBacktracks (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  BSTR str = pDoc->RemoveBacktracks (my_checkstring (L, 1));
  return pushBstr (L, str);
  } // end of L_RemoveBacktracks


//----------------------------------------
//  world.GetRemoveMapReverses
//----------------------------------------
static int L_GetRemoveMapReverses (lua_State *L)
  {
  lua_pushboolean (L, doc (L)->m_bRemoveMapReverses);
  return 1;  // number of result fields
  } // end of L_GetRemoveMapReverses

//----------------------------------------
//  world.SetRemoveMapReverses
//----------------------------------------
static int L_SetRemoveMapReverses (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->m_bRemoveMapReverses = my_checknumber (L, 1) != 0;
  return 0;  // number of result fields
  } // end of L_SetRemoveMapReverses

//----------------------------------------
//  world.Repaint
//----------------------------------------
static int L_Repaint (lua_State *L)
  {
  doc (L)->Repaint ();
  return 0;  // number of result fields
  } // end of L_Repaint

//----------------------------------------
//  world.Replace
//----------------------------------------
static int L_Replace (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  BSTR str = pDoc->Replace (
      my_checkstring (L, 1),    // Source
      my_checkstring (L, 2),    // SearchFor
      my_checkstring (L, 3),    // ReplaceWith
      optboolean (L, 4, 1)    // Multiple - optional
      );
  return pushBstr (L, str);
  } // end of L_Replace


//----------------------------------------
//  world.ReplaceNotepad
//----------------------------------------
static int L_ReplaceNotepad (lua_State *L)
  {
  CMUSHclientDoc * pDoc = doc (L);  // must do this first
  lua_pushboolean (L, pDoc->ReplaceNotepad (
      my_checkstring (L, 1),  // Title
      concatArgs (L, "", 2).c_str ()   // Contents
      ));
  return 0;  // number of result fields
  } // end of L_ReplaceNotepad


//----------------------------------------
//  world.Reset
//----------------------------------------
static int L_Reset (lua_State *L)
  {
  doc (L)->Reset ();
  return 0;  // number of result fields
  } // end of L_Reset


//----------------------------------------
//  world.ResetIP
//----------------------------------------
static int L_ResetIP (lua_State *L)
  {
  doc (L)->ResetIP ();
  return 0;  // number of result fields
  } // end of ResetIP


//----------------------------------------
//  world.ResetStatusTime
//----------------------------------------
static int L_ResetStatusTime (lua_State *L)
  {
  doc (L)->ResetStatusTime ();
  return 0;  // number of result fields
  } // end of L_ResetStatusTime


//----------------------------------------
//  world.ResetTimer
//----------------------------------------
static int L_ResetTimer (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->ResetTimer (
      my_checkstring (L, 1)      // TimerName
      ));
  return 1;  // number of result fields
  } // end of L_ResetTimer


//----------------------------------------
//  world.ResetTimers
//----------------------------------------
static int L_ResetTimers (lua_State *L)
  {
  doc (L)->ResetTimers ();
  return 0;  // number of result fields
  } // end of L_ResetTimers


//----------------------------------------
//  world.ReverseSpeedwalk
//----------------------------------------
static int L_ReverseSpeedwalk (lua_State *L)
  {
  doc (L);
  // this is a helper function that returns a CString, not a BSTR
  lua_pushstring (L, CMUSHclientDoc::DoReverseSpeedwalk (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_ReverseSpeedwalk


//----------------------------------------
//  world.RGBColourToName
//----------------------------------------
static int L_RGBColourToName (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  BSTR str = pDoc->RGBColourToName ((long) my_checknumber (L, 1));
  return pushBstr (L, str);
  } // end of L_RGBColourToName


//----------------------------------------
//  world.Save
//----------------------------------------
static int L_Save (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  CString strOldName = pDoc->GetPathName ();
  bool bSaveAs = optboolean (L, 2, 0);  // Save-As flag
  if (bSaveAs) 
    lua_pushboolean (L, pDoc->Save (my_checkstring (L, 1)));
  else
    lua_pushboolean (L, pDoc->Save (my_optstring (L, 1, "")));
  if (bSaveAs)
    pDoc->SetPathName (strOldName, FALSE);
  return 1;  // number of result fields
  } // end of L_Save


//----------------------------------------
//  world.SaveNotepad
//----------------------------------------
static int L_SaveNotepad (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->SaveNotepad (
      my_checkstring (L, 1),     // Title
      my_checkstring (L, 2),     // FileName
      optboolean (L, 3, 0)     // ReplaceExisting
      ));
  return 1;  // number of result fields
  } // end of L_SaveNotepad


//----------------------------------------
//  world.SaveState
//----------------------------------------
static int L_SaveState (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->SaveState ());
  return 1;  // number of result fields
  } // end of L_SaveState


//----------------------------------------
//  world.SelectCommand
//----------------------------------------
static int L_SelectCommand (lua_State *L)
  {
  doc (L)->SelectCommand ();
  return 0;  // number of result fields
  } // end of L_SelectCommand


//----------------------------------------
//  world.Send
//----------------------------------------
static int L_Send (lua_State *L)
  {
  CMUSHclientDoc * pDoc = doc (L);  // must do this first
  lua_pushnumber (L, pDoc->Send (concatArgs (L).c_str ()));
  return 1;  // number of result fields
  } // end of L_Send


//----------------------------------------
//  world.SendImmediate
//----------------------------------------
static int L_SendImmediate (lua_State *L)
  {
  CMUSHclientDoc * pDoc = doc (L);  // must do this first
  lua_pushnumber (L, pDoc->SendImmediate (concatArgs (L).c_str ()));
  return 1;  // number of result fields
  } // end of L_SendImmediate


//----------------------------------------
//  world.SendNoEcho
//----------------------------------------
static int L_SendNoEcho (lua_State *L)
  {
  CMUSHclientDoc * pDoc = doc (L);  // must do this first
  lua_pushnumber (L, pDoc->SendNoEcho (concatArgs (L).c_str ()));
  return 1;  // number of result fields
  } // end of L_SendNoEcho

//----------------------------------------
//  world.SendPkt
//----------------------------------------
static int L_SendPkt (lua_State *L)
  {
  CMUSHclientDoc * pDoc = doc (L);  // must do this first

  long result = eWorldClosed;

  if (pDoc->m_iConnectPhase == eConnectConnectedToMud)
    {
    // get text to send (can include nulls)
    size_t textLength;
    const char * text = my_checklstring (L, 1, &textLength);
    pDoc->SendPacket (text, textLength);
	  result = eOK;
    }

  lua_pushnumber (L, result);
  return 1;  // number of result fields
  } // end of L_SendPkt

//----------------------------------------
//  world.SendPush
//----------------------------------------
static int L_SendPush (lua_State *L)
  {
  CMUSHclientDoc * pDoc = doc (L);  // must do this first
  lua_pushnumber (L, pDoc->SendPush (concatArgs (L).c_str ()));
  return 1;  // number of result fields
  } // end of L_SendPush

//----------------------------------------
//  world.SendSpecial
//----------------------------------------
static int L_SendSpecial (lua_State *L)
  {
  CMUSHclientDoc * pDoc = doc (L);  // must do this first
  lua_pushnumber (L, pDoc->SendSpecial (my_checkstring (L, 1),
                  optboolean (L, 2, 0),   // Echo
                  optboolean (L, 3, 0),   // Queue
                  optboolean (L, 4, 0),   // Log
                  optboolean (L, 5, 0))   // History
                  );
  return 1;  // number of result fields
  } // end of L_SendSpecial

//----------------------------------------
//  world.SendToNotepad
//----------------------------------------
static int L_SendToNotepad (lua_State *L)
  {
  CMUSHclientDoc * pDoc = doc (L);  // must do this first
  lua_pushboolean (L, pDoc->SendToNotepad (
      my_checkstring (L, 1), // Title
      concatArgs (L, "", 2).c_str ()  // Contents
      ));
  return 1;  // number of result fields
  } // end of L_SendToNotepad


//----------------------------------------
//  world.SetAliasOption
//----------------------------------------
static int L_SetAliasOption (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->SetAliasOption (
      my_checkstring (L, 1),    // AliasName
      my_checkstring (L, 2),    // OptionName
      get_option_value (L, 3)   // Value
      ));
  return 1;  // number of result fields
  } // end of L_SetAliasOption


//----------------------------------------
//  world.SetAlphaOption
//----------------------------------------
static int L_SetAlphaOption (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->SetAlphaOption (
      my_checkstring (L, 1),    // OptionName
      my_checkstring (L, 2)     // Value
      ));
  return 1;  // number of result fields
  } // end of L_SetAlphaOption

//----------------------------------------
//  world.SetBackgroundColour
//----------------------------------------
static int L_SetBackgroundColour (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->SetBackgroundColour (
      (long) my_checknumber (L, 1)     // RGB colour
      ));
  return 1;  // number of result fields
  } // end of L_SetBackgroundColour

//----------------------------------------
//  world.SetBackgroundImage
//----------------------------------------
static int L_SetBackgroundImage (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->SetBackgroundImage (
      my_checkstring (L, 1),    // FileName
      (short) my_checknumber (L, 2)     // mode
      ));
  return 1;  // number of result fields
  } // end of L_SetBackgroundImage

//----------------------------------------
//  world.SetChatOption
//----------------------------------------
static int L_SetChatOption (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->SetChatOption (
      (long) my_checknumber (L, 1),  // Chat ID
      my_checkstring (L, 2),         // OptionName
      get_option_value (L, 3)        // Value
      ));
  return 1;  // number of result fields
  } // end of L_SetChatOption


//----------------------------------------
//  world.SetChanged
//----------------------------------------
static int L_SetChanged (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->SetChanged (optboolean (L, 1, 1));
  return 0;  // number of result fields
  } // end of L_SetChanged

//----------------------------------------
//  world.SetClipboard
//----------------------------------------
static int L_SetClipboard (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->SetClipboard (concatArgs (L).c_str ());
  return 0;  // number of result fields
  } // end of L_SetClipboard

//----------------------------------------
//  world.SetCommand
//----------------------------------------
static int L_SetCommand (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->SetCommand (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_SetCommand

//----------------------------------------
//  world.SetCommandSelection
//----------------------------------------
static int L_SetCommandSelection (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->SetCommandSelection (
                    (long) my_checknumber (L, 1),   // First
                    (long) my_checknumber (L, 2))); // Last
  return 1;  // number of result fields
  } // end of L_SetCommandSelection

//----------------------------------------
//  world.SetCommandWindowHeight
//----------------------------------------
static int L_SetCommandWindowHeight (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->SetCommandWindowHeight (
	              (short) my_checknumber (L, 1)));
  return 1;  // number of result fields
  } // end of L_SetCommandWindowHeight

//----------------------------------------
//  world.SetCustomColourName
//----------------------------------------
static int L_SetCustomColourName (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber(L, pDoc->SetCustomColourName (
      (short) my_checknumber (L, 1),  // WhichColour
      my_checkstring (L, 2)           // Name
      ));
  return 1;  // number of result fields
  } // end of L_SetCustomColourName

//----------------------------------------
//  world.SetEntity
//----------------------------------------
static int L_SetEntity (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->SetEntity (my_checkstring (L, 1),
                   my_optstring (L, 2, ""));
  return 0;  // number of result fields
  } // end of L_SetEntity

//----------------------------------------
//  world.SetForegroundImage
//----------------------------------------
static int L_SetForegroundImage (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->SetForegroundImage (
      my_checkstring (L, 1),            // FileName
      (short) my_checknumber (L, 2)     // mode
      ));
  return 1;  // number of result fields
  } // end of L_SetForegroundImage


//----------------------------------------
//  world.SetInputFont
//----------------------------------------
static int L_SetInputFont (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->SetInputFont (
      my_checkstring (L, 1),          // FontName
      (short) my_checknumber (L, 2),  // PointSize
      (short) my_checknumber (L, 3),  // Weight
      (BOOL) my_optnumber (L, 4, 0)   // Italic
      );
  return 0;  // number of result fields
  } // end of L_SetInputFont


//----------------------------------------
//  world.SetNotes
//----------------------------------------
static int L_SetNotes (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->SetNotes (my_checkstring (L, 1));
  return 0;  // number of result fields
  } // end of L_SetNotes


//----------------------------------------
//  world.SetOption
//----------------------------------------
static int L_SetOption (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_Number option;

    // if boolean convert to 0 or 1
  if (lua_isboolean (L, 2))
    option = lua_toboolean (L, 2) ? 1 : 0;
  else if (lua_isnil (L, 2))
    option = 0;
  else
    option = my_checknumber (L, 2);

  lua_pushnumber (L, pDoc->SetOption (
      my_checkstring (L, 1),    // OptionName
      (long) option             // Value
      ));
  return 1;  // number of result fields
  } // end of L_SetOption


//----------------------------------------
//  world.SetOutputFont
//----------------------------------------
static int L_SetOutputFont (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->SetOutputFont (
      my_checkstring (L, 1),          // FontName
      (short) my_checknumber (L, 2)   // PointSize
      );
  return 0;  // number of result fields
  } // end of L_SetOutputFont

//----------------------------------------
//  world.SetScroll
//----------------------------------------
static int L_SetScroll (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->SetScroll (
      (long) my_checknumber (L, 1),    // position
      optboolean (L, 2, 1)      // enabled flag, defaults to true
      ));
  return 1;  // number of result fields
  } // end of L_SetScroll

//----------------------------------------
//  world.SetStatus
//----------------------------------------
static int L_SetStatus (lua_State *L)
  {
  CMUSHclientDoc * pDoc = doc (L);  // must do this first
  pDoc->SetStatus (concatArgs (L).c_str ());
  return 0;  // number of result fields
  } // end of L_SetStatus


//----------------------------------------
//  world.SetTimerOption
//----------------------------------------
static int L_SetTimerOption (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->SetTimerOption (
      my_checkstring (L, 1),    // TimerName
      my_checkstring (L, 2),    // OptionName
      get_option_value (L, 3)   // Value
      ));
  return 1;  // number of result fields
  } // end of L_SetTimerOption

//----------------------------------------
//  world.SetToolBarPosition
//----------------------------------------
static int L_SetToolBarPosition (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->SetToolBarPosition (
      (short) my_checknumber (L, 1),   // Which
      optboolean (L, 2, 0),            // Float
      (short) my_checknumber (L, 3),   // Side
      (long) my_checknumber (L, 4),    // Top
      (long) my_checknumber (L, 5)     // Left
      ));
  return 1;  // number of result fields
  } // end of L_SetToolBarPosition

//----------------------------------------
//  world.SetTriggerOption
//----------------------------------------
static int L_SetTriggerOption (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->SetTriggerOption (
      my_checkstring (L, 1),    // TriggerName
      my_checkstring (L, 2),    // OptionName
      get_option_value (L, 3)   // Value
      ));
  return 1;  // number of result fields
  } // end of L_SetTriggerOption


//----------------------------------------
//  world.SetVariable
//----------------------------------------
static int L_SetVariable (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->SetVariable (
      my_checkstring (L, 1),    // VariableName
      my_checkstring (L, 2)     // Contents
      ));
  return 1;  // number of result fields
  } // end of L_SetVariable

//----------------------------------------
//  world.SetWorldWindowStatus
//----------------------------------------
static int L_SetWorldWindowStatus (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->SetWorldWindowStatus ((short) my_checknumber (L, 1));
  return 0;  // number of result fields
  } // end of L_SetWorldWindowStatus

//----------------------------------------
//  world.ShiftTabCompleteItem
//----------------------------------------
static int L_ShiftTabCompleteItem (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L,pDoc->ShiftTabCompleteItem (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_ShiftTabCompleteItem

//----------------------------------------
//  world.ShowInfoBar
//----------------------------------------
static int L_ShowInfoBar (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->ShowInfoBar (optboolean (L, 1, 1));
  return 0;  // number of result fields
  } // end of L_ShowInfoBar

//----------------------------------------
//  world.Simulate
//----------------------------------------
static int L_Simulate (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->Simulate (concatArgs (L).c_str ());
  return 0;  // number of result fields
  } // end of L_Simulate

//----------------------------------------
//  world.Sound
//----------------------------------------
static int L_Sound (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->Sound (
      my_checkstring (L, 1)    // SoundFileName
      ));
  return 1;  // number of result fields
  } // end of L_Sound


//----------------------------------------
//  world.GetSpeedWalkDelay
//----------------------------------------
static int L_GetSpeedWalkDelay (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->GetSpeedWalkDelay ());
  return 1;  // number of result fields
  } // end of GetSpeedWalkDelay

//----------------------------------------
//  world.SetSpeedWalkDelay
//----------------------------------------
static int L_SetSpeedWalkDelay (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->SetSpeedWalkDelay (
      (short) my_checknumber (L, 1)   // nNewValue
      );
  return 0;  // number of result fields
  } // end of SetSpeedWalkDelay

//----------------------------------------
//  world.SpellCheck
//----------------------------------------
static int L_SpellCheck (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->SpellCheck (my_checkstring (L, 1));
  return pushVariant (L, v);
  } // end of L_SpellCheck

//----------------------------------------
//  world.SpellCheckCommand
//----------------------------------------
static int L_SpellCheckCommand (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->SpellCheckCommand (
            (long) my_optnumber (L, 1, -1),  // Start column
            (long) my_optnumber (L, 2, -1)   // End column
            ));
  return 1;  // number of result fields
  } // end of SpellCheckCommand

//----------------------------------------
//  world.SpellCheckDlg
//----------------------------------------
static int L_SpellCheckDlg (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->SpellCheckDlg (my_checkstring (L, 1));
  return pushVariant (L, v);
  } // end of L_SpellCheckDlg

// prototype ;)
CString StripAnsi (const CString strMessage);

//----------------------------------------
//  world.StripANSI
//----------------------------------------
static int L_StripANSI (lua_State *L)
  {
  // this is a helper function that returns a CString, not a BSTR
  lua_pushstring (L, StripAnsi (my_checkstring (L, 1)));
  return 1;  // number of result fields
  } // end of L_StripANSI

//----------------------------------------
//  world.StopSound
//----------------------------------------
static int L_StopSound (lua_State *L)
  {
  CMUSHclientDoc * pDoc = doc (L);  // must do this first
  lua_pushnumber (L, pDoc->StopSound (
	              (short) my_optnumber (L, 1, 0)));   // Buffer
  return 1;  // number of result fields
  } // end of L_StopSound

//----------------------------------------
//  world.Tell
//----------------------------------------
static int L_Tell (lua_State *L)
  {
  CMUSHclientDoc * pDoc = doc (L);  // must do this first
  pDoc->Tell (concatArgs (L).c_str ());
  return 0;  // number of result fields
  } // end of L_Tell


//----------------------------------------
//  world.TextRectangle
//----------------------------------------
static int L_TextRectangle (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->TextRectangle (
            (long) my_checknumber (L, 1),    // Left
            (long) my_checknumber (L, 2),    // Top
            (long) my_checknumber (L, 3),    // Right
            (long) my_checknumber (L, 4),    // Bottom
            (long) my_checknumber (L, 5),    // BorderOffset
            (long) my_checknumber (L, 6),    // BorderColour
            (long) my_checknumber (L, 7),    // BorderWidth
            (long) my_checknumber (L, 8),    // OutsideFillColour
            (long) my_checknumber (L, 9)     // OutsideFillStyle
            ));
  return 1;  // number of result fields
  } // end of L_TextRectangle


//----------------------------------------
//  world.GetTrace
//----------------------------------------
static int L_GetTrace (lua_State *L)
  {
  lua_pushboolean (L, doc (L)->m_bTrace);
  return 1;  // number of result fields
  } // end of L_GetTrace

//----------------------------------------
//  world.SetTrace
//----------------------------------------
static int L_SetTrace (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  pDoc->m_bTrace = optboolean (L, 1, 1) != 0;
  return 0;  // number of result fields
  } // end of L_SetTrace


//----------------------------------------
//  world.TraceOut
//----------------------------------------
static int L_TraceOut (lua_State *L)
  {
  CMUSHclientDoc * pDoc = doc (L);  // must do this first
  pDoc->TraceOut (concatArgs (L).c_str ());
  return 0;  // number of result fields
  } // end of L_TraceOut

//----------------------------------------
//  world.TranslateGerman
//----------------------------------------
static int L_TranslateGerman (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  BSTR str = pDoc->TranslateGerman (
                  my_checkstring (L, 1)  // Text
                  );
  return pushBstr (L, str);  // number of result fields
  } // end of L_TranslateGerman

//----------------------------------------
//  world.TranslateDebug
//----------------------------------------
static int L_TranslateDebug (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->TranslateDebug (
                  my_optstring (L, 1, "")  // Message
                  ));
  return 1;  // number of result fields
  } // end of L_TranslateDebug

//----------------------------------------
//  world.Transparency
//----------------------------------------
static int L_Transparency (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushboolean (L, pDoc->Transparency (
          (long) my_checknumber (L, 1),    // Key
          (short) my_checknumber (L, 2)    // Amount
      ));
  return 1;  // number of result fields
  } // end of L_Transparency

//----------------------------------------
//  world.Trim
//----------------------------------------
static int L_Trim (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  BSTR str = pDoc->Trim (
                  my_checkstring (L, 1)  // Source
                  );
  return pushBstr (L, str);  // number of result fields
  } // end of L_Trim

//----------------------------------------
//  world.UdpListen
//----------------------------------------
static int L_UdpListen (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->UdpListen (
            my_checkstring (L, 1),          // IP
            (short) my_checknumber (L, 2),  // Port
            my_checkstring (L, 3)           // Script
            ));
  return 1;  // number of result fields
  } // end of L_UdpListen

//----------------------------------------
//  world.UdpPortList
//----------------------------------------
static int L_UdpPortList (lua_State *L)
  {
  CMUSHclientDoc * pDoc = doc (L);  // must do this first
  VARIANT v = pDoc->UdpPortList ();
  return pushVariant (L, v);
  } // end of L_UdpPortList

//----------------------------------------
//  world.UdpSend
//----------------------------------------
static int L_UdpSend (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->UdpSend (
            my_checkstring (L, 1),  // IP
            (short) my_checknumber (L, 2),  // Port
            my_checkstring (L, 3)   // Text
            ));
  return 1;  // number of result fields
  } // end of L_UdpSend

//----------------------------------------
//  world.Version
//----------------------------------------
static int L_Version (lua_State *L)
  {
  BSTR str = doc (L)->Version ();
  return pushBstr (L, str);  // number of result fields
  } // end of L_Version

//----------------------------------------
//  world.WindowAddHotspot
//----------------------------------------
static int L_WindowAddHotspot (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowAddHotspot (
            my_checkstring (L, 1),  // Name
            my_checkstring (L, 2),  // HotspotId
            (long) my_checknumber (L, 3),  // Left
            (long) my_checknumber (L, 4),  // Top
            (long) my_checknumber (L, 5),  // Right
            (long) my_checknumber (L, 6),  // Bottom
            my_optstring (L, 7, ""),  // MouseOver
            my_optstring (L, 8, ""),  // CancelMouseOver
            my_optstring (L, 9, ""),  // MouseDown
            my_optstring (L, 10, ""), // CancelMouseDown
            my_optstring (L, 11, ""), // MouseUp
            my_optstring (L, 12, ""), // TooltipText
            (long) my_optnumber (L, 13, 0),  // Cursor
            (long) my_optnumber (L, 14, 0)   // Flags

            ));
  return 1;  // number of result fields
  } // end of L_WindowAddHotspot

//----------------------------------------
//  world.WindowArc
//----------------------------------------
static int L_WindowArc (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowArc (
            my_checkstring (L, 1),  // Name
            (long) my_checknumber (L, 2),  // Left
            (long) my_checknumber (L, 3),  // Top
            (long) my_checknumber (L, 4),  // Right
            (long) my_checknumber (L, 5),  // Bottom
            (long) my_checknumber (L, 6),  // x1
            (long) my_checknumber (L, 7),  // y2
            (long) my_checknumber (L, 8),  // x1
            (long) my_checknumber (L, 9),  // y2
            (long) my_checknumber (L, 10),  // PenColour
            (long) my_checknumber (L, 11),  // PenStyle
            (long) my_checknumber (L, 12)   // PenWidth
            ));
  return 1;  // number of result fields
  } // end of L_WindowArc

//----------------------------------------
//  world.WindowBezier
//----------------------------------------
static int L_WindowBezier (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowBezier (
            my_checkstring (L, 1),  // Name
            my_checkstring (L, 2),  // Points
            (long) my_checknumber (L, 3),  // PenColour
            (long) my_checknumber (L, 4),  // PenStyle
            (long) my_checknumber (L, 5)   // PenWidth
            ));
  return 1;  // number of result fields
  } // end of L_WindowBezier

//----------------------------------------
//  world.WindowBlendImage
//----------------------------------------
static int L_WindowBlendImage (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowBlendImage (
            my_checkstring (L, 1),    // Name
            my_checkstring (L, 2),    // ImageId
            (long) my_checknumber (L, 3),    // Left
            (long) my_checknumber (L, 4),    // Top
            (long) my_checknumber (L, 5),    // Right
            (long) my_checknumber (L, 6),    // Bottom
            (short) my_checknumber (L, 7),   // Mode
            my_optnumber (L, 8, 1),   // Opacity
            (long) my_optnumber (L, 9, 0),   // SrcLeft
            (long) my_optnumber (L, 10, 0),  // SrcTop
            (long) my_optnumber (L, 11, 0),  // SrcRight
            (long) my_optnumber (L, 12, 0)   // SrcBottom
            ));
  return 1;  // number of result fields
  } // end of L_WindowBlendImage

//----------------------------------------
//  world.WindowCircleOp
//----------------------------------------
static int L_WindowCircleOp (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowCircleOp (
            my_checkstring (L, 1),    // Name
            (short) my_checknumber (L, 2),   // Action
            (long) my_checknumber (L, 3),    // Left
            (long) my_checknumber (L, 4),    // Top
            (long) my_checknumber (L, 5),    // Right
            (long) my_checknumber (L, 6),    // Bottom
            (long) my_checknumber (L, 7),    // PenColour
            (long) my_checknumber (L, 8),    // PenStyle
            (long) my_checknumber (L, 9),    // PenWidth
            (long) my_checknumber (L, 10),   // BrushColour
            (long) my_optnumber (L, 11, 0),  // BrushStyle
            (long) my_optnumber (L, 12, 0),  // Extra1
            (long) my_optnumber (L, 13, 0),  // Extra2
            (long) my_optnumber (L, 14, 0),  // Extra3
            (long) my_optnumber (L, 15, 0)   // Extra4
            ));
  return 1;  // number of result fields
  } // end of L_WindowCircleOp

//----------------------------------------
//  world.WindowCreate
//----------------------------------------
static int L_WindowCreate (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowCreate (
      my_checkstring (L, 1),    // Name
      (long) my_checknumber (L, 2),    // Left
      (long) my_checknumber (L, 3),    // Top
      (long) my_checknumber (L, 4),    // Width
      (long) my_checknumber (L, 5),    // Height
      (short) my_checknumber (L, 6),   // Position
      (long) my_checknumber (L, 7),    // Flags
      (long) my_checknumber (L, 8)     // Background Colour
    ));

  return 1;  // number of result fields
  } // end of L_WindowCreate

//----------------------------------------
//  world.WindowCreateImage
//----------------------------------------
static int L_WindowCreateImage (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowCreateImage (
      my_checkstring (L, 1),    // Name
      my_checkstring (L, 2),    // ImageId
      (long) my_checknumber (L, 3),    // Row1
      (long) my_checknumber (L, 4),    // Row2
      (long) my_checknumber (L, 5),    // Row3
      (long) my_checknumber (L, 6),    // Row4
      (long) my_checknumber (L, 7),    // Row5
      (long) my_checknumber (L, 8),    // Row6
      (long) my_checknumber (L, 9),    // Row7
      (long) my_checknumber (L, 10)    // Row8
    ));

  return 1;  // number of result fields
  } // end of L_WindowCreateImage

//----------------------------------------
//  world.WindowDelete
//----------------------------------------
static int L_WindowDelete (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowDelete (
            my_checkstring (L, 1)  // Name
            ));
  return 1;  // number of result fields
  } // end of L_WindowDelete

//----------------------------------------
//  world.WindowDeleteAllHotspots
//----------------------------------------
static int L_WindowDeleteAllHotspots (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowDeleteAllHotspots (
            my_checkstring (L, 1)  // Name
            ));
  return 1;  // number of result fields
  } // end of L_WindowDeleteAllHotspots

//----------------------------------------
//  world.WindowDeleteHotspot
//----------------------------------------
static int L_WindowDeleteHotspot (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowDeleteHotspot (
            my_checkstring (L, 1),  // Name
            my_checkstring (L, 2)   // HotspotId
            ));
  return 1;  // number of result fields
  } // end of L_WindowDeleteHotspot

//----------------------------------------
//  world.WindowDragHandler
//----------------------------------------
static int L_WindowDragHandler (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowDragHandler (
            my_checkstring (L, 1),    // Name
            my_checkstring (L, 2),    // HotspotId
            my_optstring (L, 3, ""),  // MoveCallback
            my_optstring (L, 4, ""),  // ReleaseCallback
            (long) my_optnumber (L, 5, 0)    // Flags
            ));
  return 1;  // number of result fields
  } // end of L_WindowDragHandler

//----------------------------------------
//  world.WindowDrawImage
//----------------------------------------
static int L_WindowDrawImage (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowDrawImage (
            my_checkstring (L, 1),    // Name
            my_checkstring (L, 2),    // ImageId
            (long) my_checknumber (L, 3),    // Left
            (long) my_checknumber (L, 4),    // Top
            (long) my_checknumber (L, 5),    // Right
            (long) my_checknumber (L, 6),    // Bottom
            (short) my_optnumber (L, 7, 1),  // Mode
            (long) my_optnumber (L, 8, 0),   // SrcLeft
            (long) my_optnumber (L, 9, 0),   // SrcTop
            (long) my_optnumber (L, 10, 0),  // SrcRight
            (long) my_optnumber (L, 11, 0)   // SrcBottom
            ));
  return 1;  // number of result fields
  } // end of L_WindowDrawImage

//----------------------------------------
//  world.WindowDrawImageAlpha
//----------------------------------------
static int L_WindowDrawImageAlpha (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowDrawImageAlpha (
            my_checkstring (L, 1),    // Name
            my_checkstring (L, 2),    // ImageId
            (long) my_checknumber (L, 3),    // Left
            (long) my_checknumber (L, 4),    // Top
            (long) my_checknumber (L, 5),    // Right
            (long) my_checknumber (L, 6),    // Bottom
            my_optnumber (L, 7, 1),   // Opacity
            (long) my_optnumber (L, 8, 0),   // SrcLeft
            (long) my_optnumber (L, 9, 0)    // SrcTop
            ));
  return 1;  // number of result fields
  } // end of L_WindowDrawImageAlpha

//----------------------------------------
//  world.WindowFilter
//----------------------------------------
static int L_WindowFilter (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowFilter (
            my_checkstring (L, 1),    // Name
            (long) my_checknumber (L, 2),    // Left
            (long) my_checknumber (L, 3),    // Top
            (long) my_checknumber (L, 4),    // Right
            (long) my_checknumber (L, 5),    // Bottom
            (short) my_checknumber (L, 6),   // Operation
            my_checknumber (L, 7)     // Options
            ));
  return 1;  // number of result fields
  } // end of L_WindowFilter

//----------------------------------------
//  world.WindowFont
//----------------------------------------
static int L_WindowFont (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowFont (
      my_checkstring (L, 1),    // Name
      my_checkstring (L, 2),    // FontId
      my_checkstring (L, 3),    // FontName
      my_checknumber (L, 4),    // Size
      optboolean (L, 5, 0),    // Bold
      optboolean (L, 6, 0),    // Italic
      optboolean (L, 7, 0),    // Underline
      optboolean (L, 8, 0),    // Strikeout
      (short) my_optnumber (L, 9, DEFAULT_CHARSET),  // Charset
      (short) my_optnumber (L, 10, FF_DONTCARE)  // PitchAndFamily
    ));

  return 1;  // number of result fields
  } // end of L_WindowFont

//----------------------------------------
//  world.WindowFontInfo
//----------------------------------------
static int L_WindowFontInfo (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->WindowFontInfo (
      my_checkstring (L, 1),    // Name
      my_checkstring (L, 2),    // FontId
      (long) my_checknumber (L, 3)     // InfoType
    );

  return pushVariant (L, v);
  } // end of L_WindowFontInfo

//----------------------------------------
//  world.WindowFontList
//----------------------------------------
static int L_WindowFontList (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->WindowFontList (my_checkstring (L, 1)); // name
  return pushVariant (L, v);  // number of result fields
  } // end of L_WindowFontList

//----------------------------------------
//  world.WindowGetImageAlpha
//----------------------------------------
static int L_WindowGetImageAlpha (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowGetImageAlpha (
            my_checkstring (L, 1),    // Name
            my_checkstring (L, 2),    // ImageId
            (long) my_checknumber (L, 3),    // Left
            (long) my_checknumber (L, 4),    // Top
            (long) my_checknumber (L, 5),    // Right
            (long) my_checknumber (L, 6),    // Bottom
            (long) my_optnumber (L, 7, 0),   // SrcLeft
            (long) my_optnumber (L, 8, 0)    // SrcTop
            ));
  return 1;  // number of result fields
  } // end of L_WindowGetImageAlpha

//----------------------------------------
//  world.WindowGetPixel
//----------------------------------------
static int L_WindowGetPixel (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowGetPixel (
            my_checkstring (L, 1),  // Name
            (long) my_checknumber (L, 2),  // x
            (long) my_checknumber (L, 3)   // y
            ));
  return 1;  // number of result fields
  } // end of L_WindowGetPixel

//----------------------------------------
//  world.WindowGradient
//----------------------------------------
static int L_WindowGradient (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowGradient (
            my_checkstring (L, 1),           // Name
            (long) my_checknumber (L, 2),    // Left
            (long) my_checknumber (L, 3),    // Top
            (long) my_checknumber (L, 4),    // Right
            (long) my_checknumber (L, 5),    // Bottom
            (long) my_checknumber (L, 6),    // StartColour
            (long) my_checknumber (L, 7),    // EndColour
            (short) my_checknumber (L, 8)    // Mode
            ));
  return 1;  // number of result fields
  } // end of L_WindowGradient

//----------------------------------------
//  world.WindowHotspotInfo
//----------------------------------------
static int L_WindowHotspotInfo (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->WindowHotspotInfo (
      my_checkstring (L, 1),    // Name
      my_checkstring (L, 2),    // HotspotId
      (long) my_checknumber (L, 3)     // InfoType
    );

  return pushVariant (L, v);
  } // end of L_WindowHotspotInfo

//----------------------------------------
//  world.WindowHotspotList
//----------------------------------------
static int L_WindowHotspotList (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->WindowHotspotList (my_checkstring (L, 1)); // name
  return pushVariant (L, v);  // number of result fields
  } // end of L_WindowHotspotList

//----------------------------------------
//  world.WindowHotspotTooltip
//----------------------------------------
static int L_WindowHotspotTooltip (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowHotspotTooltip (
      my_checkstring (L, 1),    // Name
      my_checkstring (L, 2),    // HotspotId
      my_checkstring (L, 3)     // TooltipText
    ));

  return 1;
  } // end of L_WindowHotspotTooltip

//----------------------------------------
//  world.WindowImageFromWindow
//----------------------------------------
static int L_WindowImageFromWindow (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowImageFromWindow (
      my_checkstring (L, 1),    // Name
      my_checkstring (L, 2),    // ImageId
      my_checkstring (L, 3)     // SourceWindow
    ));

  return 1;
  } // end of L_WindowImageFromWindow

//----------------------------------------
//  world.WindowImageInfo
//----------------------------------------
static int L_WindowImageInfo (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->WindowImageInfo (
      my_checkstring (L, 1),    // Name
      my_checkstring (L, 2),    // ImageId
      (long) my_checknumber (L, 3)     // InfoType
    );

  return pushVariant (L, v);
  } // end of L_WindowImageInfo

//----------------------------------------
//  world.WindowImageList
//----------------------------------------
static int L_WindowImageList (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->WindowImageList (my_checkstring (L, 1)); // name
  return pushVariant (L, v);  // number of result fields
  } // end of L_WindowImageList

//----------------------------------------
//  world.WindowImageOp
//----------------------------------------
static int L_WindowImageOp (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowImageOp (
            my_checkstring (L, 1),           // Name
            (short) my_checknumber (L, 2),   // Action
            (long) my_checknumber (L, 3),    // Left
            (long) my_checknumber (L, 4),    // Top
            (long) my_checknumber (L, 5),    // Right
            (long) my_checknumber (L, 6),    // Bottom
            (long) my_checknumber (L, 7),    // PenColour
            (long) my_checknumber (L, 8),    // PenStyle
            (long) my_checknumber (L, 9),    // PenWidth
            (long) my_checknumber (L, 10),   // BrushColour
            my_checkstring (L, 11),          // ImageId
            (long) my_optnumber (L, 12, 0),  // EllipseWidth
            (long) my_optnumber (L, 13, 0)   // EllipseHeight
            ));
  return 1;  // number of result fields
  } // end of L_WindowImageOp

//----------------------------------------
//  world.WindowInfo
//----------------------------------------
static int L_WindowInfo (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  VARIANT v = pDoc->WindowInfo (
      my_checkstring (L, 1),           // Name
      (long) my_checknumber (L, 2)     // InfoType
    );

  return pushVariant (L, v);
  } // end of L_WindowInfo

//----------------------------------------
//  world.WindowLine
//----------------------------------------
static int L_WindowLine (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowLine (
            my_checkstring (L, 1),  // Name
            (long) my_checknumber (L, 2),  // x1
            (long) my_checknumber (L, 3),  // y2
            (long) my_checknumber (L, 4),  // x1
            (long) my_checknumber (L, 5),  // y2
            (long) my_checknumber (L, 6),  // PenColour
            (long) my_checknumber (L, 7),  // PenStyle
            (long) my_checknumber (L, 8)   // PenWidth
            ));
  return 1;  // number of result fields
  } // end of L_WindowLine

//----------------------------------------
//  world.WindowList
//----------------------------------------
static int L_WindowList (lua_State *L)
  {
  VARIANT v = doc (L)->WindowList ();
  return pushVariant (L, v);  // number of result fields
  } // end of L_WindowList

//----------------------------------------
//  world.WindowLoadImage
//----------------------------------------
static int L_WindowLoadImage (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowLoadImage (
      my_checkstring (L, 1),    // Name
      my_checkstring (L, 2),    // FontId
      my_checkstring (L, 3)     // FileName
     ));

  return 1;  // number of result fields
  } // end of L_WindowLoadImage

//----------------------------------------
//  world.WindowLoadImageMemory
//----------------------------------------
static int L_WindowLoadImageMemory (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  LPCTSTR sBuffer;
  size_t  iBufferLength;
  sBuffer = luaL_checklstring (L, 3, &iBufferLength);

  lua_pushnumber (L, pDoc->WindowLoadImageMemory (
      my_checkstring (L, 1),    // Name
      my_checkstring (L, 2),    // FontId
      (unsigned char *) sBuffer,
      iBufferLength,
      optboolean (L, 4, 0)      // swap alpha?
     ));

  return 1;  // number of result fields
  } // end of L_WindowLoadImageMemory

//----------------------------------------
//  world.WindowMenu
//----------------------------------------
static int L_WindowMenu (lua_State *L)
  {
  BSTR str;  

  CMUSHclientDoc *pDoc = doc (L);
  str = pDoc->WindowMenu (
      my_checkstring (L, 1),    // Name
      (long) my_checknumber (L, 2),    // Left
      (long) my_checknumber (L, 3),    // Top
      my_checkstring (L, 4)     // Items
     );

  return pushBstr (L, str);  // number of result fields
  } // end of L_WindowMenu

//----------------------------------------
//  world.WindowMergeImageAlpha
//----------------------------------------
static int L_WindowMergeImageAlpha (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowMergeImageAlpha (
            my_checkstring (L, 1),    // Name
            my_checkstring (L, 2),    // ImageId
            my_checkstring (L, 3),    // MaskId
            (long) my_checknumber (L, 4),    // Left
            (long) my_checknumber (L, 5),    // Top
            (long) my_checknumber (L, 6),    // Right
            (long) my_checknumber (L, 7),    // Bottom
            (short) my_checknumber (L, 8),   // Mode
            my_optnumber (L, 9, 1),   // Opacity
            (long) my_optnumber (L, 10, 0),  // SrcLeft
            (long) my_optnumber (L, 11, 0),  // SrcTop
            (long) my_optnumber (L, 12, 0),  // SrcRight
            (long) my_optnumber (L, 13, 0)   // SrcBottom
            ));
  return 1;  // number of result fields
  } // end of L_WindowMergeImageAlpha


//----------------------------------------
//  world.WindowMoveHotspot
//----------------------------------------
static int L_WindowMoveHotspot (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowMoveHotspot (
            my_checkstring (L, 1),  // Name
            my_checkstring (L, 2),  // HotspotId
            (long) my_checknumber (L, 3),  // Left
            (long) my_checknumber (L, 4),  // Top
            (long) my_checknumber (L, 5),  // Right
            (long) my_checknumber (L, 6)   // Bottom
            ));
  return 1;  // number of result fields
  } // end of L_WindowMoveHotspot

//----------------------------------------
//  world.WindowPolygon
//----------------------------------------
static int L_WindowPolygon (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowPolygon (
            my_checkstring (L, 1),  // Name
            my_checkstring (L, 2),  // Points
            (long) my_checknumber (L, 3),  // PenColour
            (long) my_checknumber (L, 4),  // PenStyle
            (long) my_checknumber (L, 5),  // PenWidth
            (long) my_checknumber (L, 6),  // BrushColour
            (long) my_optnumber (L, 7, 0), // BrushStyle
            optboolean (L, 8, 0),   // Close
            optboolean (L, 9, 0)    // Winding
            ));
  return 1;  // number of result fields
  } // end of L_WindowPolygon

//----------------------------------------
//  world.WindowPosition
//----------------------------------------
static int L_WindowPosition (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowPosition (
      my_checkstring (L, 1),    // Name
      (long) my_checknumber (L, 2),    // Left
      (long) my_checknumber (L, 3),    // Top
      (short) my_checknumber (L, 4),   // Position
      (long) my_checknumber (L, 5)     // Flags
    ));

  return 1;  // number of result fields
  } // end of L_WindowPosition

//----------------------------------------
//  world.WindowRectOp
//----------------------------------------
static int L_WindowRectOp (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowRectOp (
            my_checkstring (L, 1),         // Name
            (short) my_checknumber (L, 2), // Action
            (long) my_checknumber (L, 3),  // Left
            (long) my_checknumber (L, 4),  // Top
            (long) my_checknumber (L, 5),  // Right
            (long) my_checknumber (L, 6),  // Bottom
            (long) my_checknumber (L, 7),  // Colour1
            (long) my_optnumber (L, 8, 0)  // Colour2
            ));
  return 1;  // number of result fields
  } // end of L_WindowRectOp


//----------------------------------------
//  world.WindowResize
//----------------------------------------
static int L_WindowResize (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowResize (
      my_checkstring (L, 1),    // Name
      (long) my_checknumber (L, 2),    // Width
      (long) my_checknumber (L, 3),    // Height
      (long) my_checknumber (L, 4)     // Background Colour
    ));

  return 1;  // number of result fields
  } // end of L_WindowResize

//----------------------------------------
//  world.WindowScrollwheelHandler
//----------------------------------------
static int L_WindowScrollwheelHandler (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowScrollwheelHandler (
            my_checkstring (L, 1),    // Name
            my_checkstring (L, 2),    // HotspotId
            my_optstring (L, 3, "")   // MoveCallback
            ));
  return 1;  // number of result fields
  } // end of L_WindowScrollwheelHandler

//----------------------------------------
//  world.WindowSetPixel
//----------------------------------------
static int L_WindowSetPixel (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowSetPixel (
            my_checkstring (L, 1),  // Name
            (long) my_checknumber (L, 2),  // x
            (long) my_checknumber (L, 3),  // y
            (long) my_checknumber (L, 4)   // Colour
            ));
  return 1;  // number of result fields
  } // end of L_WindowSetPixel

//----------------------------------------
//  world.WindowShow
//----------------------------------------
static int L_WindowShow (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L,pDoc->WindowShow (
        my_checkstring (L, 1),    // Name
        optboolean (L, 2, 1)));    // Show
  return 1;  // number of result fields
  } // end of L_WindowShow

//----------------------------------------
//  world.WindowText
//----------------------------------------
static int L_WindowText (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowText (
        my_checkstring (L, 1),    // Name
        my_checkstring (L, 2),    // Font
        my_checkstring (L, 3),    // Text
        (long) my_checknumber (L, 4),    // Left
        (long) my_checknumber (L, 5),    // Top
        (long) my_checknumber (L, 6),    // Right
        (long) my_checknumber (L, 7),    // Bottom
        (long) my_checknumber (L, 8),    // Colour
        optboolean (L, 9, 0)     // Unicode
        ));

  return 1;  // number of result fields
  } // end of L_WindowText

//----------------------------------------
//  world.WindowTextWidth
//----------------------------------------
static int L_WindowTextWidth (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowTextWidth (
        my_checkstring (L, 1),    // Name
        my_checkstring (L, 2),    // Font
        my_checkstring (L, 3),    // Text
        optboolean (L, 4, 0)      // Unicode
        ));

  return 1;  // number of result fields
  } // end of WindowTextWidth

//----------------------------------------
//  world.WindowTransformImage
//----------------------------------------
static int L_WindowTransformImage (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowTransformImage (
            my_checkstring (L, 1),            // Name
            my_checkstring (L, 2),            // ImageId
            (float) my_checknumber (L, 3),    // Left
            (float) my_checknumber (L, 4),    // Top
            (short) my_checknumber (L, 5),    // Mode
            (float) my_checknumber (L, 6),    // Mxx
            (float) my_checknumber (L, 7),    // Mxy
            (float) my_checknumber (L, 8),    // Myx
            (float) my_checknumber (L, 9)     // Myy
            ));
  return 1;  // number of result fields
  } // end of L_WindowTransformImage

//----------------------------------------
//  world.WindowWrite
//----------------------------------------
static int L_WindowWrite (lua_State *L)
  {
  CMUSHclientDoc *pDoc = doc (L);
  lua_pushnumber (L, pDoc->WindowWrite (
      my_checkstring (L, 1),    // Name
      my_checkstring (L, 2)     // FileName
     ));

  return 1;  // number of result fields
  } // end of L_WindowWrite

//----------------------------------------
//  world.WorldAddress
//----------------------------------------
static int L_WorldAddress (lua_State *L)
  {
  BSTR str = doc (L)->WorldAddress ();
  return pushBstr (L, str);  // number of result fields
  } // end of L_WorldAddress


//----------------------------------------
//  world.WorldName
//----------------------------------------
static int L_WorldName (lua_State *L)
  {
  BSTR str = doc (L)->WorldName ();
  return pushBstr (L, str);  // number of result fields
  } // end of L_WorldName


//----------------------------------------
//  world.WorldPort
//----------------------------------------
static int L_WorldPort (lua_State *L)
  {
  lua_pushnumber (L, doc (L)->WorldPort ());
  return 1;  // number of result fields
  } // end of L_WorldPort

//----------------------------------------
//  world.WriteLog
//----------------------------------------
static int L_WriteLog (lua_State *L)
  {
  CMUSHclientDoc * pDoc = doc (L);  // must do this first
  lua_pushnumber (L, pDoc->WriteLog (concatArgs (L).c_str ()));
  return 1;  // number of result fields
  } // end of L_WriteLog

// methods implemented in Lua
// warning - case-sensitive!

// eg. world.Note ("hello ", "world")

static const struct luaL_reg worldlib [] = 
  {

  {"Accelerator", L_Accelerator},
  {"AcceleratorTo", L_AcceleratorTo},
  {"AcceleratorList", L_AcceleratorList},
  {"Activate", L_Activate},
  {"ActivateClient", L_ActivateClient},
  {"ActivateNotepad", L_ActivateNotepad},
  {"AddAlias", L_AddAlias},
  {"AddFont", L_AddFont},
  {"AddMapperComment", L_AddMapperComment},
  {"AddSpellCheckWord", L_AddSpellCheckWord},
  {"AddTimer", L_AddTimer},
  {"AddToMapper", L_AddToMapper},
  {"AddTrigger", L_AddTrigger},
  {"AddTriggerEx", L_AddTriggerEx},
  {"AdjustColour", L_AdjustColour},
  {"ANSI", L_ANSI},
  {"AnsiNote", L_AnsiNote},
  {"AppendToNotepad", L_AppendToNotepad},
  {"ArrayClear", L_ArrayClear},
  {"ArrayCount", L_ArrayCount},
  {"ArrayCreate", L_ArrayCreate},
  {"ArrayDelete", L_ArrayDelete},
  {"ArrayDeleteKey", L_ArrayDeleteKey},
  {"ArrayExists", L_ArrayExists},
  {"ArrayExport", L_ArrayExport},
  {"ArrayExportKeys", L_ArrayExportKeys},
  {"ArrayGet", L_ArrayGet},
  {"ArrayGetFirstKey", L_ArrayGetFirstKey},
  {"ArrayGetLastKey", L_ArrayGetLastKey},
  {"ArrayImport", L_ArrayImport},
  {"ArrayKeyExists", L_ArrayKeyExists},
  {"ArrayList", L_ArrayList},
  {"ArrayListAll", L_ArrayListAll},
  {"ArrayListKeys", L_ArrayListKeys},
  {"ArrayListValues", L_ArrayListValues},
  {"ArraySet", L_ArraySet},
  {"ArraySize", L_ArraySize},
  {"Base64Decode", L_Base64Decode},
  {"Base64Encode", L_Base64Encode},
  {"BlendPixel", L_BlendPixel},
  {"GetBoldColour", L_GetBoldColour},
  {"SetBoldColour", L_SetBoldColour},
  {"BroadcastPlugin", L_BroadcastPlugin},
  {"CallPlugin", L_CallPlugin},
  {"ChangeDir", L_ChangeDir},
  {"ChatAcceptCalls", L_ChatAcceptCalls},
  {"ChatCall", L_ChatCall},
  {"ChatCallzChat", L_ChatCallzChat},
  {"ChatDisconnect", L_ChatDisconnect},
  {"ChatDisconnectAll", L_ChatDisconnectAll},
  {"ChatEverybody", L_ChatEverybody},
  {"ChatGetID", L_ChatGetID},
  {"ChatGroup", L_ChatGroup},
  {"ChatID", L_ChatID},
  {"ChatMessage", L_ChatMessage},
  {"ChatNameChange", L_ChatNameChange},
  {"ChatNote", L_ChatNote},
  {"ChatPasteEverybody", L_ChatPasteEverybody},
  {"ChatPasteText", L_ChatPasteText},
  {"ChatPeekConnections", L_ChatPeekConnections},
  {"ChatPersonal", L_ChatPersonal},
  {"ChatPing", L_ChatPing},
  {"ChatRequestConnections", L_ChatRequestConnections},
  {"ChatSendFile", L_ChatSendFile},
  {"ChatStopAcceptingCalls", L_ChatStopAcceptingCalls},
  {"ChatStopFileTransfer", L_ChatStopFileTransfer},
  {"CloseLog", L_CloseLog},
  {"CloseNotepad", L_CloseNotepad},
  {"ColourNameToRGB", L_ColourNameToRGB},
  {"ColourNote", L_ColourNote},
  {"ColourTell", L_ColourTell},
  {"Connect", L_Connect},
  {"CreateGUID", L_CreateGUID},
  {"SetCustomColourBackground", L_SetCustomColourBackground},
  {"GetCustomColourBackground", L_GetCustomColourBackground},
  {"SetCustomColourText", L_SetCustomColourText},
  {"GetCustomColourText", L_GetCustomColourText},
  {"DatabaseOpen", L_DatabaseOpen},
  {"DatabaseClose", L_DatabaseClose},
  {"DatabasePrepare", L_DatabasePrepare},
  {"DatabaseFinalize", L_DatabaseFinalize},
  {"DatabaseColumns", L_DatabaseColumns},
  {"DatabaseStep", L_DatabaseStep},
  {"DatabaseError", L_DatabaseError},
  {"DatabaseColumnName", L_DatabaseColumnName},
  {"DatabaseColumnText", L_DatabaseColumnText},
  {"DatabaseColumnValue", L_DatabaseColumnValue},
  {"DatabaseColumnType", L_DatabaseColumnType},
  {"DatabaseTotalChanges", L_DatabaseTotalChanges},
  {"DatabaseChanges", L_DatabaseChanges},
  {"DatabaseLastInsertRowid", L_DatabaseLastInsertRowid},
  {"DatabaseList", L_DatabaseList},
  {"DatabaseInfo", L_DatabaseInfo},
  {"DatabaseExec", L_DatabaseExec},
  {"DatabaseColumnNames", L_DatabaseColumnNames},
  {"DatabaseColumnValues", L_DatabaseColumnValues},
  {"DatabaseReset", L_DatabaseReset},
  {"Debug", L_Debug},
  {"DeleteAlias", L_DeleteAlias},
  {"DeleteAliasGroup", L_DeleteAliasGroup},
  {"DeleteAllMapItems", L_DeleteAllMapItems},
  {"DeleteCommandHistory", L_DeleteCommandHistory},
  {"DeleteGroup", L_DeleteGroup},
  {"DeleteLastMapItem", L_DeleteLastMapItem},
  {"DeleteLines", L_DeleteLines},
  {"DeleteOutput", L_DeleteOutput},
  {"DeleteTemporaryAliases", L_DeleteTemporaryAliases},
  {"DeleteTemporaryTimers", L_DeleteTemporaryTimers},
  {"DeleteTemporaryTriggers", L_DeleteTemporaryTriggers},
  {"DeleteTimer", L_DeleteTimer},
  {"DeleteTimerGroup", L_DeleteTimerGroup},
  {"DeleteTrigger", L_DeleteTrigger},
  {"DeleteTriggerGroup", L_DeleteTriggerGroup},
  {"DeleteVariable", L_DeleteVariable},
  {"DiscardQueue", L_DiscardQueue},
  {"Disconnect", L_Disconnect},
  {"DoAfter", L_DoAfter},
  {"DoAfterNote", L_DoAfterNote},
  {"DoAfterSpecial", L_DoAfterSpecial},
  {"DoAfterSpeedWalk", L_DoAfterSpeedWalk},
  {"DoCommand", L_DoCommand},
  {"GetEchoInput", L_GetEchoInput},
  {"SetEchoInput", L_SetEchoInput},
  {"EditDistance", L_EditDistance},
  {"EnableAlias", L_EnableAlias},
  {"EnableAliasGroup", L_EnableAliasGroup},
  {"EnableGroup", L_EnableGroup},
  {"EnableMapping", L_EnableMapping},
  {"EnablePlugin", L_EnablePlugin},
  {"EnableTimer", L_EnableTimer},
  {"EnableTimerGroup", L_EnableTimerGroup},
  {"EnableTrigger", L_EnableTrigger},
  {"EnableTriggerGroup", L_EnableTriggerGroup},
  {"ErrorDesc", L_ErrorDesc},
  {"EvaluateSpeedwalk", L_EvaluateSpeedwalk},
  {"Execute", L_Execute},
  {"ExportXML", L_ExportXML},
  {"FilterPixel", L_FilterPixel},
  {"FixupEscapeSequences", L_FixupEscapeSequences},
  {"FixupHTML", L_FixupHTML},
  {"FlashIcon", L_FlashIcon},
  {"FlushLog", L_FlushLog},
  {"GenerateName", L_GenerateName},
  {"GetAlias", L_GetAlias},
  {"GetAliasInfo", L_GetAliasInfo},
  {"GetAliasList", L_GetAliasList},
  {"GetAliasOption", L_GetAliasOption},
  {"GetAliasWildcard", L_GetAliasWildcard},
  {"GetAlphaOption", L_GetAlphaOption},
  {"GetAlphaOptionList", L_GetAlphaOptionList},
  {"GetChatInfo", L_GetChatInfo},
  {"GetChatList", L_GetChatList},
  {"GetChatOption", L_GetChatOption},
  {"GetClipboard", L_GetClipboard},
  {"GetCommand", L_GetCommand},
  {"GetCommandList", L_GetCommandList},
  {"GetConnectDuration", L_GetConnectDuration},
  {"GetCurrentValue", L_GetCurrentValue},
  {"GetCustomColourName", L_GetCustomColourName},
  {"GetDefaultValue", L_GetDefaultValue},
  {"GetDeviceCaps", L_GetDeviceCaps},
  {"GetEntity", L_GetEntity},
  {"GetXMLEntity", L_GetXMLEntity},
  {"GetFrame", L_GetFrame},
  {"GetGlobalOption", L_GetGlobalOption},
  {"GetGlobalOptionList", L_GetGlobalOptionList},
  {"GetHostAddress", L_GetHostAddress},
  {"GetHostName", L_GetHostName},
  {"GetInfo", L_GetInfo},
  {"GetInternalCommandsList", L_GetInternalCommandsList},
  {"GetLineCount", L_GetLineCount},
  {"GetLineInfo", L_GetLineInfo},
  {"GetLinesInBufferCount", L_GetLinesInBufferCount},
  {"GetLoadedValue", L_GetLoadedValue},
  {"GetMainWindowPosition", L_GetMainWindowPosition},
  {"GetWorldWindowPosition", L_GetWorldWindowPosition},
  {"GetNotepadWindowPosition", L_GetNotepadWindowPosition},
  {"GetMapColour", L_GetMapColour},
  {"GetMappingCount", L_GetMappingCount},
  {"GetMappingItem", L_GetMappingItem},
  {"GetMappingString", L_GetMappingString},
  {"GetNotepadLength", L_GetNotepadLength},
  {"GetNotepadList", L_GetNotepadList},
  {"GetNotepadText", L_GetNotepadText},
  {"GetNotes", L_GetNotes},
  {"GetNoteStyle", L_GetNoteStyle},
  {"GetOption", L_GetOption},
  {"GetOptionList", L_GetOptionList},
  {"GetPluginAliasInfo", L_GetPluginAliasInfo},
  {"GetPluginAliasList", L_GetPluginAliasList},
  {"GetPluginAliasOption", L_GetPluginAliasOption},
  {"GetPluginID", L_GetPluginID},
  {"GetPluginInfo", L_GetPluginInfo},
  {"GetPluginList", L_GetPluginList},
  {"GetPluginName", L_GetPluginName},
  {"GetPluginTimerInfo", L_GetPluginTimerInfo},
  {"GetPluginTimerList", L_GetPluginTimerList},
  {"GetPluginTimerOption", L_GetPluginTimerOption},
  {"GetPluginTriggerInfo", L_GetPluginTriggerInfo},
  {"GetPluginTriggerList", L_GetPluginTriggerList},
  {"GetPluginTriggerOption", L_GetPluginTriggerOption},
  {"GetPluginVariable", L_GetPluginVariable},
  {"GetPluginVariableList", L_GetPluginVariableList},
  {"GetQueue", L_GetQueue},
  {"GetReceivedBytes", L_GetReceivedBytes},
  {"GetRecentLines", L_GetRecentLines},
  {"GetScriptTime", L_GetScriptTime},
  {"GetSelectionEndColumn", L_GetSelectionEndColumn},
  {"GetSelectionEndLine", L_GetSelectionEndLine},
  {"GetSelectionStartColumn", L_GetSelectionStartColumn},
  {"GetSelectionStartLine", L_GetSelectionStartLine},
  {"GetSentBytes", L_GetSentBytes},
  {"GetSoundStatus", L_GetSoundStatus},
  {"GetStyleInfo", L_GetStyleInfo},
  {"GetSysColor", L_GetSysColor},
  {"GetSystemMetrics", L_GetSystemMetrics},
  {"GetTimer", L_GetTimer},
  {"GetTimerInfo", L_GetTimerInfo},
  {"GetTimerList", L_GetTimerList},
  {"GetTimerOption", L_GetTimerOption},
  {"GetTrigger", L_GetTrigger},
  {"GetTriggerInfo", L_GetTriggerInfo},
  {"GetTriggerList", L_GetTriggerList},
  {"GetTriggerOption", L_GetTriggerOption},
  {"GetTriggerWildcard", L_GetTriggerWildcard},
  {"GetUdpPort", L_GetUdpPort},
  {"GetUniqueID", L_GetUniqueID},
  {"GetUniqueNumber", L_GetUniqueNumber},
  {"GetVariable", L_GetVariable},
  {"GetVariableList", L_GetVariableList},
  {"GetWorld", L_GetWorld},
  {"GetWorldById", L_GetWorldById},
  {"GetWorldID", L_GetWorldID},
  {"GetWorldIdList", L_GetWorldIdList},
  {"GetWorldList", L_GetWorldList},
  {"Hash", L_Hash},
  {"Help", L_Help},
  {"Hyperlink", L_Hyperlink},
  {"ImportXML", L_ImportXML},
  {"Info", L_Info},
  {"InfoBackground", L_InfoBackground},
  {"InfoClear", L_InfoClear},
  {"InfoColour", L_InfoColour},
  {"InfoFont", L_InfoFont},
  {"IsAlias", L_IsAlias},
  {"IsConnected", L_IsConnected},
  {"IsLogOpen", L_IsLogOpen},
  {"IsPluginInstalled", L_IsPluginInstalled},
  {"IsTimer", L_IsTimer},
  {"IsTrigger", L_IsTrigger},
  {"LoadPlugin", L_LoadPlugin},
  {"GetLogInput", L_GetLogInput},
  {"SetLogInput", L_SetLogInput},
  {"GetLogNotes", L_GetLogNotes},
  {"SetLogNotes", L_SetLogNotes},
  {"GetLogOutput", L_GetLogOutput},
  {"SetLogOutput", L_SetLogOutput},
  {"LogSend", L_LogSend},
  {"MakeRegularExpression", L_MakeRegularExpression},
  {"MapColour", L_MapColour},
  {"MapColourList", L_MapColourList},
  {"Metaphone", L_Metaphone},
  {"GetMapping", L_GetMapping},
  {"SetMapping", L_SetMapping},
  {"GetNormalColour", L_GetNormalColour},
  {"SetNormalColour", L_SetNormalColour},
  {"MoveMainWindow", L_MoveMainWindow},
  {"MoveWorldWindow", L_MoveWorldWindow},
  {"MoveNotepadWindow", L_MoveNotepadWindow},
  {"MtSrand", L_MtSrand},
  {"MtRand", L_MtRand},
  {"Note", L_Note},
  {"NoteHr", L_NoteHr},
  {"GetNoteColour", L_GetNoteColour},
  {"SetNoteColour", L_SetNoteColour},
  {"GetNoteColourBack", L_GetNoteColourBack},
  {"SetNoteColourBack", L_SetNoteColourBack},
  {"GetNoteColourFore", L_GetNoteColourFore},
  {"SetNoteColourFore", L_SetNoteColourFore},
  {"NoteColourName", L_NoteColourName},
  {"NoteColourRGB", L_NoteColourRGB},
  {"NotepadColour", L_NotepadColour},
  {"NotepadFont", L_NotepadFont},
  {"NotepadReadOnly", L_NotepadReadOnly},
  {"NotepadSaveMethod", L_NotepadSaveMethod},
  {"NoteStyle", L_NoteStyle},
  {"Open", L_Open},
  {"OpenBrowser", L_OpenBrowser},
  {"OpenLog", L_OpenLog},
  {"PasteCommand", L_PasteCommand},
  {"Pause", L_Pause},
  {"PickColour", L_PickColour},
  {"PlaySound", L_PlaySound},
  {"PlaySoundMemory", L_PlaySoundMemory},
  {"PluginSupports", L_PluginSupports},
  {"PushCommand", L_PushCommand},
  {"Queue", L_Queue},
  {"ReadNamesFile", L_ReadNamesFile},
  {"Redraw", L_Redraw},
  {"ReloadPlugin", L_ReloadPlugin},
  {"RemoveBacktracks", L_RemoveBacktracks},
  {"GetRemoveMapReverses", L_GetRemoveMapReverses},
  {"SetRemoveMapReverses", L_SetRemoveMapReverses},
  {"Repaint", L_Repaint},
  {"Replace", L_Replace},
  {"ReplaceNotepad", L_ReplaceNotepad},
  {"Reset", L_Reset},
  {"ResetIP",  L_ResetIP},
  {"ResetStatusTime", L_ResetStatusTime},
  {"ResetTimer", L_ResetTimer},
  {"ResetTimers", L_ResetTimers},
  {"ReverseSpeedwalk", L_ReverseSpeedwalk},
  {"RGBColourToName", L_RGBColourToName},
  {"Save", L_Save},
  {"SaveNotepad", L_SaveNotepad},
  {"SaveState", L_SaveState},
  {"SelectCommand", L_SelectCommand},
  {"Send", L_Send},
  {"SendImmediate", L_SendImmediate},
  {"SendNoEcho", L_SendNoEcho},
  {"SendPkt", L_SendPkt},
  {"SendPush", L_SendPush},
  {"SendSpecial", L_SendSpecial},
  {"SendToNotepad", L_SendToNotepad},
  {"SetAliasOption", L_SetAliasOption},
  {"SetAlphaOption", L_SetAlphaOption},
  {"SetBackgroundColour", L_SetBackgroundColour},
  {"SetBackgroundImage", L_SetBackgroundImage},
  {"SetChatOption", L_SetChatOption},
  {"SetChanged", L_SetChanged},
  {"SetClipboard", L_SetClipboard},
  {"SetCommand", L_SetCommand},
  {"SetCommandSelection", L_SetCommandSelection},
  {"SetCommandWindowHeight", L_SetCommandWindowHeight},
  {"SetCursor", L_SetCursor},
  {"SetCustomColourName", L_SetCustomColourName},
  {"SetEntity", L_SetEntity},
  {"SetForegroundImage", L_SetForegroundImage},
  {"SetInputFont", L_SetInputFont},
  {"SetNotes", L_SetNotes},
  {"SetOption", L_SetOption},
  {"SetOutputFont", L_SetOutputFont},
  {"SetScroll", L_SetScroll},
  {"SetStatus", L_SetStatus},
  {"SetTimerOption", L_SetTimerOption},
  {"SetToolBarPosition", L_SetToolBarPosition},
  {"SetTriggerOption", L_SetTriggerOption},
  {"SetVariable", L_SetVariable}, 
  {"SetWorldWindowStatus", L_SetWorldWindowStatus},
  {"ShiftTabCompleteItem", L_ShiftTabCompleteItem},
  {"ShowInfoBar", L_ShowInfoBar},
  {"Simulate", L_Simulate},
  {"Sound", L_Sound},
  {"GetSpeedWalkDelay", L_GetSpeedWalkDelay},
  {"SetSpeedWalkDelay", L_SetSpeedWalkDelay},
  {"SpellCheck", L_SpellCheck},
  {"SpellCheckCommand", L_SpellCheckCommand},
  {"SpellCheckDlg", L_SpellCheckDlg},
  {"StripANSI", L_StripANSI},
  {"StopSound", L_StopSound},
  {"Tell", L_Tell},
  {"TextRectangle", L_TextRectangle},
  {"GetTrace", L_GetTrace},
  {"SetTrace", L_SetTrace},
  {"TraceOut", L_TraceOut},
  {"TranslateDebug", L_TranslateDebug},
  {"TranslateGerman", L_TranslateGerman},
  {"Transparency", L_Transparency},
  {"Trim", L_Trim},
  {"UdpListen", L_UdpListen},
  {"UdpPortList", L_UdpPortList},
  {"UdpSend", L_UdpSend},
  {"Version", L_Version},
  {"WindowAddHotspot", L_WindowAddHotspot},
  {"WindowArc", L_WindowArc},
  {"WindowBezier", L_WindowBezier},
  {"WindowBlendImage", L_WindowBlendImage},
  {"WindowCircleOp", L_WindowCircleOp},
  {"WindowCreate", L_WindowCreate},
  {"WindowCreateImage", L_WindowCreateImage},
  {"WindowDeleteAllHotspots", L_WindowDeleteAllHotspots},
  {"WindowDelete", L_WindowDelete},
  {"WindowDeleteHotspot", L_WindowDeleteHotspot},
  {"WindowDragHandler", L_WindowDragHandler},
  {"WindowDrawImage", L_WindowDrawImage},
  {"WindowDrawImageAlpha", L_WindowDrawImageAlpha},
  {"WindowFilter", L_WindowFilter},
  {"WindowFont", L_WindowFont},
  {"WindowFontInfo", L_WindowFontInfo},
  {"WindowFontList", L_WindowFontList},
  {"WindowGetImageAlpha", L_WindowGetImageAlpha},
  {"WindowGetPixel", L_WindowGetPixel},
  {"WindowGradient", L_WindowGradient},
  {"WindowHotspotInfo", L_WindowHotspotInfo},
  {"WindowHotspotList", L_WindowHotspotList},
  {"WindowHotspotTooltip", L_WindowHotspotTooltip},
  {"WindowImageFromWindow", L_WindowImageFromWindow},
  {"WindowImageInfo", L_WindowImageInfo},
  {"WindowImageList", L_WindowImageList},
  {"WindowImageOp", L_WindowImageOp},
  {"WindowInfo", L_WindowInfo},
  {"WindowLine", L_WindowLine},
  {"WindowList", L_WindowList},
  {"WindowLoadImage", L_WindowLoadImage},
  {"WindowLoadImageMemory", L_WindowLoadImageMemory},
  {"WindowMenu", L_WindowMenu},
  {"WindowMergeImageAlpha", L_WindowMergeImageAlpha},
  {"WindowMoveHotspot", L_WindowMoveHotspot},
  {"WindowPolygon", L_WindowPolygon},
  {"WindowPosition", L_WindowPosition},
  {"WindowRectOp", L_WindowRectOp},
  {"WindowResize", L_WindowResize},
  {"WindowScrollwheelHandler", L_WindowScrollwheelHandler},
  {"WindowSetPixel", L_WindowSetPixel},
  {"WindowShow", L_WindowShow},
  {"WindowText", L_WindowText},
  {"WindowTextWidth", L_WindowTextWidth},
  {"WindowTransformImage", L_WindowTransformImage},
  {"WindowWrite", L_WindowWrite},
  {"WorldAddress", L_WorldAddress},
  {"WorldName", L_WorldName},
  {"WorldPort", L_WorldPort},
  {"WriteLog", L_WriteLog},
    
  {NULL, NULL}
  };


static int l_print (lua_State *L) 
  {
  CMUSHclientDoc * pDoc = doc (L);  // have to do it first
  pDoc->Note (concatArgs (L, " ").c_str ());
  return 0;
} // end of l_print


int world2string (lua_State *L) 
  {
//  CMUSHclientDoc *pDoc = doc(L);
//  lua_pushfstring(L, "world(%s)", pDoc->m_mush_name);

  lua_pushliteral (L, "world");
  return 1;
  }
  
static const struct luaL_reg worldlib_meta [] = 
  {
    {"__tostring", world2string},
    {NULL, NULL}
  };

static int MakeFlagsTable (lua_State *L, 
                           const char *name,
                           const flags_pair *arr)
{
  const flags_pair *p;
  lua_newtable(L);
  for(p=arr; p->key != NULL; p++) {
    lua_pushstring(L, p->key);
    lua_pushnumber(L, p->val);
    lua_rawset(L, -3);
  }
  lua_setglobal (L, name);
  return 1;
}

// warning - translates messages
static int MakeIntFlagsTable (lua_State *L, 
                             const char *name,
                             const int_flags_pair *arr)
{
  const int_flags_pair *p;
  lua_newtable(L);
  for(p=arr; p->val != NULL; p++) {
    lua_pushnumber(L, p->key);
    lua_pushstring(L, Translate (p->val));
    lua_rawset(L, -3);
  }
  lua_setglobal (L, name);
  return 1;
}

static flags_pair trigger_flags[] =
{

  { "Enabled", 1 },  // enable trigger 
  { "OmitFromLog", 2 },  // omit from log file 
  { "OmitFromOutput", 4 },  // omit trigger from output 
  { "KeepEvaluating", 8 },  // keep evaluating 
  { "IgnoreCase", 16 },  // ignore case when matching 
  { "RegularExpression", 32 },  // trigger uses regular expression 
  { "ExpandVariables", 512 },  // expand variables like @direction 
  { "Replace", 1024 },  // replace existing trigger of same name 
  { "LowercaseWildcard", 2048 },   // wildcards forced to lower-case
  { "Temporary", 16384 },  // temporary - do not save to world file 
  { "OneShot", 32768 },  // if set, trigger only fires once 

  { NULL, 0 }
};

static flags_pair custom_colours[] =
{

  { "NoChange", -1 }, 
  { "Custom1", 0 }, 
  { "Custom2", 1 }, 
  { "Custom3", 2 },  
  { "Custom4", 3 }, 
  { "Custom5", 4 }, 
  { "Custom6", 5 }, 
  { "Custom7", 6 }, 
  { "Custom8", 7 }, 
  { "Custom9", 8 }, 
  { "Custom10", 9 }, 
  { "Custom11", 10 }, 
  { "Custom12", 11 }, 
  { "Custom13", 12 }, 
  { "Custom14", 13 }, 
  { "Custom15", 14 }, 
  { "Custom16", 15 }, 
  { "CustomOther", 16 },   // triggers only

  { NULL, 0 }
};

static flags_pair alias_flags[] =
{

  { "Enabled", 1 },  // same as for AddTrigger 
  { "IgnoreAliasCase", 32 },  // ignore case when matching 
  { "OmitFromLogFile", 64 },  // omit this alias from the log file 
  { "RegularExpression", 128 },  // alias is regular expressions 
  { "ExpandVariables", 512 },   // same as for AddTrigger 
  { "Replace", 1024 },   // same as for AddTrigger 
  { "AliasSpeedWalk", 2048 },  // interpret send string as a speed walk string 
  { "AliasQueue", 4096 },  // queue this alias for sending at the speedwalking delay interval 
  { "AliasMenu", 8192 },  // this alias appears on the alias menu 
  { "Temporary", 16384 },   // same as for AddTrigger 
  { "OneShot", 32768 },   // if set, alias only fires once 

  { NULL, 0 }
};

static flags_pair timer_flags[] =
{

  { "Enabled", 1 },  // same as for AddTrigger 
  { "AtTime", 2 },  // if not set, time is "every" 
  { "OneShot", 4 },  // if set, timer only fires once 
  { "TimerSpeedWalk", 8 },  // timer does a speed walk when it fires 
  { "TimerNote", 16 },  // timer does a world.note when it fires 
  { "ActiveWhenClosed", 32 },   // timer fires even when world is disconnected
  { "Replace", 1024 },  // same as for AddTrigger
  { "Temporary", 16384 },  // same as for AddTrigger

  { NULL, 0 }
};

static flags_pair error_codes[] =
{

  { "eOK", 0 },                             // No error
  { "eWorldOpen", 30001 },                  // The world is already open
  { "eWorldClosed", 30002 },                // The world is closed, this action cannot be performed
  { "eNoNameSpecified", 30003 },            // No name has been specified where one is required
  { "eCannotPlaySound", 30004 },            // The sound file could not be played
  { "eTriggerNotFound", 30005 },            // The specified trigger name does not exist
  { "eTriggerAlreadyExists", 30006 },       // Attempt to add a trigger that already exists
  { "eTriggerCannotBeEmpty", 30007 },       // The trigger "match" string cannot be empty
  { "eInvalidObjectLabel", 30008 },         // The name of this object is invalid
  { "eScriptNameNotLocated", 30009 },       // Script name is not in the script file
  { "eAliasNotFound", 30010 },              // The specified alias name does not exist
  { "eAliasAlreadyExists", 30011 },         // Attempt to add a alias that already exists
  { "eAliasCannotBeEmpty", 30012 },         // The alias "match" string cannot be empty
  { "eCouldNotOpenFile", 30013 },           // Unable to open requested file
  { "eLogFileNotOpen", 30014 },             // Log file was not open
  { "eLogFileAlreadyOpen", 30015 },         // Log file was already open
  { "eLogFileBadWrite", 30016 },            // Bad write to log file
  { "eTimerNotFound", 30017 },              // The specified timer name does not exist
  { "eTimerAlreadyExists", 30018 },         // Attempt to add a timer that already exists
  { "eVariableNotFound", 30019 },           // Attempt to delete a variable that does not exist
  { "eCommandNotEmpty", 30020 },            // Attempt to use SetCommand with a non-empty command window
  { "eBadRegularExpression", 30021 },       // Bad regular expression syntax
  { "eTimeInvalid", 30022 },                // Time given to AddTimer is invalid
  { "eBadMapItem", 30023 },                 // Direction given to AddToMapper is invalid
  { "eNoMapItems", 30024 },                 // No items in mapper
  { "eUnknownOption", 30025 },              // Option name not found
  { "eOptionOutOfRange", 30026 },           // New value for option is out of range
  { "eTriggerSequenceOutOfRange", 30027 },  // Trigger sequence value invalid
  { "eTriggerSendToInvalid", 30028 },       // Where to send trigger text to is invalid
  { "eTriggerLabelNotSpecified", 30029 },   // Trigger label not specified/invalid for 'send to variable'
  { "ePluginFileNotFound", 30030 },         // File name specified for plugin not found
  { "eProblemsLoadingPlugin", 30031 },      // There was a parsing or other problem loading the plugin
  { "ePluginCannotSetOption", 30032 },      // Plugin is not allowed to set this option
  { "ePluginCannotGetOption", 30033 },      // Plugin is not allowed to get this option
  { "eNoSuchPlugin", 30034 },               // Requested plugin is not installed
  { "eNotAPlugin", 30035 },                 // Only a plugin can do this
  { "eNoSuchRoutine", 30036 },              // Plugin does not support that subroutine (subroutine not in script)
  { "ePluginDoesNotSaveState", 30037 },     // Plugin does not support saving state
  { "ePluginCouldNotSaveState", 30037 },    // Plugin could not save state (eg. no state directory)
  { "ePluginDisabled", 30039 },             // Plugin is currently disabled
  { "eErrorCallingPluginRoutine", 30040 },  // Could not call plugin routine
  { "eCommandsNestedTooDeeply", 30041 },    // Calls to "Execute" nested too deeply
  { "eCannotCreateChatSocket", 30042 },     // Unable to create socket for chat connection
  { "eCannotLookupDomainName", 30043 },     // Unable to do DNS (domain name) lookup for chat connection
  { "eNoChatConnections", 30044 },          // No chat connections open
  { "eChatPersonNotFound", 30045 },         // Requested chat person not connected
  { "eBadParameter", 30046 },               // General problem with a parameter to a script call
  { "eChatAlreadyListening", 30047 },       // Already listening for incoming chats
  { "eChatIDNotFound", 30048 },             // Chat session with that ID not found
  { "eChatAlreadyConnected", 30049 },       // Already connected to that server/port
  { "eClipboardEmpty", 30050 },             // Cannot get (text from the) clipboard
  { "eFileNotFound", 30051 },               // Cannot open the specified file
  { "eAlreadyTransferringFile", 30052 },    // Already transferring a file
  { "eNotTransferringFile", 30053 },        // Not transferring a file
  { "eNoSuchCommand", 30054 },              // There is not a command of that name
  { "eArrayAlreadyExists", 30055 },         // Chat session with that ID not found
  { "eArrayDoesNotExist", 30056 },          // Already connected to that server/port
  { "eArrayNotEvenNumberOfValues", 30057 }, // Cannot get (text from the) clipboard
  { "eImportedWithDuplicates", 30058 },     // Cannot open the specified file
  { "eBadDelimiter", 30059 },               // Already transferring a file
  { "eSetReplacingExistingValue", 30060 },  // Not transferring a file
  { "eKeyDoesNotExist", 30061 },            // There is not a command of that name
  { "eCannotImport", 30062 },               // There is not a command of that name
  { "eItemInUse",  30063 },                 // Cannot delete trigger/alias/timer because it is executing a script 
  { "eSpellCheckNotActive", 30064 },        // spell checker is not active 
  { "eCannotAddFont", 30065 },              // cannot create requested font
  { "ePenStyleNotValid", 30066 },           // invalid settings for pen parameter
  { "eUnableToLoadImage", 30067 },          // bitmap image could not be loaded
  { "eImageNotInstalled", 30068 },          // image has not been loaded into window
  { "eInvalidNumberOfPoints", 30069 },      // number of points supplied is incorrect
  { "eInvalidPoint", 30070 },               // point is not numeric
  { "eHotspotPluginChanged", 30071 },       // hotspot processing must all be in same plugin
  { "eHotspotNotInstalled", 30072 },        // hotspot has not been defined for this window
  { "eNoSuchWindow", 30073 },               // requested miniwindow does not exist
  { "eBrushStyleNotValid", 30074 },         // invalid settings for brush parameter


  { NULL, 0 }
};

int_flags_pair error_descriptions[] =
{

  {  0,      Translate_NoOp ("No error") },
  {  30001,  Translate_NoOp ("The world is already open") },
  {  30002,  Translate_NoOp ("The world is closed, this action cannot be performed") },
  {  30003,  Translate_NoOp ("No name has been specified where one is required") },
  {  30004,  Translate_NoOp ("The sound file could not be played") },
  {  30005,  Translate_NoOp ("The specified trigger name does not exist") },
  {  30006,  Translate_NoOp ("Attempt to add a trigger that already exists") },
  {  30007,  Translate_NoOp ("The trigger \"match\" string cannot be empty") },
  {  30008,  Translate_NoOp ("The name of this object is invalid") },
  {  30009,  Translate_NoOp ("Script name is not in the script file") },
  {  30010,  Translate_NoOp ("The specified alias name does not exist") },
  {  30011,  Translate_NoOp ("Attempt to add a alias that already exists") },
  {  30012,  Translate_NoOp ("The alias \"match\" string cannot be empty") },
  {  30013,  Translate_NoOp ("Unable to open requested file") },
  {  30014,  Translate_NoOp ("Log file was not open") },
  {  30015,  Translate_NoOp ("Log file was already open") },
  {  30016,  Translate_NoOp ("Bad write to log file") },
  {  30017,  Translate_NoOp ("The specified timer name does not exist") },
  {  30018,  Translate_NoOp ("Attempt to add a timer that already exists") },
  {  30019,  Translate_NoOp ("Attempt to delete a variable that does not exist") },
  {  30020,  Translate_NoOp ("Attempt to use SetCommand with a non-empty command window") },
  {  30021,  Translate_NoOp ("Bad regular expression syntax") },
  {  30022,  Translate_NoOp ("Time given to AddTimer is invalid") },
  {  30023,  Translate_NoOp ("Direction given to AddToMapper is invalid") },
  {  30024,  Translate_NoOp ("No items in mapper") },
  {  30025,  Translate_NoOp ("Option name not found") },
  {  30026,  Translate_NoOp ("New value for option is out of range") },
  {  30027,  Translate_NoOp ("Trigger sequence value invalid") },
  {  30028,  Translate_NoOp ("Where to send trigger text to is invalid") },
  {  30029,  Translate_NoOp ("Trigger label not specified/invalid for 'send to variable'") },
  {  30030,  Translate_NoOp ("File name specified for plugin not found") },
  {  30031,  Translate_NoOp ("There was a parsing or other problem loading the plugin") },
  {  30032,  Translate_NoOp ("Plugin is not allowed to set this option") },
  {  30033,  Translate_NoOp ("Plugin is not allowed to get this option") },
  {  30034,  Translate_NoOp ("Requested plugin is not installed") },
  {  30035,  Translate_NoOp ("Only a plugin can do this") },
  {  30036,  Translate_NoOp ("Plugin does not support that subroutine (subroutine not in script)") },
  {  30037,  Translate_NoOp ("Plugin does not support saving state") },
  {  30037,  Translate_NoOp ("Plugin could not save state (eg. no state directory)") },
  {  30039,  Translate_NoOp ("Plugin is currently disabled") },
  {  30040,  Translate_NoOp ("Could not call plugin routine") },
  {  30041,  Translate_NoOp ("Calls to \"Execute\" nested too deeply") },
  {  30042,  Translate_NoOp ("Unable to create socket for chat connection") },
  {  30043,  Translate_NoOp ("Unable to do DNS (domain name) lookup for chat connection") },
  {  30044,  Translate_NoOp ("No chat connections open") },
  {  30045,  Translate_NoOp ("Requested chat person not connected") },
  {  30046,  Translate_NoOp ("General problem with a parameter to a script call") },
  {  30047,  Translate_NoOp ("Already listening for incoming chats") },
  {  30048,  Translate_NoOp ("Chat session with that ID not found") },
  {  30049,  Translate_NoOp ("Already connected to that server/port") },
  {  30050,  Translate_NoOp ("Cannot get (text from the) clipboard") },
  {  30051,  Translate_NoOp ("Cannot open the specified file") },
  {  30052,  Translate_NoOp ("Already transferring a file") },
  {  30053,  Translate_NoOp ("Not transferring a file") },
  {  30054,  Translate_NoOp ("There is not a command of that name") },
  {  30055,  Translate_NoOp ("That array already exists") },
  {  30056,  Translate_NoOp ("That array does not exist") },
  {  30057,  Translate_NoOp ("Values to be imported into array are not in pairs") },
  {  30058,  Translate_NoOp ("Import succeeded, however some values were overwritten") },
  {  30059,  Translate_NoOp ("Import/export delimiter must be a single character, other than backslash") },
  {  30060,  Translate_NoOp ("Array element set, existing value overwritten") },
  {  30061,  Translate_NoOp ("Array key does not exist") },
  {  30062,  Translate_NoOp ("Cannot import because cannot find unused temporary character") },
  {  30063,  Translate_NoOp ("Cannot delete trigger/alias/timer because it is executing a script") },
  {  30064,  Translate_NoOp ("Spell checker is not active") },
  {  30065,  Translate_NoOp ("Cannot create requested font") },                     
  {  30066,  Translate_NoOp ("Invalid settings for pen parameter") },               
  {  30067,  Translate_NoOp ("Bitmap image could not be loaded") },                 
  {  30068,  Translate_NoOp ("Image has not been loaded into window") },            
  {  30069,  Translate_NoOp ("Number of points supplied is incorrect") },           
  {  30070,  Translate_NoOp ("Point is not numeric") },                             
  {  30071,  Translate_NoOp ("Hotspot processing must all be in same plugin") },    
  {  30072,  Translate_NoOp ("Hotspot has not been defined for this window") },     
  {  30073,  Translate_NoOp ("Requested miniwindow does not exist") }, 
  {  30074,  Translate_NoOp ("Invalid settings for brush parameter") },
           
  { NULL, 0 }
};

static flags_pair send_to_destinations [] =
{

  {  "world",            eSendToWorld  },
  {  "command",          eSendToCommand  },
  {  "output",           eSendToOutput  },
  {  "status",           eSendToStatus  },
  {  "notepad",          eSendToNotepad  },
  {  "notepadappend",    eAppendToNotepad  },
  {  "logfile",          eSendToLogFile  },
  {  "notepadreplace",   eReplaceNotepad  },
  {  "commandqueue",     eSendToCommandQueue  },
  {  "variable",         eSendToVariable  },
  {  "execute",          eSendToExecute  },
  {  "speedwalk",        eSendToSpeedwalk  },
  {  "script",           eSendToScript  },
  {  "immediate",        eSendImmediate  },
  {  "scriptafteromit",  eSendToScriptAfterOmit  },

  { NULL, 0 }
};


// Constants: miniwin.xxxx

static flags_pair miniwindow_flags [] =
{
               
  // positions for WindowCreate, SetForegroundImage, SetBackgroundImage, WindowPosition
  {  "pos_stretch_to_view",               0 },
  {  "pos_stretch_to_view_with_aspect",   1 },
  {  "pos_stretch_to_owner",              2 },
  {  "pos_stretch_to_owner_with_aspect",  3 },
  {  "pos_top_left",                      4  },
  {  "pos_top_center",                    5  },
  {  "pos_top_right",                     6  },
  {  "pos_center_right",                  7  },
  {  "pos_bottom_right",                  8  },
  {  "pos_bottom_center",                 9  },
  {  "pos_bottom_left",                  10  },
  {  "pos_center_left",                  11  },
  {  "pos_center_all",                   12  },
  {  "pos_tile",                         13  },

  // flags for WindowCreate

  {  "create_underneath",         MINIWINDOW_DRAW_UNDERNEATH  },
  {  "create_absolute_location",  MINIWINDOW_ABSOLUTE_LOCATION  },
  {  "create_transparent",        MINIWINDOW_TRANSPARENT  },
  {  "create_ignore_mouse",       MINIWINDOW_IGNORE_MOUSE  },
  {  "create_keep_hotspots",      MINIWINDOW_KEEP_HOTSPOTS  },


  // pen styles for drawing shapes

  {  "pen_solid",        PS_SOLID  },           // solid pen  (0)
  {  "pen_dash",         PS_DASH  },            // -------    
  {  "pen_dot",          PS_DOT  },             // .......    
  {  "pen_dash_dot",     PS_DASHDOT  },         // _._._._    
  {  "pen_dash_dot_dot", PS_DASHDOTDOT  },      // _.._.._    
  {  "pen_null",         PS_NULL  },            // no pen
  {  "pen_inside_frame", PS_INSIDEFRAME  },     // solid, inside the shape

  // end style ('OR' in one of these)
  {  "pen_endcap_round", PS_ENDCAP_ROUND  },    // rounded ends  (default)
  {  "pen_endcap_square",PS_ENDCAP_SQUARE },    // square ends
  {  "pen_endcap_flat",  PS_ENDCAP_FLAT   },    // flat end (does not extend past end)
  
  // join style ('OR' in one of these)
  {  "pen_join_round",   PS_JOIN_ROUND  },      // rounded join  (default)
  {  "pen_join_bevel",   PS_JOIN_BEVEL  },      // bevelled join
  {  "pen_join_miter",   PS_JOIN_MITER  },      // mitered join

  // brush styles for drawing shapes

  {  "brush_solid",                   0  },            
  {  "brush_null",                    1  },            
  {  "brush_hatch_horizontal",        2  },            
  {  "brush_hatch_vertical",          3  },            
  {  "brush_hatch_forwards_diagonal", 4  },            
  {  "brush_hatch_backwards_diagonal",5  },            
  {  "brush_hatch_cross",             6  },            
  {  "brush_hatch_cross_diagonal",    7  },            
  {  "brush_fine_pattern",            8  },            
  {  "brush_medium_pattern",          9  },            
  {  "brush_coarse_pattern",         10  },            
  {  "brush_waves_horizontal",       11  },            
  {  "brush_waves_vertical",         12  },            


  // WindowRectOp action parameter (argument 2)

  {  "rect_frame",               1  },          
  {  "rect_fill",                2  },          
  {  "rect_invert",              3  },          
  {  "rect_3d_rect",             4  },          
  {  "rect_draw_edge",           5  },          
  {  "rect_flood_fill_border",   6  },          
  {  "rect_flood_fill_surface",  7  },          

  // WindowRectOp colour1 parameter (argument 7) for action 5   (rect_draw_edge)

  {  "rect_edge_raised",  EDGE_RAISED  },          
  {  "rect_edge_etched",  EDGE_ETCHED  },          
  {  "rect_edge_bump",    EDGE_BUMP  },          
  {  "rect_edge_sunken",  EDGE_SUNKEN  },          

  // WindowRectOp colour2 parameter (argument 8) for action 5   (rect_draw_edge)

  {  "rect_edge_at_top_left",     BF_TOPLEFT  },          
  {  "rect_edge_at_top_right",    BF_TOPRIGHT  },          
  {  "rect_edge_at_bottom_left",  BF_BOTTOMLEFT  },          
  {  "rect_edge_at_bottom_right", BF_BOTTOMRIGHT  },          
  {  "rect_edge_at_all",          BF_RECT  },          

  {  "rect_diagonal_end_top_left",      BF_DIAGONAL_ENDTOPLEFT  },          
  {  "rect_diagonal_end_top_right",     BF_DIAGONAL_ENDTOPRIGHT  },          
  {  "rect_diagonal_end_bottom_left",   BF_DIAGONAL_ENDBOTTOMLEFT  },          
  {  "rect_diagonal_end_bottom_right",  BF_DIAGONAL_ENDBOTTOMRIGHT  },          

  // WindowRectOp other colour2 flags (or in) (argument 8) for action 5   (rect_draw_edge)

  {  "rect_option_fill_middle",       BF_MIDDLE  },          
  {  "rect_option_softer_buttons",    BF_SOFT  },          
  {  "rect_option_flat_borders",      BF_FLAT  },          
  {  "rect_option_monochrom_borders", BF_MONO  },          
  


  // WindowCircleOp action parameter (argument 2)

  {  "circle_ellipse",         1  },          
  {  "circle_rectangle",       2  },          
  {  "circle_round_rectangle", 3  },          
  {  "circle_chord",           4  },          
  {  "circle_pie",             5  },          


  //  WindowGradient mode parameter (argument 8)

  {  "gradient_horizontal", 1  },          
  {  "gradient_vertical",   2  },          
  {  "gradient_texture",    3  },          

  // WindowFont charset

  {  "font_charset_ansi",    ANSI_CHARSET  },          
  {  "font_charset_default", DEFAULT_CHARSET  },          
  {  "font_charset_symbol",  SYMBOL_CHARSET  },          

  // WindowFont PitchAndFamily 

  {  "font_family_any",         FF_DONTCARE },          
  {  "font_family_roman",       FF_ROMAN  },          
  {  "font_family_swiss",       FF_SWISS  },          
  {  "font_family_modern",      FF_MODERN  },          
  {  "font_family_script",      FF_SCRIPT  },          
  {  "font_family_decorative",  FF_DECORATIVE  },          


  {  "font_pitch_default",      DEFAULT_PITCH  },          
  {  "font_pitch_fixed",        FIXED_PITCH  },          
  {  "font_pitch_variable",     VARIABLE_PITCH  },          
  {  "font_pitch_monospaced",   MONO_FONT  },          

  {  "font_truetype",           TRUETYPE_FONTTYPE  },          

  // WindowDrawImage mode (argument 7)

  {  "image_copy",              1  },          
  {  "image_stretch",           2  },          
  {  "image_transparent_copy",  3  },          


  // WindowImageOp  actions (argument 2)

  {  "image_fill_ellipse",         1  },          
  {  "image_fill_rectangle",       2  },          
  {  "image_fill_round_fill_rectangle", 3  },          

  // WindowFilter operations (argument 6)

  {  "filter_noise",                     1  },          
  {  "filter_monochrome_noise",          2  },          
  {  "filter_blur",                      3  },          
  {  "filter_sharpen",                   4  },          
  {  "filter_find_edges",                5  },          
  {  "filter_emboss",                    6  },          
  {  "filter_brightness",                7  },          
  {  "filter_contrast",                  8  },          
  {  "filter_gamma",                     9  },          
  {  "filter_red_brightness",           10  },          
  {  "filter_red_contrast",             11  },          
  {  "filter_red_gamma",                12  },          
  {  "filter_green_brightness",         13  },          
  {  "filter_green_contrast",           14  },          
  {  "filter_green_gamma",              15  },          
  {  "filter_blue_brightness",          16  },          
  {  "filter_blue_contrast",            17  },          
  {  "filter_blue_gamma",               18  },          
  {  "filter_grayscale",                19  },          
  {  "filter_normal_grayscale",         20  },          
  {  "filter_brightness_multiply",      21  },          
  {  "filter_red_brightness_multiply",  22  },          
  {  "filter_green_brightness_multiply",23  },          
  {  "filter_blue_brightness_multiply", 24  },          
  {  "filter_lesser_blur",              25 },          
  {  "filter_minor_blur",               26 },          
  {  "filter_average",                  27 },          

  // WindowBlendImage modes (argument 7)

  {  "blend_normal",                                   1 },  
  {  "blend_average",                                  2 },  
  {  "blend_interpolate",                             3 },  
  {  "blend_dissolve",                                 4 },  
  {  "blend_darken",                                   5 },  
  {  "blend_multiply",                                 6 },  
  {  "blend_colour_burn",                              7 },  
  {  "blend_linear_burn",                              8 },  
  {  "blend_inverse_colour_burn",                      9 },  
  {  "blend_subtract",                                10 },  
  {  "blend_lighten",                                 11 },  
  {  "blend_screen",                                  12 },  
  {  "blend_colour_dodge",                            13 },  
  {  "blend_linear_dodge",                            14 },  
  {  "blend_inverse_colour_dodge",                    15 },  
  {  "blend_add",                                     16 },  
  {  "blend_overlay",                                 17 },  
  {  "blend_soft_light",                              18 },  
  {  "blend_hard_light",                              19 },  
  {  "blend_vivid_light",                             20 },  
  {  "blend_linear_light",                            21 },  
  {  "blend_pin_light",                               22 },  
  {  "blend_hard_mix",                                23 },  
  {  "blend_difference",                              24 },  
  {  "blend_exclusion",                               25 },  
  {  "blend_reflect",                                 26 },  
  {  "blend_glow",                                    27 },  
  {  "blend_freeze",                                  28 },  
  {  "blend_heat",                                    29 },  
  {  "blend_negation",                                30 },  
  {  "blend_phoenix",                                 31 },  
  {  "blend_stamp",                                   32 },  
  {  "blend_xor",                                     33 },  
  {  "blend_and",                                     34 },  
  {  "blend_or",                                      35 },  
  {  "blend_red",                                     36 },  
  {  "blend_green",                                   37 },  
  {  "blend_blue",                                    38 },  
  {  "blend_yellow",                                  39 },  
  {  "blend_cyan",                                    40 },  
  {  "blend_magenta",                                 41 },  
  {  "blend_green_limited_by_red",                    42 },  
  {  "blend_green_limited_by_blue",                   43 },  
  {  "blend_green_limited_by_average_of_red_and_blue",44 },  
  {  "blend_blue_limited_by_red",                     45 },  
  {  "blend_blue_limited_by_green",                   46 },  
  {  "blend_blue_limited_by_average_of_red_and_green",47 },  
  {  "blend_red_limited_by_green",                    48 },  
  {  "blend_red_limited_by_blue",                     49 },  
  {  "blend_red_limited_by_average_of_green_and_blue",50 },  
  {  "blend_red_only",                                51 },  
  {  "blend_green_only",                              52 },  
  {  "blend_blue_only",                               53 },  
  {  "blend_discard_red",                             54 },  
  {  "blend_discard_green",                           55 },  
  {  "blend_discard_blue",                            56 },  
  {  "blend_all_red",                                 57 },  
  {  "blend_all_green",                               58 },  
  {  "blend_all_blue",                                59 },  
  {  "blend_hue_mode",                                60 },  
  {  "blend_saturation_mode",                         61 },  
  {  "blend_colour_mode",                             62 },  
  {  "blend_luminance_mode",                          63 },  
  {  "blend_hsl",                                     64 },


  // WindowAddHotspot cursor modes
  
  {  "cursor_none",         -1  },          
  {  "cursor_arrow",        0  },          
  {  "cursor_hand",         1  },          
  {  "cursor_ibeam",        2  },          
  {  "cursor_plus",         3  },          
  {  "cursor_wait",         4  },          
  {  "cursor_up",           5  },          
  {  "cursor_nw_se_arrow",  6  },          
  {  "cursor_ne_sw_arrow",  7  },          
  {  "cursor_ew_arrow",     8  },          
  {  "cursor_ns_arrow",     9  },          
  {  "cursor_both_arrow",   10  },          
  {  "cursor_x",            11  },          
  {  "cursor_help",         12  },          


  // AddHotspot flags (argument 14)

  {  "hotspot_report_all_mouseovers",   0x01  },          

  // hotspot callback flags

  {  "hotspot_got_shift",      0x01  },          
  {  "hotspot_got_control",    0x02  },          
  {  "hotspot_got_alt",        0x04  },          
  {  "hotspot_got_lh_mouse",   0x10  },          
  {  "hotspot_got_rh_mouse",   0x20  },          
  {  "hotspot_got_dbl_click",  0x40  },          
  {  "hotspot_got_not_first",  0x80  },          

  // WindowMergeImageAlpha Mode (argument 8)

  {  "merge_straight",    0  },          
  {  "merge_transparent", 1  },          

  // drag handler callback flags

  {  "drag_got_shift",      0x01  },          
  {  "drag_got_control",    0x02  },          
  {  "drag_got_alt",        0x04  },          

  // mouse wheel handler callback flags

  {  "wheel_got_shift",      0x01  },          
  {  "wheel_got_control",    0x02  },          
  {  "wheel_got_alt",        0x04  },          
  {  "wheel_scroll_back",    0x100  },          


  { NULL, 0 }
};

extern const struct luaL_reg *ptr_xmllib;


// Note stack contains: 1: "mushclient.document"   (DOCUMENT_STATE)
//                      2: pointer to document     (CMUSHclientDoc)

int RegisterLuaRoutines (lua_State *L)
  {

  lua_newtable (L);                    // make a new table
  lua_replace (L, LUA_ENVIRONINDEX);   // global environment is now this empty table

  // this line is doing: global_environment ["mushclient.document"] = world_pointer  (CMUSHclientDoc)
  lua_settable(L, LUA_ENVIRONINDEX);

  // we want the global table so we can put a metatable on it  (ie. _G)
  lua_pushvalue (L, LUA_GLOBALSINDEX);   // for setting the metatable later

  // add the __index metamethod to the world library
  lua_newtable (L);                     // make a new table (the metatable)

  // register the "world" library - leaves world library on stack
  luaL_register (L, WORLD_LIBRARY, worldlib);

  // make our new metatable.__index entry point to the world table
  lua_setfield  (L, -2, "__index");   // metatable __index function       

  // we do this so you can just say "Note 'hello'" rather than
  // world.Note 'hello'

  // set the new table as the metatable for the global library
  lua_setmetatable (L, -2);

  // our print function
  lua_pushcfunction (L, l_print);
  lua_setglobal(L, "print");
        
  // see manual, 28.2, to see why we do this bit:

  // we make a metatable for the world in case they do a world.GetWorld ("name")

  luaL_newmetatable (L, mushclient_typename);  // ie. "mushclient.world"
  luaL_register (L, NULL, worldlib_meta);      // gives us a __tostring function

  // Note: this index is not for the main world document, but for getting a metadata
  // world. That is something like:  w = GetWorld ("myworld"); w:Note ("hello")
  //  -- in this case the __index entry makes Note available inside w
  lua_getglobal (L, WORLD_LIBRARY);         // ie the "world" table
  lua_setfield  (L, -2, "__index");   // metatable __index function       

  lua_settop(L, 0);             // pop everything

  // various constants for scripting (eg. AddTrigger, AddAlias)
  MakeFlagsTable (L, "trigger_flag", trigger_flags);
  MakeFlagsTable (L, "alias_flag", alias_flags);
  MakeFlagsTable (L, "timer_flag", timer_flags);
  // custom colour names
  MakeFlagsTable (L, "custom_colour", custom_colours);
  // return codes - map name to number
  MakeFlagsTable (L, "error_code", error_codes);
  // return codes - map number to description
  MakeIntFlagsTable (L, "error_desc", error_descriptions);
  // send-to table
  MakeFlagsTable (L, "sendto", send_to_destinations);
  // miniwindow constants table
  MakeFlagsTable (L, "miniwin", miniwindow_flags);

  // colour names

  lua_newtable(L);

  for (POSITION pos = App.m_ColoursMap.GetStartPosition(); pos; )
    {
    CColours * pColour;
    CString strColourName;
    App.m_ColoursMap.GetNextAssoc (pos, strColourName, pColour);
    lua_pushstring(L, strColourName);
    lua_pushnumber(L, pColour->iColour);
    lua_rawset(L, -3);
    }

  lua_setglobal (L, "colour_names");

  // extended colour values

  lua_newtable(L);

  int i;
  for (i = 0; i < 256; i++)
    {
    lua_pushnumber(L, xterm_256_colours [i]);
    lua_rawseti (L, -2, i);
    }

  lua_setglobal (L, "extended_colours");

  // add xml reader to utils lib
  luaL_register (L, "utils", ptr_xmllib);

  return 0;
  }


CString GetFromStack (lua_State *L, const char * what)
  {
  CString s;

  lua_pushstring (L, what);  
  lua_rawget    (L, -2);               // get it

  lua_getglobal(L, "tostring");
  lua_pushvalue (L, -2);    // make copy of result
  lua_call (L, 1, 1);       // convert to string

  s = lua_tostring (L, -1);
  lua_pop (L, 2);   // pop result and tostring function

  return s;
  } // end of GetFromStack

static int our_lua_debug_colournote (lua_State *L)
  {
  // just get what they were trying to say
  const char * text = luaL_checkstring (L, 3);

  CString sText (text);
  sText.Replace ("\n", ENDLINE);
  sText += ENDLINE;

  App.AppendToTheNotepad ("Lua Debugging",
                          sText,
                          false,  // don't replace selection
                          eNotepadPacketDebug);

  // make sure they see it
  App.ActivateNotepad ("Lua Debugging");

  return 0;
  }

static int our_lua_debug_function (lua_State *L)
  {

  CDebugLuaDlg dlg;

  dlg.L = L;    // Lua state for doing commands

  lua_getglobal (L, LUA_DBLIBNAME);  /* "debug" */

  if (!lua_istable (L, -1))
     luaL_error(L, "Cannot debug, no debug table");	

  // make our own ColourNote in case they are not attached to a world

  lua_getglobal (L, "ColourNote");  

  if (!lua_isfunction (L, -1))
    {
    lua_pushcfunction(L, our_lua_debug_colournote);
    lua_setglobal    (L, "ColourNote");    // set ColourNote to our function   
    }

  lua_pop (L, 1);   // pop original ColourNote from stack

  // get a heap of info
  lua_pushstring(L, "getinfo");  
  lua_rawget    (L, -2);               // get getinfo function
  
  if (lua_isfunction (L, -1))
    {
    lua_pushnumber (L, 2);    // caller
    lua_pushstring (L, "nfSlu");  // everything except table of lines
    lua_call (L, 2, 1);   // 2 args, expect table as result

    // on stack now should be table of results

    if (lua_istable (L, -1))
      {
      // source of function
      dlg.m_strSource = GetFromStack (L, "short_src");

      // first line defined
      CString strFirstLine = GetFromStack (L, "linedefined");

      // last line defined
      CString strLastLine = GetFromStack (L, "lastlinedefined");
      dlg.m_strLines.Format ("%s, %s", (LPCTSTR) strFirstLine, (LPCTSTR) strLastLine);

      // current line executing
      dlg.m_strCurrentLine = GetFromStack (L, "currentline");

      // what function is: Lua, C, main
      dlg.m_strWhat = GetFromStack (L, "what");

      // what the "name" field is (might be empty)
      CString strNameWhat = GetFromStack (L, "namewhat");

      // if empty, there is no name field, otherwise get it
      if (!strNameWhat.IsEmpty ())
        {
        // name of function
        dlg.m_strFunctionName = GetFromStack (L, "name");
        dlg.m_strFunctionName += " (";
        dlg.m_strFunctionName += strNameWhat;
        dlg.m_strFunctionName += ")";
        }

      // number of upvalues
      dlg.m_strNups = GetFromStack (L, "nups");

      // function address
      dlg.m_strFunction = GetFromStack (L, "func");
      
      } // end of table returned

    } // we have getinfo function

  lua_settop (L, 0);  // discard any results now


  // preload with their last command

  dlg.m_strCommand = App.m_strLastDebugCommand;

  dlg.DoModal ();

  // put last command back, unless they blanked it out

  if (!dlg.m_strCommand.IsEmpty ())
    App.m_strLastDebugCommand = dlg.m_strCommand;

  if (dlg.m_bAbort)
      luaL_error(L, "Script aborted");	

  return 0;
  } // end of our_lua_debug_function

static int our_exit_function (lua_State *L)
  {
  return luaL_error(L, LUA_QL("os.exit") " not implemented in MUSHclient");	
  }  // end of our_exit_function

static int our_popen_function (lua_State *L)
  {
  return luaL_error(L, LUA_QL("io.popen") " not implemented in MUSHclient");	
  } // end of our_popen_function

int DisableDLLs (lua_State * L)
  {
  if (!App.m_bEnablePackageLibrary)
    {
    // grab package library
    lua_getglobal (L, LUA_LOADLIBNAME);  // package table

    // remove package.loadlib
    lua_pushnil     (L);
    lua_setfield    (L, -2, "loadlib");   // package.loadlib = nil       
    

    // grab package.loaders table
    lua_getfield (L, -1, "loaders");  // get package.loaders table
    if (!lua_istable(L, -1))
      luaL_error(L, LUA_QL("package.loaders") " must be a table");

    // remove index 4 - all-in-one loader
    lua_pushnil     (L);
    lua_rawseti     (L, -2, 4);                  //  package.loaders [4] = nil   
  
    // remove index 3 - DLL loader
    lua_pushnil     (L);
    lua_rawseti     (L, -2, 3);                  //  package.loaders [3] = nil   

    lua_settop(L, 0);   // clear stack

    }

  // change debug.debug to ours

  lua_getglobal (L, LUA_DBLIBNAME);  // package table

  if (lua_istable(L, -1))
    {
    lua_pushcfunction(L, our_lua_debug_function);
    lua_setfield    (L, -2, "debug");   // set debug.debug to our function       
    }  // debug library was there
  lua_pop(L, 1);  // pop debug table or whatever

  // get rid of os.exit

  lua_getglobal (L, LUA_OSLIBNAME);  // os table

  if (lua_istable(L, -1))
    {
    lua_pushcfunction(L, our_exit_function);
    lua_setfield    (L, -2, "exit");   // set os.exit to our function       
    }  // os library was there
  lua_pop(L, 1);  // pop os table or whatever

  // get rid of io.popen 

  lua_getglobal (L, LUA_IOLIBNAME);  // io table

  if (lua_istable(L, -1))
    {
    lua_pushcfunction(L, our_popen_function);
    lua_setfield    (L, -2, "popen");   // set io.poen to our function       
    }  // io library was there
  lua_pop(L, 1);  // pop io table  or whatever

  lua_settop(L, 0);   // clear stack
  return 0;
  } // end of DisableDLLs

