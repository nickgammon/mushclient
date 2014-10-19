// Various Lua utilities:

// Implements:

//   utils.activatenotepad
//   utils.appendtonotepad
//   utils.callbackslist
//   utils.choose
//   utils.directorypicker
//   utils.edit_distance
//   utils.editbox
//   utils.filepicker
//   utils.filterpicker
//   utils.fontpicker
//   utils.functionargs
//   utils.functionlist
//   utils.getfontfamilies
//   utils.glyph_available
//   utils.info
//   utils.infotypes
//   utils.inputbox
//   utils.listbox
//   utils.metaphone
//   utils.msgbox
//   utils.multilistbox
//   utils.reload_global_prefs
//   utils.sendtofront
//   utils.shellexecute
//   utils.showdebugstatus
//   utils.spellcheckdialog
//   utils.timer
//   utils.umsgbox
//   utils.utf8decode
//   utils.utf8encode
//   utils.utf8convert
//   utils.utf8sub
//   utils.utf8valid
//   utils.xmlread

// Also see: lua_compress.c  for some more utils.xxx functions

#include "stdafx.h"
#include "..\mainfrm.h"
#include "..\MUSHclient.h"
#include "..\doc.h"
#include "..\dialogs\LuaInputBox.h"
#include "..\dialogs\LuaInputEditDlg.h"
#include "..\dialogs\SpellCheckDlg.h"
#include "..\dialogs\LuaChooseBox.h"
#include "..\dialogs\LuaChooseList.h"
#include "..\dialogs\LuaChooseListMulti.h"
#include "..\dialogs\FunctionListDlg.h"
#include "..\dmetaph.h"
#include <direct.h>
#include <wingdi.h>

extern char working_dir [_MAX_PATH];
extern tInfoTypeMapping InfoTypes [];

// recursive node outputter
static void xmlOuputNode (lua_State *L, CXMLelement & node) 
  {
  POSITION pos;

  lua_newtable(L);    // table of this node

  MakeTableItem (L, "name", node.strName);    // node name, eg. <trigger>
  MakeTableItem (L, "content", node.strContent);  // content (inside element)
  if (node.bEmpty)
    MakeTableItemBool (L, "empty", node.bEmpty);  // empty, eg. <br/>
  MakeTableItem (L, "line", node.iLine);      // line it appeared on

  if (!node.AttributeMap.IsEmpty ())
    {
    CString strName;
    CAttribute * attribute;

    lua_pushliteral (L, "attributes");
    lua_newtable(L);    // table of attributes

    // attributes
    for (pos = node.AttributeMap.GetStartPosition(); pos; )
      {
      node.AttributeMap.GetNextAssoc (pos, strName, attribute);
      MakeTableItem (L, strName, attribute->strValue);    // attribute name -> value
      }      // end of checking each attribute

    lua_rawset(L, -3);              // bind attributes table to "attributes" entry
    }   // end of having attributes

  if (!node.ChildrenList.IsEmpty ())
    {
    lua_pushliteral (L, "nodes");
    lua_newtable(L);    // table of attributes

    int item = 1;

    // children
    for (pos = node.ChildrenList.GetHeadPosition (); pos; item++)
      {
      CXMLelement * pElement = node.ChildrenList.GetNext (pos);

      xmlOuputNode (L, *pElement);
      lua_rawseti (L, -2, item);   // number each node in sequence
      } // end of checking each tag


    lua_rawset(L, -3);              // bind children table to "children" entry
    } // end of having children

  }

static int xmlread (lua_State *L) 
  {

// get xml text
size_t xmlLength;
const char * xml = luaL_checklstring (L, 1, &xmlLength);

CXMLparser parser;

  try
    {

    CMemFile f ((unsigned char *) xml, xmlLength);
    CArchive ar (&f, CArchive::load);

    parser.BuildStructure (ar.GetFile ());

    } // end of try block
  catch (CException* e) 
    {
    char sMessage [1000];
    e->GetErrorMessage (sMessage, sizeof sMessage);
    e->Delete ();
    lua_pushnil (L);     // nil indicates error
    lua_pushstring (L, sMessage); // nature of error
    lua_pushnumber (L, parser.m_xmlLine);  // which line it occurred on
    return 3;
    }

  int iResults = 2;

  // output root node
  xmlOuputNode (L, parser.m_xmlRoot);

  lua_pushstring (L, parser.m_strDocumentName); // root document name

  if (!parser.m_CustomEntityMap.IsEmpty ())
    {
    lua_newtable(L);    // table of custom entities
    iResults++;

    CString strName, strValue;

    // entities
    for (POSITION pos = parser.m_CustomEntityMap.GetStartPosition(); pos; )
      {
      parser.m_CustomEntityMap.GetNextAssoc (pos, strName, strValue);
      MakeTableItem (L, strName, strValue);    // entity name -> value
      }      // end of doing each entity

    }

  return iResults; // root node, plus document name, and maybe entities table
  } // end of  xmlread


// message-box display routine
// arg1 = message to display
// arg2 = title of box - if nil, defaults to "MUSHclient"
// arg3 = type of box: ok, abortretryignore, okcancel, retrycancel, yesno, yesnocancel
// arg4 = type of icon: ?, !, i, .
// arg5 = default button (1 - 3)
//
// returns: yes, no, ok, retry, ignore, cancel, abort, other 

static int generic_msgbox (lua_State *L, const bool bUnicode) 
  {
  const char * boxmsg   = luaL_checkstring (L, 1);
  const char * boxtitle = luaL_optstring (L, 2, "MUSHclient");
  const char * boxtype  = luaL_optstring (L, 3, "ok");
  const char * boxicon  = luaL_optstring (L, 4, "!");
  const int boxdefault  = luaL_optnumber (L, 5, 1);

  if (strlen (boxmsg) > 1000)
     luaL_error (L, "msgbox message too long (max 1000 characters)");

  if (strlen (boxtitle) > 100)
     luaL_error (L, "msgbox title too long (max 100 characters)");

  int msgboxtype = MB_APPLMODAL;  

  // type of box
  if (strcmp (boxtype, "ok") == 0)
    msgboxtype = MB_OK;
  else if (strcmp (boxtype, "abortretryignore") == 0)
    msgboxtype = MB_ABORTRETRYIGNORE;
  else if (strcmp (boxtype, "okcancel") == 0)
    msgboxtype = MB_OKCANCEL;
  else if (strcmp (boxtype, "retrycancel") == 0)
    msgboxtype = MB_RETRYCANCEL;
  else if (strcmp (boxtype, "yesno") == 0)
    msgboxtype = MB_YESNO;
  else if (strcmp (boxtype, "yesnocancel") == 0)
    msgboxtype = MB_YESNOCANCEL;
  else
    luaL_error (L, "msgbox type unknown");

  // icon

  switch (tolower (boxicon [0]))
    {
    case '?' : msgboxtype |= MB_ICONQUESTION;     break;
    case '!' : msgboxtype |= MB_ICONEXCLAMATION;  break;
    case 'i' : msgboxtype |= MB_ICONINFORMATION;  break;
    case '.' : msgboxtype |= MB_ICONSTOP;         break;
    default  : luaL_error (L, "msgbox icon unknown");
   // default is ?
    } // end of switch on icon

  // default button

  switch (boxdefault)
    {
    case 1:    msgboxtype |= MB_DEFBUTTON1; break;
    case 2:    msgboxtype |= MB_DEFBUTTON2; break;
    case 3:    msgboxtype |= MB_DEFBUTTON3; break;
    default:   luaL_error (L, "msgbox default button must be 1, 2 or 3");
    }

  int result;
  
  if (bUnicode)
    result = ::UMessageBox (boxmsg, msgboxtype, boxtitle);
  else
    result = ::MessageBox (Frame, boxmsg, boxtitle, msgboxtype);

  char * p;

  switch (result)
    {
    case IDYES:     p = "yes";    break;
    case IDNO:      p = "no";     break;
    case IDOK:      p = "ok";     break;
    case IDRETRY:   p = "retry";  break;
    case IDIGNORE:  p = "ignore"; break;
    case IDCANCEL:  p = "cancel"; break;
    case IDABORT:   p = "abort";  break;
    default:        p = "other";  break;
    } // end of switch

  lua_pushstring (L, p);

  return 1;
} // end of generic_msgbox


static int msgbox (lua_State *L) 
  {
  return generic_msgbox (L, false);  // not Unicode
  }   // end of msgbox

