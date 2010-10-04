// methods_spellchecker.cpp

// Spellchecker and related helper functions

#include "stdafx.h"
#include "..\..\MUSHclient.h"
#include "..\..\doc.h"
#include "..\errors.h"
#include "..\..\sendvw.h"
#include "..\..\dmetaph.h"

// Implements:

//    AddSpellCheckWord
//    EditDistance
//    Metaphone
//    SpellCheck
//    SpellCheckCommand
//    SpellCheckDlg


VARIANT CMUSHclientDoc::SpellCheck(LPCTSTR Text) 
{

const char sFunction [] = "spellcheck_string";

	VARIANT vaResult;
	VariantInit(&vaResult);

  if (!App.m_bSpellCheckOK)
    return vaResult;

set<string> errors;

  if (App.m_SpellChecker_Lua)
    {

    lua_settop(App.m_SpellChecker_Lua, 0);   // clear stack

    lua_getglobal (App.m_SpellChecker_Lua, sFunction);  
    if (!lua_isfunction (App.m_SpellChecker_Lua, -1))
      return vaResult;  // cannot spell check string

    lua_pushstring (App.m_SpellChecker_Lua, Text);  // string to be checked

    int narg = lua_gettop(App.m_SpellChecker_Lua) - 1;  // all but the function
    int error = CallLuaWithTraceBack (App.m_SpellChecker_Lua, narg, 1);
  
    if (error)
      {
      LuaError (App.m_SpellChecker_Lua, "Run-time error", sFunction, "world.SpellCheck", "", this);
      return vaResult;  // cannot spell check string - syntax error
      }  

    if (lua_isnumber (App.m_SpellChecker_Lua, -1))
      {
      SetUpVariantLong (vaResult, lua_tonumber (App.m_SpellChecker_Lua, -1));        // no errors
  	  return vaResult;
      }

    // must be table or else return bad result
    if (!lua_istable (App.m_SpellChecker_Lua, -1))
      return vaResult;  // cannot spell check string - syntax error

    // convert returned table into a set
    for (int i = 1; ; i++)
      {
      lua_rawgeti (App.m_SpellChecker_Lua, 1, i);   // get i'th item
      if (lua_isnil (App.m_SpellChecker_Lua, -1))
        break;    // first nil key, leave loop
      // to avoid crashes, ignore table items that are not strings
      if (lua_isstring (App.m_SpellChecker_Lua, -1))
         errors.insert (lua_tostring (App.m_SpellChecker_Lua, -1));
      lua_pop (App.m_SpellChecker_Lua, 1); // remove value
      } // end of looping through table

    // maybe didn't find any errors?
    if (errors.empty ())
      {
      SetUpVariantLong (vaResult, 0);        // no errors
  	  return vaResult;
      }

    // now make array of the errors
    COleSafeArray sa;   // for wildcard list

    sa.CreateOneDim (VT_VARIANT, errors.size ());

    long iCount = 0;

    for (set<string>::const_iterator it = errors.begin (); 
         it != errors.end (); it++, iCount++)
      {
      // the array must be a bloody array of variants, or VBscript kicks up
      COleVariant v (it->c_str ());
      sa.PutElement (&iCount, &v);
      }      // end of looping through each error

	  return sa.Detach ();
    }   // end custom spell check


return vaResult;

} // end of SpellCheck
  

long CMUSHclientDoc::SpellCheckCommand(long StartCol, long EndCol) 
{
  if (!App.m_bSpellCheckOK)
    return -1;

  CEdit * pEdit = NULL;
  CWnd * pWnd = NULL;

  // find command window

  for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	  {
	  CView* pView = GetNextView(pos);
	  
	  if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
  	  {
		  CSendView* pmyView = (CSendView*)pView;

      // what is the current selection?

      pWnd = pmyView;
      pEdit = & (pmyView->GetEditCtrl());
      break;
      
      }	  // end of being a CSendView
    }   // end of loop through views

  if (pEdit == NULL)
	  return -1;    // couldn't find it

  int nStartChar, 
      nEndChar;

  // get current selection
  pEdit->GetSel (nStartChar, nEndChar); 

  // make wanted selection 1-relative
  if (StartCol > 0)
    StartCol--;

  bool bHaveSelection = EndCol > StartCol &&
                         StartCol >= 0 &&
                         EndCol >= 0;

  // select what the scripter wanted
  if (bHaveSelection)
     pEdit->SetSel (StartCol, EndCol); 

  if (App.m_SpellChecker_Lua)
    {

    lua_settop(App.m_SpellChecker_Lua, 0);   // clear stack

    lua_getglobal (App.m_SpellChecker_Lua, SPELLCHECKFUNCTION);  
    if (!lua_isfunction (App.m_SpellChecker_Lua, -1))
      return true;  // assume ok, what can we do?

    CString strText;
    bool bAll = GetSelection (pEdit, strText);
    
    lua_pushstring (App.m_SpellChecker_Lua, strText);  // string to be checked
    lua_pushboolean (App.m_SpellChecker_Lua, bAll);    // doing all?

    int narg = lua_gettop(App.m_SpellChecker_Lua) - 1;  // all but the function
    int error = CallLuaWithTraceBack (App.m_SpellChecker_Lua, narg, 1);
    
    if (error)
      {
      LuaError (App.m_SpellChecker_Lua, "Run-time error", SPELLCHECKFUNCTION, "Command-line spell-check");
      lua_close (App.m_SpellChecker_Lua);
      App.m_SpellChecker_Lua = NULL;
      App.m_bSpellCheckOK = false;
      return -1;    
      }  

    if (lua_isstring (App.m_SpellChecker_Lua, -1))
      {
      const char * p = lua_tostring (App.m_SpellChecker_Lua, -1);
      if (bAll)
        pEdit->SetSel (0, -1, TRUE);
      pEdit->ReplaceSel (p, true);
      // put original selection back
      pEdit->SetSel (nStartChar, nEndChar); 
      return 1;   // spell checked ok
      }

    // put original selection back
    pEdit->SetSel (nStartChar, nEndChar); 
    return 0;      // they cancelled
    }

  return -1;

} // end of CMUSHclientDoc::SpellCheckCommand


