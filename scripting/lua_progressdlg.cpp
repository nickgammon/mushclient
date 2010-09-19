#include "stdafx.h"
#include "..\MUSHclient.h"
#include "..\mainfrm.h"

#include "..\dialogs\ProgDlg.h"


//----------------------- begin Lua stuff ----------------------------

const char progress_dlg_handle[] = "progress_dialog_handle";


static CProgressDlg * Lprogress_getdialog (lua_State *L)
{
  CProgressDlg **ud = (CProgressDlg **) luaL_checkudata (L, 1, progress_dlg_handle);
  luaL_argcheck(L, *ud != NULL, 1, "progress dialog userdata expected");
  return *ud;
  }

static int Lprogress_setstatus(lua_State *L)
  {
  CProgressDlg *pProgressDlg = Lprogress_getdialog (L);

  string word (luaL_checkstring (L, 2));
  pProgressDlg->SetStatus (word.c_str ());
  return 0;
  } // end of Lprogress_setstatus

static int Lprogress_setrange(lua_State *L)
  {
  CProgressDlg *pProgressDlg = Lprogress_getdialog (L);

  const int iStart = luaL_checkinteger (L, 2);
  const int iEnd = luaL_checkinteger (L, 3);
  pProgressDlg->SetRange (iStart, iEnd);
  return 0;
  } // end of Lprogress_setrange

static int Lprogress_setposition(lua_State *L)
  {
  CProgressDlg *pProgressDlg = Lprogress_getdialog (L);

  const int iPos = luaL_checkinteger (L, 2);
  pProgressDlg->SetPos (iPos);
  return 0;
  } // end of Lprogress_setposition

static int Lprogress_setstep(lua_State *L)
  {
  CProgressDlg *pProgressDlg = Lprogress_getdialog (L);

  const int iStep = luaL_checkinteger (L, 2);
  pProgressDlg->SetStep (iStep);
  return 0;
  } // end of Lprogress_setstep

static int Lprogress_stepit(lua_State *L)
  {
  CProgressDlg *pProgressDlg = Lprogress_getdialog (L);

  pProgressDlg->StepIt ();
  return 0;
  } // end of Lprogress_stepit

static int Lprogress_checkcancel(lua_State *L)
  {
  CProgressDlg *pProgressDlg = Lprogress_getdialog (L);
  lua_pushboolean (L, pProgressDlg->CheckCancelButton ());
  return 1;
  } // end of Lprogress_checkcancel

// done with the dialog, delete it
static int Lprogress_gc (lua_State *L) {
  CProgressDlg **ud = (CProgressDlg **) luaL_checkudata (L, 1, progress_dlg_handle);
  CProgressDlg *pProgressDlg = *ud;  // note, might be NULL already if they manually closed it
  delete pProgressDlg;
  // set userdata to NULL, so we don't try to use it now
  *ud = NULL;
  return 0;
  }  // end of Lprogress_gc

// tostring helper
static int Lprogress_tostring (lua_State *L) 
  {
  lua_pushstring(L, "progress_dialog");
  return 1;
}  // end of Lprogress_tostring

//----------------------- create a new trie object ----------------------------

static int Lprogress_new(lua_State *L)
{
  CProgressDlg *pProgressDlg = new CProgressDlg (); 
  pProgressDlg->Create ();
  pProgressDlg->SetWindowText (luaL_optstring (L, 1, "Progress ..."));
  CProgressDlg **ud = (CProgressDlg **)lua_newuserdata(L, sizeof (CProgressDlg *));
  luaL_getmetatable(L, progress_dlg_handle);
  lua_setmetatable(L, -2);
  *ud = pProgressDlg;    // store pointer to this dialog in the userdata
  return 1;
  }  // end of Lprogress_new



static const luaL_reg progress_dialog_meta[] = {

  {"__gc",       Lprogress_gc},
  {"__tostring", Lprogress_tostring},
  {"checkcancel",Lprogress_checkcancel},// cancelled?
  {"close",      Lprogress_gc},         // close dialog (same as garbage-collect)
  {"position",   Lprogress_setposition},// set position
  {"range",      Lprogress_setrange},   // set range
  {"setstep",    Lprogress_setstep},    // set step amount?
  {"status",     Lprogress_setstatus},  // set status description
  {"step",       Lprogress_stepit},     // step it

  {NULL, NULL}
};


/* Open the library */

static const luaL_reg progress_dialog_lib[] = {
  {"new",     Lprogress_new},
  {NULL, NULL}
};

static void createmeta(lua_State *L, const char *name)
{
  luaL_newmetatable(L, name);   /* create new metatable */
  lua_pushliteral(L, "__index");
  lua_pushvalue(L, -2);         /* push metatable */
  lua_rawset(L, -3);            /* metatable.__index = metatable */
}

LUALIB_API int luaopen_progress_dialog(lua_State *L)
{
  createmeta(L, progress_dlg_handle);
  luaL_register (L, NULL, progress_dialog_meta);
  lua_pop(L, 1);
  luaL_register (L, "progress", progress_dialog_lib);
  return 1;
}