static int umsgbox (lua_State *L) 
  {
  return generic_msgbox (L, true);  // Unicode
  }  // end of umsgbox


static int get_extra_integer (lua_State *L, const int narg, const char * name)
  {
  int iResult;

  lua_getfield (L, narg, name);
  iResult = luaL_optinteger (L, -1, 0);
  lua_pop (L, 1);

  return iResult;
  }  // end of get_extra_integer

static bool get_extra_boolean (lua_State *L, const int narg, const char * name)
  {
  bool bResult;

  lua_getfield (L, narg, name);
  bResult = optboolean (L, -1, 0);
  lua_pop (L, 1);

  return bResult;
  }  // end of get_extra_boolean

static CString get_extra_string (lua_State *L, const int narg, const char * name)
  {
  CString strResult;

  lua_getfield (L, narg, name);
  strResult = luaL_optstring (L, -1, "");
  lua_pop (L, 1);

  return strResult;
  }  // end of get_extra_string

// input-box display routine
// arg1 = message to display
// arg2 = title of box - if nil, defaults to "MUSHclient"
// arg3 = default text
// arg4 = input font
// arg5 = input font size
// arg6 = table of extra stuff:
//          box_width           --> width of message box in pixels (min 180)
//          box_height          --> height of message box in pixels (min 125)
//          prompt_width        --> width of prompt text in pixels (min 10)
//          prompt_height       --> height of prompt text in pixels (min 12)
//          reply_width         --> width of reply in pixels (min 10)
//          reply_height        --> height of reply in pixels (min 10)
//          max_length          --> max characters they can type (min 1)
//          validate            --> validation function
//          ok_button           --> what to put on the OK button (default: OK)
//          cancel_button       --> what to put on the Cancel button (default: Cancel)
//          read_only           --> text cannot be changed
//          ok_button_width     --> width of OK button in pixels
//          cancel_button_width --> width of Cancel button in pixels
//          no_default          --> there is no default button

//
// returns: what they typed, or nil if cancelled

template <typename T>
static int gen_inputbox (lua_State *L, T & msg) 
  {
  const char * inputmsg     = luaL_checkstring (L, 1);
  const char * inputtitle   = luaL_optstring (L, 2, "MUSHclient");
  const char * inputdefault = luaL_optstring (L, 3, "");
  const char * inputfont    = luaL_optstring (L, 4, "");
  const int    inputsize    = luaL_optnumber (L, 5, 9);
  const int    nExtraStuffArg = 6;  // arg 6 is extra stuff

  // zero means take dialog default
  int iBoxWidth     = 0;
  int iBoxHeight    = 0;
  int iPromptWidth  = 0;
  int iPromptHeight = 0;
  int iReplyWidth   = 0;
  int iReplyHeight  = 0;
  int iMaxReplyLength = 0;
  bool bReadOnly = false;
  bool bNoDefault = false;
  CString strOKbuttonLabel;
  CString strCancelbuttonLabel;
  int iOKbuttonWidth = 0;
  int iCancelbuttonWidth = 0;


  // if arg6 present, and a table, grab extra stuff
  if (lua_istable (L, nExtraStuffArg))
    {
    iBoxWidth             = get_extra_integer (L, nExtraStuffArg, "box_width");
    iBoxHeight            = get_extra_integer (L, nExtraStuffArg, "box_height");
    iPromptWidth          = get_extra_integer (L, nExtraStuffArg, "prompt_width");
    iPromptHeight         = get_extra_integer (L, nExtraStuffArg, "prompt_height");
    iReplyWidth           = get_extra_integer (L, nExtraStuffArg, "reply_width");
    iReplyHeight          = get_extra_integer (L, nExtraStuffArg, "reply_height");
    iMaxReplyLength       = get_extra_integer (L, nExtraStuffArg, "max_length");
    bReadOnly             = get_extra_boolean (L, nExtraStuffArg, "read_only");
    strOKbuttonLabel      = get_extra_string  (L, nExtraStuffArg, "ok_button");
    strCancelbuttonLabel  = get_extra_string  (L, nExtraStuffArg, "cancel_button");
    iOKbuttonWidth        = get_extra_integer (L, nExtraStuffArg, "ok_button_width");
    iCancelbuttonWidth    = get_extra_integer (L, nExtraStuffArg, "cancel_button_width");
    bNoDefault            = get_extra_boolean (L, nExtraStuffArg, "no_default");


    // see if validation function there - do this last so we can leave it on the stack
    lua_getfield (L, nExtraStuffArg, "validate");

    if (!lua_isnil (L, -1))
      {
      if (!lua_isfunction (L, -1))
        luaL_error (L, "inputbox argument #6 value for 'validate' must be a function");

      lua_pushvalue (L, -1);    // function is now on top of stack   
      msg.m_L = L;             // non-NULL indicates we have function there

      // we can't leave the function on the stack, that gets cleared from time to time
      // while the dialog box is running - so we store it in the registry and get the
      // unique index back
      msg.m_iValidationIndex = luaL_ref (L, LUA_REGISTRYINDEX);
      } // validate function there

    }  // table of extra stuff there
 
  if (strlen (inputmsg) > 1000)
     luaL_error (L, "inputbox message too long (max 1000 characters)");

  // if we leave in & it will make the next letter underlined
  string sInputMsg = FindAndReplace (inputmsg, "&", "&&");

  if (strlen (inputtitle) > 100)
     luaL_error (L, "inputbox title too long (max 100 characters)");

  msg.m_strMessage  = sInputMsg.c_str ();
  msg.m_strTitle    = inputtitle;
  msg.m_strReply    = inputdefault;
  msg.m_strFont     = inputfont;
  msg.m_iFontSize   = inputsize;

  // extra stuff from table which is argument 6 (if present)
  msg.m_iBoxWidth             = iBoxWidth    ;
  msg.m_iBoxHeight            = iBoxHeight   ;
  msg.m_iPromptWidth          = iPromptWidth ;
  msg.m_iPromptHeight         = iPromptHeight;
  msg.m_iReplyWidth           = iReplyWidth  ;
  msg.m_iReplyHeight          = iReplyHeight ;
  msg.m_iMaxReplyLength       = iMaxReplyLength;
  msg.m_bReadOnly             = bReadOnly;
  msg.m_strOKbuttonLabel      = strOKbuttonLabel ;
  msg.m_strCancelbuttonLabel  = strCancelbuttonLabel;
  msg.m_iOKbuttonWidth        = iOKbuttonWidth ;
  msg.m_iCancelbuttonWidth    = iCancelbuttonWidth;
  msg.m_bNoDefault            = bNoDefault;

  lua_settop (L, 0);

  if (msg.DoModal () != IDOK)
    lua_pushnil (L);
  else
    lua_pushstring (L, msg.m_strReply);

  // free up registry entry  (is OK with LUA_NOREF)
  luaL_unref(L, LUA_REGISTRYINDEX, msg.m_iValidationIndex);

  return 1;
} // end of gen_inputbox

static int inputbox (lua_State *L) 
  {
  CLuaInputBox msg;

  return gen_inputbox (L, msg);
  }  // end of inputbox

static int editbox (lua_State *L) 
  {
  CLuaInputEditDlg msg;

  return gen_inputbox (L, msg);
  }  // end of editbox

// combo-box choose routine
// arg1 = message to display
// arg2 = title of box - if nil, defaults to "MUSHclient"
// arg3 = table of key/value pairs (value is displayed)
// arg4 = default *key*
//
// returns: chosen key, or nil if: cancelled, or none selected