VARIANT CMUSHclientDoc::SpellCheckDlg(LPCTSTR Text) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  if (!App.m_bSpellCheckOK)
    return vaResult;


  if (App.m_SpellChecker_Lua)
    {

    lua_settop(App.m_SpellChecker_Lua, 0);   // clear stack

    lua_getglobal (App.m_SpellChecker_Lua, SPELLCHECKFUNCTION);  
    if (!lua_isfunction (App.m_SpellChecker_Lua, -1))
      return vaResult;  // cannot spell check string

    lua_pushstring (App.m_SpellChecker_Lua, Text);  // string to be checked

    int narg = lua_gettop(App.m_SpellChecker_Lua) - 1;  // all but the function
    int error = CallLuaWithTraceBack (App.m_SpellChecker_Lua, narg, 1);
  
    if (error)
      {
      LuaError (App.m_SpellChecker_Lua, "Run-time error", SPELLCHECKFUNCTION, "world.SpellCheckDlg", "", this);
      return vaResult;  // cannot spell check string - syntax error or dialog cancelled
      }  

    // must be string or else return bad result
    if (!lua_isstring (App.m_SpellChecker_Lua, -1))
      return vaResult;  // shouldn't happen? maybe dialog cancelled

    SetUpVariantString (vaResult, lua_tostring (App.m_SpellChecker_Lua, -1)); 
    }


	return vaResult;
}   // end of CMUSHclientDoc::SpellCheckDlg



long CMUSHclientDoc::AddSpellCheckWord(LPCTSTR OriginalWord, LPCTSTR ActionCode, LPCTSTR ReplacementWord) 
{

const char sFunction [] = "spellcheck_add_word";

  // make sure we enabled spell check
  if (!App.m_bSpellCheckOK)
    return eSpellCheckNotActive;

  // word to match on should be supplied
  if (strlen (OriginalWord) <= 0 || strlen (OriginalWord) > 63)
    return eBadParameter;

  // replacement word is optional
  if (strlen (ReplacementWord) > 63)
    return eBadParameter;

  // check action is single letter
  if (strlen (ActionCode) != 1)
    return eUnknownOption;

unsigned char cAction = ActionCode [0];

  // validate action code
  switch (cAction)
    {
    case 'a':
    case 'A':
    case 'c':
    case 'C':
    case 'e':
    case 'i':
      break;

    default:
      return eUnknownOption;
    }


  if (App.m_SpellChecker_Lua)
    {

    lua_settop(App.m_SpellChecker_Lua, 0);   // clear stack

    lua_getglobal (App.m_SpellChecker_Lua, sFunction);  
    if (!lua_isfunction (App.m_SpellChecker_Lua, -1))
      return eSpellCheckNotActive;  // cannot spell check string

    lua_pushstring (App.m_SpellChecker_Lua, OriginalWord);      // word to be added
    lua_pushstring (App.m_SpellChecker_Lua, ActionCode);        // action
    lua_pushstring (App.m_SpellChecker_Lua, ReplacementWord);   // replacement

    int narg = lua_gettop(App.m_SpellChecker_Lua) - 1;  // all but the function
    int error = CallLuaWithTraceBack (App.m_SpellChecker_Lua, narg, 1);
  
    if (error)
      {
      LuaError (App.m_SpellChecker_Lua, "Run-time error", sFunction, "world.AddSpellCheckWord", "", this);
      return eSpellCheckNotActive;  // cannot spell check string - syntax error
      }  

    // if it returns false, there was a problem
    if (lua_isboolean (App.m_SpellChecker_Lua, -1))
      {
      int ok = lua_toboolean (App.m_SpellChecker_Lua, -1);
      if (!ok)
        return eBadParameter;
      }

    return eOK;
    }   // end custom spell check


  return eSpellCheckNotActive;

} // end of  CMUSHclientDoc::AddSpellCheckWord


BSTR CMUSHclientDoc::Metaphone(LPCTSTR Word, short Length) 
{

// char buf [60];
//  metaphone (Word, buf, Length);

  MString m (Word, Length);

  CString str1, str2;

  m.DoubleMetaphone (str1, str2);

	CString strResult (str1);

  if (!str2.IsEmpty ())
    strResult += "," + str2;

	return strResult.AllocSysString();
}   // end of CMUSHclientDoc::Metaphone


long CMUSHclientDoc::EditDistance(LPCTSTR Source, LPCTSTR Target) 
{
  return ::EditDistance (Source, Target);
}   // end of CMUSHclientDoc::EditDistance