template <typename T>
static int gen_choose (lua_State *L, T & msg) 
  {
  const char * choosemsg     = luaL_checkstring (L, 1);
  const char * choosetitle   = luaL_optstring (L, 2, "MUSHclient");
  // arg3 is table
  // arg4 is default

  bool bDefaultIsNumber = lua_type (L, 4) == LUA_TNUMBER;
  bool bHaveDefault = lua_gettop (L) >= 4 && !lua_isnil (L, 4);

  string defaultstring;
  lua_Number defaultnumber = 0;

  if (bHaveDefault)
    {
    if (!lua_isstring (L, 4))
      luaL_error (L, "default key must be string or number");
    if (bDefaultIsNumber)
      defaultnumber = lua_tonumber (L, 4);
    else
      defaultstring = lua_tostring (L, 4);
    }

  // if we leave in & it will make the next letter underlined
  string sChooseMsg = FindAndReplace (choosemsg, "&", "&&");

  if (sChooseMsg.length () > 1000)
     luaL_error (L, "message too long (max 1000 characters)");

  if (strlen (choosetitle) > 100)
     luaL_error (L, "title too long (max 100 characters)");

  const int table = 3;

  if (!lua_istable (L, table))
     luaL_error (L, "must have table of choices as 3rd argument");

  msg.m_strMessage  = sChooseMsg.c_str ();
  msg.m_strTitle    = choosetitle;

  // standard Lua table iteration
  for (lua_pushnil (L); lua_next (L, table) != 0; lua_pop (L, 1))
    {
    if (!lua_isstring (L, -2))
      luaL_error (L, "table must have string or number keys");

    if (!lua_isstring (L, -1))
      luaL_error (L, "table must have string or number values");

    // value can simply be converted to a string
    string sValue = lua_tostring (L, -1);

    // get key
    CKeyValuePair kv (sValue);
    
    if (lua_type (L, -2) == LUA_TSTRING)
      {
      kv.sKey_ = lua_tostring (L, -2);
      // remember default position
      if (bHaveDefault && 
          !bDefaultIsNumber && 
          kv.sKey_ == defaultstring)
          msg.m_iDefault = msg.m_data.size ();
      }
    else
      {     // not string, should be number :)
      // cannot do lua_tostring because that confuses lua_next
      kv.bNumber_ = true;
      kv.iKey_ = lua_tonumber (L, -2);
      // remember default position
      if (bHaveDefault && 
          bDefaultIsNumber && 
          kv.iKey_ == defaultnumber)
          msg.m_iDefault = msg.m_data.size ();
      }   // end of key being a number


    msg.m_data.push_back (kv);

    } // end of looping through table


  if (msg.DoModal () != IDOK)
    lua_pushnil (L);
  else
    {  // not cancelled
    if (msg.m_iResult == CB_ERR)
      lua_pushnil (L);   // no choice made
    else
      {    // get key of choice
      if (msg.m_data [msg.m_iResult].bNumber_)
        lua_pushnumber (L, msg.m_data [msg.m_iResult].iKey_);
      else
        lua_pushstring (L, msg.m_data [msg.m_iResult].sKey_.c_str ()); 
      }
    }
  
  return 1;   // 1 result

} // end of gen_choose

static int choose (lua_State *L) 
  {
  CLuaChooseBox msg;
  return gen_choose (L, msg);   // templated function above
  }  // end of choose

static int listbox (lua_State *L) 
  {
  CLuaChooseList msg;
  return gen_choose (L, msg);   // templated function above
  }  // end of listbox


// combo-box choose routine
// arg1 = message to display
// arg2 = title of box - if nil, defaults to "MUSHclient"
// arg3 = table of key/value pairs (value is displayed)
// arg4 = default *key*
//
// returns: chosen key, or nil if: cancelled, or none selected

static int multilistbox (lua_State *L) 
  {
  CLuaChooseListMulti msg;

  const char * choosemsg     = luaL_checkstring (L, 1);
  const char * choosetitle   = luaL_optstring (L, 2, "MUSHclient");
  // arg3 is table
  // arg4 is table of defaults

  bool bHaveDefaults = lua_gettop (L) >= 4 && !lua_isnil (L, 4);

  if (bHaveDefaults)
    {
    if (!lua_istable (L, 4))
      luaL_error (L, "defaults must be a table, or nil");
    }

  // if we leave in & it will make the next letter underlined
  string sChooseMsg = FindAndReplace (choosemsg, "&", "&&");

  if (sChooseMsg.length () > 1000)
     luaL_error (L, "message too long (max 1000 characters)");

  if (strlen (choosetitle) > 100)
     luaL_error (L, "title too long (max 100 characters)");

  if (!lua_istable (L, 3))
     luaL_error (L, "must have table of choices as 3rd argument");

  msg.m_strMessage  = sChooseMsg.c_str ();
  msg.m_strTitle    = choosetitle;

  const int table = 3;

  // standard Lua table iteration
  for (lua_pushnil (L); lua_next (L, table) != 0; lua_pop (L, 1))
    {
    if (!lua_isstring (L, -2))
      luaL_error (L, "table must have string or number keys");

    if (!lua_isstring (L, -1))
      luaL_error (L, "table must have string or number values");

    // value can simply be converted to a string
    string sValue = lua_tostring (L, -1);

    // get key
    CKeyValuePair kv (sValue);
    
    if (lua_type (L, -2) == LUA_TSTRING)
      {
      kv.sKey_ = lua_tostring (L, -2);
      }   // end string key
    else
      {     // not string, should be number :)
      kv.bNumber_ = true;
      kv.iKey_ = lua_tonumber (L, -2);
      }   // end of key being a number

    // remember default positions
    if (bHaveDefaults)
      {
      lua_pushvalue (L, -2);   // copy key to top of stack
      lua_gettable (L, 4);  // see if this key is in defaults table
      if (!(lua_isnil (L, -1) ||
            lua_isboolean (L, -1) &&  !lua_toboolean (L, -1))) 
        msg.m_iDefaults.insert (msg.m_data.size ());
      lua_pop (L, 1); // remove result of table-lookup
      }  // end of having a defaults table

    msg.m_data.push_back (kv);

    } // end of looping through table

  lua_settop (L, 0);    // get rid of stuff lying around

  if (msg.DoModal () != IDOK)
    lua_pushnil (L);
  else
    {  // not cancelled
    lua_newtable(L);    // table of results

    set<int>::const_iterator iter;

    for (iter = msg.m_iResults.begin ();
         iter != msg.m_iResults.end ();
         iter++)
           {
            if (msg.m_data [*iter].bNumber_)
              lua_pushnumber (L, msg.m_data [*iter].iKey_);
            else
              lua_pushstring (L, msg.m_data [*iter].sKey_.c_str ()); 
            lua_pushboolean (L, 1); // 
            lua_settable (L, -3); // add key = true to table
           }   // end of processing each result
    }
  
  return 1;   // 1 result

} // end of multilistbox


extern tInternalFunctionsTable InternalFunctionsTable [1];

// returns table of internal functions
static int functionlist (lua_State *L) 
  {
  lua_newtable(L);    // table of function names

  for (int i = 0; InternalFunctionsTable [i].sFunction [0]; i++)
    {
    lua_pushstring (L, InternalFunctionsTable [i].sFunction);
    lua_rawseti(L, -2, i + 1);  // make 1-relative
    }

  return 1;   // 1 table
  } // end of functionlist

// returns table of internal functions keyed by name, value is argument list
static int functionargs (lua_State *L) 
  {
  lua_newtable(L);    // table of function names

  for (int i = 0; InternalFunctionsTable [i].sFunction [0]; i++)
    {
    lua_pushstring (L, InternalFunctionsTable [i].sFunction);
    lua_pushstring (L, InternalFunctionsTable [i].sArgs);
    lua_rawset(L, -3);  
    }

  return 1;   // 1 table
  } // end of functionargs

// returns table of internal functions
static int callbackslist (lua_State *L) 
  {
  lua_newtable(L);    // table of function names

  for (int i = 0;  PluginCallbacksNames [i] != ""; i++)
    {
    lua_pushstring (L, PluginCallbacksNames [i].c_str ());
    lua_rawseti(L, -2, i + 1);  // make 1-relative
    }

  return 1;   // 1 table
  } // end of callbackslist


// file picker routine
// arg1 = title of dialog
// arg2 = default file name
// arg3 = default extension
// arg4 = filter table 
// arg5 = true to save, false or nil to load
//
// returns: chosen file name, or nil if: cancelled, or none selected

static int filepicker (lua_State *L) 
  {
  BOOL bLoad;  // true to load, false to save
  DWORD flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

  const char * sTitle = luaL_optstring (L, 1, "");
  const char * sDefaultName = luaL_optstring (L, 2, "");
  const char * sDefaultExtension = luaL_optstring (L, 3, "");
  string sFilter;             // arg 4 = table
  bool bSave = optboolean (L, 5, 0);

  if (bSave)
    {
    bLoad = false;
    }
  else
    { 
    bLoad = TRUE;
    flags |= OFN_FILEMUSTEXIST;
    }

  if (sTitle [0] == 0)
    sTitle = NULL; 

  if (sDefaultName [0] == 0)
    sDefaultName = NULL; 

  if (sDefaultExtension [0] == 0)
    sDefaultExtension = NULL; 

  const int table = 4;

  if (lua_gettop (L) >= table && !lua_isnil (L, table))
    {

    if (!lua_istable (L, table))
      luaL_error (L, "argument 4 must be a table of filters, or nil");

    // construct filter

    // standard Lua table iteration
    for (lua_pushnil (L); lua_next (L, table) != 0; lua_pop (L, 1))
      {


      if (lua_type (L, -1) != LUA_TSTRING ||
          lua_type (L, -2) != LUA_TSTRING )
        luaL_error (L, "table of filters must be suffix/description pair");

      string sDescription = lua_tostring (L, -1);
      string sSuffix = lua_tostring (L, -2);

      sFilter += sDescription;
      sFilter += " (*.";
      sFilter += sSuffix;
      sFilter += ")|*.";
      sFilter += sSuffix;
      sFilter += "|";

      } // end of looping through table

    }      // end of not nil for argument 4

  sFilter += "|";

  CFileDialog dlg ( 
    bLoad,
    sDefaultExtension,
    sDefaultName,
    flags,
    sFilter.c_str (),
    NULL);    // parent window

  if (sTitle)
	  dlg.m_ofn.lpstrTitle = sTitle;

  ChangeToFileBrowsingDirectory ();
  int nResult = dlg.DoModal();
  ChangeToStartupDirectory ();

  if (nResult != IDOK)
	  lua_pushnil (L);
  else
    lua_pushstring (L, dlg.GetPathName());

  return 1;

  } // end of filepicker


// The field below is needed to initialise the browse directory dialog 
// with the initial directory

extern CString strStartingDirectory;

// function prototypes needed for folder browsing

extern int __stdcall BrowseCallbackProc(
    HWND hwnd, 	
    UINT uMsg, 	
    LPARAM lParam, 	
    LPARAM lpData	
   );

// directory picker routine
// arg1 = title of dialog
// arg2 = default directory name
//
// returns: chosen directory name, or nil if: cancelled, or none selected

// test: /print (utils.directorypicker ("test", "c:\\source"))

static int directorypicker (lua_State *L) 
  {
  const char * sTitle = luaL_optstring (L, 1, "");
  const char * sDefaultName = luaL_optstring (L, 2, "");

	// Gets the Shell's default allocator
	LPMALLOC pMalloc;
	if (::SHGetMalloc(&pMalloc) == NOERROR)
	{
		char	pszBuffer[MAX_PATH];  // receives result
		BROWSEINFO		bi;
		LPITEMIDLIST	pidl;

    // Get help on BROWSEINFO struct - it's got all the bit settings.
		bi.hwndOwner = NULL;
		bi.pidlRoot = NULL;
		bi.pszDisplayName = pszBuffer;
		bi.lpszTitle = sTitle;
		bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
    // if possible, let them create one
    if (!bWine)  
	  	bi.ulFlags |= BIF_NEWDIALOGSTYLE | BIF_EDITBOX;     // requires CoInitialize
		bi.lpfn = BrowseCallbackProc;
		bi.lParam = 0;
    strStartingDirectory = sDefaultName;

		// This next call issues the dialog box.
		if ((pidl = ::SHBrowseForFolder(&bi)) != NULL)
		{
			if (::SHGetPathFromIDList(pidl, pszBuffer))
        lua_pushstring (L, pszBuffer);
      else
    	  lua_pushnil (L);

			// Free the PIDL allocated by SHBrowseForFolder.
			pMalloc->Free(pidl);
		}
    else
  	  lua_pushnil (L);

		// Release the shell's allocator.
		pMalloc->Release();
    }

  return 1;

  } // end of directorypicker

static int spellcheckdialog (lua_State *L) 
  {
  CSpellCheckDlg dlg;

  dlg.m_strMisspeltWord = luaL_checkstring (L, 1);

  if (lua_gettop (L) >= 2 && !lua_isnil (L, 2))
    {

    if (!lua_istable (L, 2))
      luaL_error (L, "argument 2 must be a table of suggestions, or nil");

    for (int i = 1; ; i++)
      {
      lua_rawgeti (L, 2, i);   // get i'th item
      if (lua_isnil (L, -1))
        break;    // first nil key, leave loop
      dlg.m_suggestions.push_back (luaL_checkstring (L, -1));
      lua_pop (L, 1); // remove value
      } // end of looping through table

    }      // end of not nil for argument 2

  if (dlg.DoModal () != IDOK)
    {
    lua_pushnil (L);
    return 1;
    }

  lua_pushstring (L, dlg.m_sAction.c_str ());
  lua_pushstring (L, dlg.m_strMisspeltWord);
  return 2;    
  } // end of spellcheckdialog


static int metaphone (lua_State *L) 
  {

  MString m (luaL_checkstring (L, 1), luaL_optnumber (L, 2, 4));

  CString str1, str2;

  m.DoubleMetaphone (str1, str2);

  lua_pushstring (L, str1);  // first result
  if (str2.IsEmpty ())
    lua_pushnil (L);
  else
    lua_pushstring (L, str2);  // second result

  return 2;
  }  // end of metaphone

static int info (lua_State *L) 
  {

  lua_newtable(L);    // table to hold this stuff

  char * p = _getcwd (NULL, 1024);
  if (p)
    {
    CString strPath (p);
    free (p);
    if (strPath.Right (1) != "\\")
      strPath += "\\"; // append trailing backslash
    MakeTableItem (L, "current_directory", strPath);
    }

  MakeTableItem (L, "app_directory", ExtractDirectory (App.m_strMUSHclientFileName));
  MakeTableItem (L, "world_files_directory", App.m_strDefaultWorldFileDirectory);
  MakeTableItem (L, "state_files_directory", App.m_strDefaultStateFilesDirectory);
  MakeTableItem (L, "locale", App.m_strLocale);
  MakeTableItem (L, "fixed_pitch_font", App.m_strFixedPitchFont);
  MakeTableItem (L, "fixed_pitch_font_size", App.m_iFixedPitchFontSize);
  MakeTableItem (L, "log_files_directory", App.m_strDefaultLogFileDirectory);
  MakeTableItem (L, "plugins_directory", App.m_strPluginsDirectory);
  MakeTableItem (L, "startup_directory", CString (working_dir));
  MakeTableItem (L, "translator_file", App.m_strTranslatorFile);

  return 1;
  }  // end of metaphone


static int fontpicker  (lua_State *L) 
  {
  const char * fontname = luaL_optstring    (L, 1, "");     // preferred name
  const int   fontsize  = luaL_optnumber    (L, 2, 10);     // preferred size
  const int   fontcolour  = luaL_optnumber  (L, 3, 0);      // preferred colour


CDC dc;

  dc.CreateCompatibleDC (NULL);

  LOGFONT lf;

  ZeroMemory (&lf, sizeof lf);

  lf.lfHeight = -MulDiv(fontsize, dc.GetDeviceCaps(LOGPIXELSY), 72);
  strncpy (lf.lfFaceName, fontname, sizeof (lf.lfFaceName) - 1);
  lf.lfFaceName [sizeof (lf.lfFaceName) - 1] = 0;

  lf.lfWeight = FW_NORMAL;

  CFontDialog dlg (&lf,
                   CF_SCREENFONTS | CF_EFFECTS | CF_FORCEFONTEXIST | CF_USESTYLE,
                   NULL,
                   &Frame);      

  dlg.m_cf.rgbColors = fontcolour;

  if (dlg.DoModal () != IDOK)
    {
    lua_pushnil (L);
    return 1;
    }

  lua_newtable(L);    // table to hold this stuff

  MakeTableItem (L, "name",         dlg.GetFaceName ());
  MakeTableItem (L, "style",        dlg.GetStyleName ());
  MakeTableItem (L, "size",         dlg.GetSize () / 10);
  MakeTableItem (L, "colour",       dlg.GetColor  ());
  MakeTableItem (L, "bold",         dlg.IsBold  ());
  MakeTableItem (L, "italic",       dlg.IsItalic  ());
  MakeTableItem (L, "underline",    dlg.IsUnderline ());
  MakeTableItem (L, "strikeout",    dlg.IsStrikeOut ());
  MakeTableItem (L, "charset",      lf.lfCharSet);

  return 1;    // one table
  }  // end of fontpicker

static int edit_distance (lua_State *L) 
  {
  lua_pushinteger (L, EditDistance (luaL_checkstring (L, 1), luaL_checkstring (L, 2)));
  return 1;
}   // end of edit_distance

// convert the arguments (which should be numbers in the range 0 to 7FFFFFFF) into UTF-8

static int utf8encode (lua_State *L) {

  int numArgs = lua_gettop(L);  /* number of arguments */
  int i;
  string sOutput;

unsigned char utf8 [10];    // UTF-8 should be max 6 characters

  for (i= 1; i <= numArgs; i++) 
    {

    // if we get a table, we will convert everything in it
    if (lua_istable (L, i))
      {
      // process each table item
      for (int j = 1; ; j++)
        {
        lua_rawgeti (L, i, j);   // get j'th item

        if (lua_isnil (L, -1))
          break;    // first nil key, leave loop

        // check is number
        if (!lua_isnumber (L, -1))
          luaL_error (L, "Index [%d] of table at argument #%d to 'utf8encode' is not a number", j, i);

        double f = lua_tonumber (L, -1);

        if (f < 0 || f > LONG_MAX)
          luaL_error (L,  
             "Unicode code (%f) at index [%d] of table at argument #%d to 'utf8encode' is out of range", f, j, i);

        int n = f;

        if ((double) n != f)
          luaL_error (L,
             "Unicode code (%f) at index [%d] of table at argument #%d to 'utf8encode' has decimal places", f, j, i);

        int iLen = _pcre_ord2utf (n, utf8);

        // we do it this way so we can correctly append 0x00
        sOutput.append ((const char *) utf8, iLen);

        lua_pop (L, 1); // remove value from table
        } // end of looping through table
      }   // end of this argument is a table
    else
      {    // simple (non-table) argument
      double f = luaL_checknumber (L, i);

      if (f < 0 || f > LONG_MAX)
        luaL_error (L, 
          "Unicode code (%f) at argument #%d to 'utf8encode' is out of range", f, i);

      int n = f;

      if ((double) n != f)
         luaL_error (L, 
          "Unicode code (%f) at argument #%d to 'utf8encode' has decimal places", f, i);

      int iLen = _pcre_ord2utf (n, utf8);

      // we do it this way so we can correctly append 0x00
      sOutput.append ((const char *) utf8, iLen);
      }  // end of not table item

    }  // end of doing each argument

  lua_pushlstring (L, sOutput.c_str (), sOutput.size ());

  return 1;  /* return the string */
}  // utf8encode

// convert the supplied string into UTF-8

// / print (utils.utf8convert ("^You put (.*?) (.*?) FaetorAde ® into a Bag of Aardwolf\.$"))

static int utf8convert (lua_State *L) {
  const unsigned char * s = (const unsigned char *) luaL_checkstring (L, 1);
  string sOutput;

unsigned char utf8 [10];    // UTF-8 should be max 6 characters

  for (const unsigned char * p = s; *p; p++) 
    {

    int iLen = _pcre_ord2utf (*p, utf8);

    // we do it this way so we can correctly append 0x00
    sOutput.append ((const char *) utf8, iLen);

    }  // end of doing each argument

  lua_pushlstring (L, sOutput.c_str (), sOutput.size ());

  return 1;  /* return the string */
}  // utf8convert



/* ORIGINAL ONE using Windows calls - only good from 1 to 0xFFFF

static int utf8decode (lua_State *L) {

  // argument is string to be decoded
const char * data = luaL_checkstring (L, 1);

  int iBad = _pcre_valid_utf8 ((unsigned char  *) data, strlen (data));

  // use PCRE to validate the string first
  if (iBad > 0)
    luaL_error (L, "Bad UTF-8 string. Error at column %d", iBad + 1);

  // find how big table has to be
  int iLength = MultiByteToWideChar (CP_UTF8, 0, data, strlen (data), NULL, 0);

  // vector to hold Unicode
  vector<WCHAR> v;

  // adjust size
  v.resize (iLength);

  // do the conversion now
  MultiByteToWideChar (CP_UTF8, 0, data, -1, &v [0], iLength);

  lua_newtable(L);    // table of wide characters

  // copy each one into the table
  for (int i = 0; i < iLength; i++)
    {
    lua_pushnumber(L, v [i]);
    lua_rawseti(L, -2, i + 1); // 1-relative position
    }

  return 1;  // return the table
  }  // utf8decode

*/




// Nick's first version - should be OK for 1 to 1FFFFFFF

/*
static int utf8decode (lua_State *L) {

  // argument is string to be decoded
const char * data = luaL_checkstring (L, 1);

  int iBad = _pcre_valid_utf8 ((unsigned char  *) data, strlen (data));

  // use PCRE to validate the string first
  if (iBad > 0)
    luaL_error (L, "Bad UTF-8 string. Error at column %d", iBad + 1);

  lua_newtable (L);    // table of wide characters

  register const unsigned char *p = (const unsigned char *) data;

  int i = 1;   // table item number

  // this algorithm assumes the UTF-8 is OK, based on the earlier check
  // - you could probably roll the two loops together to save a bit of time,
  //   but at the cost of increased complexity.

  for (p = (const unsigned char *) data; *p; i++)
    {
    register int c = *p++;
    unsigned long num = 0;
    if (c < 128)
      num = c;  // normal character, 0 to 0x7F - just a single byte
    else if ((c & 0xc0) == 0xc0)
      {         // start of UTF8 sequence
      register int ab = _pcre_utf8_table4[c & 0x3f];  // Number of additional bytes

      // first extract the remaining bits from the first byte
      switch (ab)
        {
        case 1:
          num = c & 0x1F;
        break;

        case 2:
          num = c & 0x0F;
        break;

        case 3:
          num = c & 0x07;
        break;

        case 4:
          num = c & 0x03;
        break;

        case 5:
          num = c & 0x01;
        break;

        }   // end of switch on ab

      // the remaining bits are taken from the low-order 6 bits of each next byte

      for (int j = 0; j < ab; j++)
        {
        c = *p++;
        num = (num << 6) | (c & 0x3F);
        } // end of doing additional bytes

      }   // end of UTF8 sequence


    lua_pushnumber(L, num);
    lua_rawseti(L, -2, i); 

    } // end of loop


  return 1;  // return the table 
  }  // utf8decode

*/

/*

Unicode range              UTF-8 bytes

0x00000000 - 0x0000007F    0 xxxxxxx  (7 bits)
0x00000080 - 0x000007FF    110 xxxxx 10 xxxxxx  (11 bits)
0x00000800 - 0x0000FFFF    1110 xxxx 10 xxxxxx 10 xxxxxx  (16 bits)
0x00010000 - 0x001FFFFF    11110 xxx 10 xxxxxx 10 xxxxxx 10 xxxxxx  (21 bits)
0x00200000 - 0x03FFFFFF    111110 xx 10 xxxxxx 10 xxxxxx 10 xxxxxx 10 xxxxxx  (26 bits)
0x04000000 - 0x7FFFFFFF    1111110 x 10 xxxxxx 10 xxxxxx 10 xxxxxx 10 xxxxxx 10 xxxxxx (31 bits)

  */


// Nick's second version - should be OK for 0 to 1FFFFFFF

// single argument is string to be decoded

static int utf8decode (lua_State *L)
{
size_t length;
const char * data = luaL_checklstring (L, 1, &length);

  // on error, table is not returned and will be garbage-collected
  lua_newtable (L);    // table of wide characters

  // cast to unsigned so we don't have problems with negative numbers
  register const unsigned char *p = (const unsigned char *) data;

  int item;   // table item number

  for (item = 1; length-- > 0; item++)
    {
    register int ab;
    register int c = *p++;
    unsigned long num = 0;

    if (c < 128) 
      num = c;        // for 0 xxxxxxx, number is simply the byte
    else
      {
      if (c < 0xc0)   
        return 0;     // if not 0 xxxxxxx must be 11 xxxxxx

      ab = _pcre_utf8_table4 [c & 0x3f];  /* Number of additional bytes */
      if (length < ab) 
        return 0;     // too short

      length -= ab;

      /* Check for overlong sequences for each different length */
      switch (ab)
        {
        /* Check for xx00 000x */
        case 1:
          if ((c & 0x3e) == 0) 
            return 0;
          num = c & 0x1F;
          break;  

        /* Check for 1110 0000, 110x xxxx */
        case 2:
          if (c == 0xe0 && (*p & 0x20) == 0) 
            return 0;
          num = c & 0x0F;
          break;

        /* Check for 1111 0000, 1100 xxxx */
        case 3:
          if (c == 0xf0 && (*p & 0x30) == 0) 
            return 0;
          num = c & 0x07;
          break;

        /* Check for 1111 1000, 1100 0xxx */
          case 4:
          if (c == 0xf8 && (*p & 0x38) == 0) 
            return 0;
          num = c & 0x03;
          break;

        /* Check for leading 0xfe or 0xff, and then for 1111 1100, xx00 00xx */
        /* 0xFE would be 11111110 and 0xFF would be 11111111
            however we need 1111110x */
        case 5:
          if (c == 0xfe || c == 0xff ||
             (c == 0xfc && (*p & 0x3c) == 0)) 
             return 0;
          num = c & 0x01;
          break;
        }  // end of switch

      /* Check for valid bytes after the 2nd, if any; all must start 10 */
      while (ab-- > 0)
        {
        c = *p++;
        if ((c & 0xc0) != 0x80) 
          return 0;
        num = (num << 6) | (c & 0x3F);
        }
      } // end of c >= 128

    // store derived Unicode number
    lua_pushnumber(L, num);
    lua_rawseti (L, -2, item); 

    }  // end of for loop

  return 1;  // return table
}  // end of utf8decode


static int utf8valid (lua_State *L) {

size_t length;

  // argument is string to be checked
const char * data = luaL_checklstring (L, 1, &length);

  int erroroffset;
  int iBad = _pcre_valid_utf ((unsigned char  *) data, length, &erroroffset);

  if (iBad > 0)
    {
    lua_pushnil (L);
    lua_pushnumber (L, erroroffset + 1);
    return 2;  /* return nil and the offset */
    }

  // string is OK, calculate its length

  int i = 0;   // length

  // this algorithm assumes the UTF-8 is OK, based on the earlier check

  for (register const unsigned char *p = (const unsigned char *) data ; 
       length-- > 0; 
       i++)
    {          
    register int ab;    // additional bytes
    register int c = *p++;  // this byte

    if (c < 128)
      continue;     // zero additional bytes

    ab = _pcre_utf8_table4 [c & 0x3f];  /* Number of additional bytes */

    length -= ab;  // we know string is valid already, so just skip the additional bytes (ab)
    p += ab;

    }
  lua_pushnumber (L, i);  // final length

  return 1;
  }   // utf8valid

static int utf8sub (lua_State *L) {

  size_t length;
  const char *s = luaL_checklstring (L, 1, &length);
  ptrdiff_t start = luaL_checkinteger (L, 2);
  ptrdiff_t end = luaL_optinteger (L, 3, -1);

  // validate

  int erroroffset;
  int iBad = _pcre_valid_utf ((unsigned char  *) s, length, &erroroffset);

  if (iBad > 0)
    {
    lua_pushnil (L);
    lua_pushnumber (L, erroroffset + 1);
    return 2;  /* return nil and the offset */
    }

  int iUtf8Length = 0;   // length in code points

  register const unsigned char *p;

  // this algorithm assumes the UTF-8 is OK, based on the earlier check

  for (p = (const unsigned char *) s ; length-- > 0; iUtf8Length++)
    {          
    register int ab;    // additional bytes
    register int c = *p++;  // this byte

    if (c < 128)
      continue;     // zero additional bytes

    ab = _pcre_utf8_table4 [c & 0x3f];  /* Number of additional bytes */

    length -= ab;  // we know string is valid already, so just skip the additional bytes (ab)
    p += ab;

    }   // looping to find length in code points


  // negative start means count from right
  if (start < 0)
    start = iUtf8Length + start + 1;
  // can't be less than start of string
  if (start < 1)
    start = 1;

  // negative end means count from right
  if (end < 0)
    end = iUtf8Length + end + 1;
  // can't be greater than length of string
  if (end > iUtf8Length)
    end = iUtf8Length;

  // empty string
  if (start > end)
    lua_pushliteral(L, "");    
  else
    {
    int iCol = 1;   // column
    int iStart = 0; // start offset
    int iEnd = 0;   // end offset

    // this algorithm assumes the UTF-8 is OK 

    for (p = (const unsigned char *) s ; ; iCol++)
      {          

      if (iCol == start)
         iStart = (p - (const unsigned char *) s);

      if (iCol > end)
        {
        iEnd = (p - (const unsigned char *) s);
        break;
        }

      register int ab;    // additional bytes
      register int c = *p++;  // this byte

      if (c < 128)
        continue;     // zero additional bytes

      ab = _pcre_utf8_table4 [c & 0x3f];  /* Number of additional bytes */

      p += ab;

      }  // finding position of start and end column

    lua_pushlstring(L, s + iStart, iEnd - iStart);

    }  // end not empty string

  return 1;
  }   // utf8sub

static int appendtonotepad (lua_State *L)
{
const char * title = luaL_checkstring (L, 1);
const char * message = luaL_checkstring (L, 2);
bool bReplace = optboolean (L, 3, 0);

CString strMessage (message);

  strMessage.Replace ("\n", ENDLINE);

  BOOL bOK = App.AppendToTheNotepad (title, strMessage, bReplace, eNotepadPacketDebug);

  lua_pushboolean (L, bOK);
  return 1;
  } 

static int activatenotepad (lua_State *L)
{
const char * title = luaL_checkstring (L, 1);

  BOOL bOK = App.ActivateNotepad (title);

  lua_pushboolean (L, bOK);
  return 1;
  } 

// used in mainfrm.cpp
extern bool bShowDebugStatus;

static int showdebugstatus (lua_State *L)
{
  bShowDebugStatus = optboolean (L, 1, 1);
  return 0;
  } 


static int __stdcall EnumFontFamExProc (CONST LOGFONTA * lpelfe,   // pointer to logical-font data
                                 CONST TEXTMETRICA * lpntme,// pointer to physical-font data
                                 DWORD FontType,            // type of font
                                 LPARAM L_state)            // application-defined data
                                                            
  {
  lua_State *L = (lua_State *) L_state;


  lua_pushstring (L, lpelfe->lfFaceName);
  lua_pushboolean (L, true);
  lua_rawset(L, -3);

  return 1;   // keep enumerating
  } // end of EnumFontFamExProc


static int getfontfamilies (lua_State *L)
  {
  lua_newtable(L);    // table of entries

  CDC dc;

  dc.CreateCompatibleDC (NULL);

  LOGFONT lf;

  ZeroMemory (&lf, sizeof lf);

  lf.lfCharSet = DEFAULT_CHARSET;
  lf.lfFaceName [0] = 0;
  lf.lfPitchAndFamily = 0;

  EnumFontFamiliesEx (dc.m_hDC,           // handle to device context
                      &lf,                // pointer to logical font information
                      EnumFontFamExProc,  // pointer to callback function
                      (long) L,           // application-supplied data
                      0);                 // reserved; must be zero


  return 1;  // one table of entries

  }

/* sends the nominated window to the front, based on the 
   leading characters in its name (first match) 

Example:

  utils.sendtofront ("Crimson Editor")

*/

static int send_to_front (lua_State *L)
  {
  lua_pushboolean  (L, SendToFront (luaL_checkstring (L, 1)));    /* send_to_front_found flag */
  return 1;   /* one result */
  } /* end of send_to_front */

/* Execute an operating-system command:

utils.shellexecute (filename, params, defdir, operation, show_command)

Only the first argument (filename) is required, the rest are optional.

filename:  The file to open or print or the folder to open or explore. 
  The function can open an executable file or a document file. 
  The function can print a document file. 

params:    A string that specifies the parameters to be passed to the application. 
  If filename specifies a document file, params should be nil or "".

defdir:    A string that specifies the default directory.
   
operation: The following operation strings are valid: 
 "open":  The function opens the file specified by the filename parameter. 
          The file can be an executable file or a document file. It can also be a folder. 
 "print": The function prints the file specified by filename. 
          The file should be a document file. 
          If the file is an executable file, the function opens the file, 
          as if "open" had been specified. 
 "explore":  The function explores the folder specified by filename.  

  This parameter can be nil or "". 
  In that case, the function opens the file specified by filename. 

show_command: If filename specifies an executable file, show_command specifies 
   how the application is to be shown when it is opened. 

  This parameter can be nil in which case it defaults to 1 - the recommended default.
  
  This parameter can be one of the following values: 
  
   0:  Hides the window and activates another window. 
   1:  Activates and displays a window. 
       If the window is minimized or maximized, Windows restores it to 
       its original size and position. 
       An application should specify this flag when displaying the window for the first time. 
   2:  Activates the window and displays it as a minimized window. 
   3:  Activates the window and displays it as a maximized window. 
   4:  Displays a window in its most recent size and position. 
       The active window remains active. 
   5:  Activates the window and displays it in its current size and position.  
   6:  Minimizes the specified window and activates the next top-level window in the z-order. 
   7:  Displays the window as a minimized window. 
       The active window remains active. 
   8:  Displays the window in its current state. 
       The active window remains active. 
   9:  Activates and displays the window. 
       If the window is minimized or maximized, Windows restores it to its original 
       size and position. 
       An application should specify this flag when restoring a minimized window. 
  10:  Sets the show state based on the SW_ flag specified in the STARTUPINFO structure 
       passed to theCreateProcess function by the program that started the application. 
       An application should call ShowWindow with this flag to set the initial show 
       state of its main window. 
  

If sucessful, the function returns true. 

If not, it returns nil followed by an error message. You could use "assert" to test for failure.

Examples:

assert (utils.shellexecute ("c:/mushclient/worlds/SMAUG.MCL"))-- document
assert (utils.shellexecute ("http://www.gammon.com.au/"))     -- web page
assert (utils.shellexecute ("mailto:someone@somewhere.com"))  -- open mail client
assert (utils.shellexecute ("c:/", nil, nil, "explore"))      -- explore disk
assert (utils.shellexecute ("c:/readme.txt", nil, nil, "print"))  -- print a file

*/

static int shell_execute (lua_State *L)
  {
  const char * filename = luaL_checkstring (L, 1);
  const char * params = luaL_optstring (L, 2, "");
  const char * defdir = luaL_optstring (L, 3, "");
  const char * operation = luaL_optstring (L, 4, "open");
  const int  nShowCmd = (int) luaL_optnumber (L, 5, SW_SHOWNORMAL);
  const char * err = NULL;
  char buf [50];  // for unknown error codes

  int result = (int) ShellExecute (NULL, 
                                  operation, 
                                  filename, 
                                  params [0] ? params : NULL ,
                                  defdir [0] ? defdir : NULL ,
                                  nShowCmd);

  if (result > 32)
    {
    lua_pushboolean  (L, 1);    /* OK flag */
    return 1;   /* one result */
    }

  lua_pushnil (L);    /* bad result flag */

  switch (result)
    {
    case 0:                     
      err = "The operating system is out of memory or resources."; 
      break;
    case ERROR_FILE_NOT_FOUND : 
      err = "The specified file was not found."; 
      break;
    case ERROR_PATH_NOT_FOUND : 
      err = "The specified path was not found."; 
      break;
    case ERROR_BAD_FORMAT : 
      err = "The .exe file is invalid (non-Win32® .exe or error in .exe image)."; 
      break;
    case SE_ERR_ACCESSDENIED : 
      err = "The operating system denied access to the specified file."; 
      break;
    case SE_ERR_ASSOCINCOMPLETE : 
      err = "The file name association is incomplete or invalid."; 
      break;
    case SE_ERR_DDEBUSY : 
      err = "The DDE transaction could not be completed because other DDE transactions were being processed."; 
      break;
    case SE_ERR_DDEFAIL : 
      err = "The DDE transaction failed."; 
      break;
    case SE_ERR_DDETIMEOUT : 
      err = "The DDE transaction could not be completed because the request timed out."; 
      break;
    case SE_ERR_DLLNOTFOUND : 
      err = "The specified dynamic-link library was not found."; 
      break;
    case SE_ERR_NOASSOC : 
      err = "There is no application associated with the given file name extension."; 
      break;
    case SE_ERR_OOM : 
      err = "There was not enough memory to complete the operation."; 
      break;
    case SE_ERR_SHARE : 
      err = "A sharing violation occurred."; 
      break;
    default:
      _snprintf (buf, sizeof buf, "Unknown error code: %i returned.", result);
      buf [sizeof (buf) - 1] = 0;
      break;

    } /* end of switch on result */

  if (err == NULL) 
    err = buf;

  lua_pushstring (L, err);

  return 2; /* returning nil and error message  */

  } // end of shell_execute	


// arg1 is dialog title
// arg2 is table of key/value pairs - value is shown
// arg3 is initial filter
// arg4 is "no sort" boolean
// arg5 is filter function

static int filterpicker (lua_State *L) 
  {
  const char * filtertitle   = luaL_optstring (L, 2, "Filter");
  const char * initialfilter = luaL_optstring (L, 3, "");
  const bool bNoSort = optboolean (L, 4, 0);

  if (strlen (filtertitle) > 100)
     luaL_error (L, "title too long (max 100 characters)");

  const int table = 1;

  if (!lua_istable (L, table))
     luaL_error (L, "must have table of choices as first argument");

CFunctionListDlg dlg;

  dlg.m_strTitle    = filtertitle;
  dlg.m_strFilter   = initialfilter;
  dlg.m_bNoSort     = bNoSort;

  // standard Lua table iteration
  for (lua_pushnil (L); lua_next (L, table) != 0; lua_pop (L, 1))
    {
    if (!lua_isstring (L, -2))
      luaL_error (L, "table must have string or number keys");

    if (!lua_isstring (L, -1))
      luaL_error (L, "table must have string or number values");

    // value can simply be converted to a string
    string sValue = lua_tostring (L, -1);

    // get key
    CKeyValuePair kv (sValue);
    
    if (lua_type (L, -2) == LUA_TSTRING)
      {
      kv.sKey_ = lua_tostring (L, -2);
      }
    else
      {     // not string, should be number :)
      // cannot do lua_tostring because that confuses lua_next
      kv.bNumber_ = true;
      kv.iKey_ = lua_tonumber (L, -2);
      }   // end of key being a number


    dlg.m_data.push_back (kv);

    } // end of looping through table


  // filter function
  if (lua_gettop (L) > 4)
    {
    if (!lua_isnil (L, 5))
      {
      luaL_checktype (L, 5, LUA_TFUNCTION);
      lua_pushvalue (L, 5);    // function is now on top of stack   

      dlg.m_L = L; 

      // we can't leave the function on the stack, that gets cleared from time to time
      // while the dialog box is running - so we store it in the registry and get the
      // unique index back
      dlg.m_iFilterIndex = luaL_ref (L, LUA_REGISTRYINDEX);

      }
    }

  // filter prep function
  if (lua_gettop (L) > 5)
    {
    if (!lua_isnil (L, 6))
      {
      luaL_checktype (L, 6, LUA_TFUNCTION);
      lua_pushvalue (L, 6);    // function is now on top of stack   

      dlg.m_L = L; 

      // we can't leave the function on the stack, that gets cleared from time to time
      // while the dialog box is running - so we store it in the registry and get the
      // unique index back
      dlg.m_iFilterPrepIndex = luaL_ref (L, LUA_REGISTRYINDEX);

      }
    }

  if (dlg.DoModal () != IDOK)
    lua_pushnil (L);
  else
    {  // not cancelled
    if (dlg.m_result.sValue_ == "")
      lua_pushnil (L);   // no choice made
    else
      {    // get key of choice
      if (dlg.m_result.bNumber_)
        lua_pushnumber (L, dlg.m_result.iKey_);
      else
        lua_pushstring (L, dlg.m_result.sKey_.c_str ()); 
      }
    }
  
  // free up registry entry  (is OK with LUA_NOREF)
  luaL_unref(L, LUA_REGISTRYINDEX, dlg.m_iFilterIndex);
  luaL_unref(L, LUA_REGISTRYINDEX, dlg.m_iFilterPrepIndex);

  return 1;   // 1 result

} // end of filterpicker

static int timer (lua_State *L) 
  {
  if (App.m_iCounterFrequency)
    {
    LARGE_INTEGER the_time;
    QueryPerformanceCounter (&the_time);
    lua_pushnumber (L, (double) the_time.QuadPart / (double) App.m_iCounterFrequency);
    }
  else
    {
    time_t timer;
    time (&timer);
    lua_pushnumber (L, (double) timer);
    }

  return 1;   // 1 result
  }  // end timer


// returns table of GetInfo selectors and their descriptions
static int infotypes (lua_State *L) 
  {
  lua_newtable(L);

  for (int iCount = 0; InfoTypes [iCount].iInfoType; iCount++)
    {
    lua_pushstring (L, InfoTypes [iCount].sDescription);
    lua_rawseti(L, -2, InfoTypes [iCount].iInfoType); 
    }

  return 1;   // 1 table
  } // end of infotypes

// reloads global preferences
static int reload_global_prefs (lua_State *L) 
  {
  App.LoadGlobalsFromDatabase ();

  return 0;   // no results
  } // end of reload_global_prefs

#define GGI_MARK_NONEXISTING_GLYPHS  0X0001

/*
typedef struct tagWCRANGE
{
    WCHAR  wcLow;
    USHORT cGlyphs;
} WCRANGE, *PWCRANGE,FAR *LPWCRANGE;

typedef struct tagGLYPHSET
{
    DWORD    cbThis;
    DWORD    flAccel;
    DWORD    cGlyphsSupported;
    DWORD    cRanges;
    WCRANGE  ranges[1];
} GLYPHSET, *PGLYPHSET, FAR *LPGLYPHSET;

*/

// sees if a glyph is available in the current font
static int glyph_available (lua_State *L) 
  {
  // which glyph?
  const char * fontName   = luaL_checkstring (L, 1);

  // this DLL might not be available
	HMODULE hDLL = LoadLibrary ("gdi32");
  if (!hDLL)
    {
    lua_pushnil (L); 
    return 1;
    }

  // make a device contect
  CDC dc;
  dc.CreateCompatibleDC (NULL);

  int lfHeight = -MulDiv(5, dc.GetDeviceCaps(LOGPIXELSY), 72);
  CFont font;

  // select requested font into it
  font.CreateFont(lfHeight, // int nHeight, 
        0,                  // int nWidth, 
        0,                  // int nEscapement, 
        0,                  // int nOrientation, 
        FW_DONTCARE,        // int nWeight, 
        0,                  // BYTE bItalic, 
        0,                  // BYTE bUnderline, 
        0,                  // BYTE cStrikeOut, 
        DEFAULT_CHARSET,    // BYTE nCharSet, 
        OUT_DEVICE_PRECIS,  // BYTE nOutPrecision, 
        CLIP_DEFAULT_PRECIS,// BYTE nClipPrecision, 
        DEFAULT_QUALITY,    // BYTE nQuality, 
        FF_DONTCARE,        // BYTE nPitchAndFamily,  
        fontName);          // LPCTSTR lpszFacename

  dc.SelectObject(font);

  // test a single character
  WORD glyph = luaL_checknumber(L, 2);    // which Unicode character

  typedef DWORD (CALLBACK* GetGlyphIndicesW_PROC)(HDC, LPCWSTR, int, LPWORD, DWORD);
  GetGlyphIndicesW_PROC pGetGlyphIndicesW = NULL;

  // get function address
  pGetGlyphIndicesW = (GetGlyphIndicesW_PROC) GetProcAddress(hDLL, "GetGlyphIndicesW");

  // if it exists, call it
  if (pGetGlyphIndicesW)
    {
	  WORD indice;

	  if ((*pGetGlyphIndicesW)(dc.m_hDC, &glyph, 1,
	                            &indice, GGI_MARK_NONEXISTING_GLYPHS ) != GDI_ERROR &&
	          indice != 0xffff)
      lua_pushnumber (L, indice);  
    else
      lua_pushnumber (L, 0); 
    }
  else
    lua_pushnumber (L, 0); 
  
  return 1;   // one result

  /*
  // return a table of what glyphs are available

  typedef GLYPHSET * (CALLBACK* GetFontUnicodeRanges_PROC)(HDC, void *);
  GetFontUnicodeRanges_PROC pGetFontUnicodeRanges = NULL;

	pGetFontUnicodeRanges = (GetFontUnicodeRanges_PROC) GetProcAddress(hDLL, "GetFontUnicodeRanges");

  if (pGetFontUnicodeRanges)
    {
    DWORD requiredSize = (DWORD) (*pGetFontUnicodeRanges) (dc.m_hDC, NULL);
    GLYPHSET * pGlyphSet = (GLYPHSET * ) malloc (requiredSize);
    pGlyphSet->cbThis = requiredSize;
    pGlyphSet->flAccel = 0;
    (*pGetFontUnicodeRanges) (dc.m_hDC, pGlyphSet);

    lua_newtable(L);    // table  

    for (int i = 0; i < pGlyphSet->cRanges; i++)
      {
      lua_newtable(L);    // table for the ranges 
      MakeTableItem (L, "from", pGlyphSet->ranges [i].wcLow);
      MakeTableItem (L, "to",   pGlyphSet->ranges [i].wcLow + pGlyphSet->ranges [i].cGlyphs - 1);
      lua_rawseti(L, -2, i + 1);  // make 1-relative
      }

    free (pGlyphSet);

    return 1;  // one table
    }

  lua_pushnumber (L, 0); 
  return 1;   // one result (namely 0)

  */


  } // end of glyph_available

extern  COLORREF xterm_256_colours [256];
  
static int colourcube (lua_State *L) {
  int which = luaL_checknumber (L, 1);
  int red, green, blue;
  const BYTE values [6] = { 
      0, 
     95, 
     95 + 40, 
     95 + 40 + 40, 
     95 + 40 + 40 + 40, 
     95 + 40 + 40 + 40 + 40
      };
  const BYTE colour_increment = 255 / 5;     // that is, 51 (0x33)

  switch (which)
    {

    case 1:  // Xterm colour cube (the default - favours brighter colours)
      for (red = 0; red < 6; red++)
        for (green = 0; green < 6; green++)
          for (blue = 0; blue < 6; blue++)
             xterm_256_colours [16 + (red * 36) + (green * 6) + blue] =
                RGB (values [red], values [green], values [blue]);

      break;

    case 2:  // Netscape colour cube - evenly spaced colour cube
      for (red = 0; red < 6; red++)
          for (green = 0; green < 6; green++)
            for (blue = 0; blue < 6; blue++)
               xterm_256_colours [16 + (red * 36) + (green * 6) + blue] =
                  RGB (red   * colour_increment, 
                       green * colour_increment, 
                       blue  * colour_increment);

      break;

    default: luaL_error (L, "Unknown option");
    } // end of switch

  return 0;  /* no return */
}

#if 0
static int registry (lua_State *L)
  {
  lua_pushvalue(L, LUA_REGISTRYINDEX);
  return 1;
  } // end of registry

static int environment (lua_State *L)
  {
  lua_pushvalue(L, LUA_ENVIRONINDEX);
  return 1;
  }  // end of environment

static int globals (lua_State *L)
  {
  lua_pushvalue(L, LUA_GLOBALSINDEX);
  return 1;
  }  // end of globals

#endif  // 0

// table of operations
static const struct luaL_Reg xmllib [] = 
  {

  {"activatenotepad",   activatenotepad},
  {"appendtonotepad",   appendtonotepad},
  {"callbackslist",     callbackslist},
  {"choose",            choose},
  {"colourcube",        colourcube},
  {"directorypicker",   directorypicker},
  {"edit_distance",     edit_distance},
  {"editbox",           editbox},
  {"filepicker",        filepicker},
  {"filterpicker",      filterpicker},
  {"fontpicker",        fontpicker},
  {"functionargs",      functionargs},
  {"functionlist",      functionlist},
  {"getfontfamilies",   getfontfamilies},
  {"glyph_available",   glyph_available},
  {"info",              info},
  {"infotypes",         infotypes},
  {"inputbox",          inputbox},
  {"listbox",           listbox},
  {"metaphone",         metaphone},
  {"msgbox",            msgbox},       // msgbox - not Unicode
  {"multilistbox",      multilistbox},
  {"reload_global_prefs", reload_global_prefs},
  {"showdebugstatus",   showdebugstatus},
  {"sendtofront",       send_to_front},
  {"shellexecute",      shell_execute},
  {"spellcheckdialog",  spellcheckdialog},
  {"timer",             timer},
  {"umsgbox",           umsgbox},      // msgbox - UTF8
  {"utf8decode",        utf8decode}, 
  {"utf8encode",        utf8encode}, 
  {"utf8convert",       utf8convert}, 
  {"utf8sub",           utf8sub},
  {"utf8valid",         utf8valid},
  {"xmlread",           xmlread},

#if 0
  // Lua debugging
  {"registry",          registry},
  {"environment",       environment},
  {"globals",           globals},
#endif  // 0

  {NULL, NULL}
  };

const struct luaL_Reg *ptr_xmllib = xmllib;

