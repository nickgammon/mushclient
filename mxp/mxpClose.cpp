// mxpClose.cpp - MXP close tag

// called from closing tag or automatic closure

#include "stdafx.h"
#include "..\MUSHclient.h"

#include "..\doc.h"
#include "..\MUSHview.h"
#include "..\mainfrm.h"
#include "mxp.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define MAX_TEXT 1000     // max size for &text; variable

void CMUSHclientDoc::MXP_CloseTag (CString strTag, const bool bOpen)
  {

  POSITION linepos = 0, 
           stylepos, 
           oldstylepos = 0,
           oldlinepos = 0;
  CStyle * pStyle = NULL;
  CLine * pLine = NULL;
  bool bFoundit = false;
  CString strFoundVariable;

  // work backwards through the lines
  for (linepos = m_LineList.GetTailPosition (); linepos && !bFoundit; )
    {
    oldlinepos = linepos;   // for going forwards again
    pLine = m_LineList.GetPrev (linepos);

    // work backwards through the styles on the line
    for (stylepos = pLine->styleList.GetTailPosition (); stylepos && !bFoundit; )
      {
      oldstylepos = stylepos;   // where we found it
      pStyle = pLine->styleList.GetPrev (stylepos);

      if ((pStyle->iFlags & START_TAG) &&
           pStyle->pAction)
        if (pStyle->pAction->m_strAction == strTag)
          bFoundit = true;

      // this seems whacky, but it seems the variable name is on a separate
      // style to the <var> tag itself - try to remember what variable to set
      if (pStyle->pAction && !pStyle->pAction->m_strVariable.IsEmpty ())
        strFoundVariable = pStyle->pAction->m_strVariable;

      }   // end of style loop
    } // end of line loop

  if (!bFoundit)
    {
    MXP_error (DBG_WARNING, wrnMXP_OpenTagNotInOutputBuffer,
              TFormat ("Opening MXP tag <%s> not found in output buffer", 
              (LPCTSTR) strTag)); 
    return;
    }

  // OK - tag was opened at pStyle in pLine at position oldstylepos

CString strVariable = "mxp_";
bool bHaveVariable = false;

   if (pStyle->pAction &&
     !pStyle->pAction->m_strVariable.IsEmpty ())
     {
     bHaveVariable = true;
     strVariable += pStyle->pAction->m_strVariable;
     }
   else if (((strTag == "var") || (strTag == "v")) && !strFoundVariable.IsEmpty ())
     {
     // <var>blah</var> is a bit different
     bHaveVariable = true;
     strVariable += strFoundVariable;
     }
 
  // establish text of characters between start tag and end of buffer

  CString strText;
  bool bStart = false;
  int iBytesToGo = MAX_TEXT;
  char * p = strText.GetBuffer (MAX_TEXT);

  for (linepos = oldlinepos; linepos && iBytesToGo > 2; )
    {
    CLine * pLine2 = m_LineList.GetNext (linepos);
    int iCol = 0;
    int iLen = 0;
 
    for (stylepos = pLine2->styleList.GetHeadPosition () ; stylepos; )
      {
      CStyle * pStyle2 = pLine2->styleList.GetNext (stylepos);
      if (pStyle2 == pStyle)
        bStart = true;
      if (bStart)
        iLen += pStyle2->iLength; // count length
      else
        iCol += pStyle2->iLength; // starting column
      }   // end of each style

    int iCopy = MIN (iLen, iBytesToGo);

    // copy line text
    
    memcpy (p, &pLine2->text [iCol], iCopy);
    p += iCopy;
    iBytesToGo -= iCopy;
    
//    strText += CString (pLine2->text, pLine2->len).Mid (iCol, iLen);

    // include newlines on intermediate lines
    if (linepos && pLine2->hard_return && iBytesToGo >= 2)
      {
      memcpy (p, ENDLINE, 2);
      p += 2;
      iBytesToGo -= 2;
      }

    }   // end of each line

  strText.ReleaseBuffer (MAX_TEXT - iBytesToGo);

  // call script if required
  if (m_dispidOnMXP_CloseTag != DISPID_UNKNOWN)
    {
    long nInvocationCount = 0;

    CString strType = "MXP close tag";
    CString strReason =  TFormat ("closing MXP tag %s", (LPCTSTR) strTag);

    if (GetScriptEngine () && GetScriptEngine ()->IsLua ())
      {
      list<double> nparams;
      list<string> sparams;
      sparams.push_back ((LPCTSTR) strTag);
      sparams.push_back ((LPCTSTR) strText);
      GetScriptEngine ()->ExecuteLua (m_dispidOnMXP_CloseTag, 
                                     m_strOnMXP_CloseTag, 
                                     eWorldAction,
                                     strType, 
                                     strReason, 
                                     nparams,
                                     sparams, 
                                     nInvocationCount); 
      }   // end of Lua
    else
      {
      // WARNING - arguments should appear in REVERSE order to what the sub expects them!

      enum
        {
        eText,
        eTagName,
        eArgCount,     // this MUST be last
        };    

      COleVariant args [eArgCount];
      DISPPARAMS params = { args, NULL, eArgCount, 0 };

      args [eTagName] = strTag;
      args [eText] = strText;

      ExecuteScript (m_dispidOnMXP_CloseTag,  
                     m_strOnMXP_CloseTag,
                     eWorldAction,
                     strType, 
                     strReason,
                     params, 
                     nInvocationCount); 
      } // not Lua
    }  // end of script callback wanted

  // tell each plugin about the close tag
  if (m_bPluginProcessesCloseTag)
    for (POSITION pluginpos = m_PluginList.GetHeadPosition(); pluginpos; )
      {
      CPlugin * pPlugin = m_PluginList.GetNext (pluginpos);

      if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
        continue;

      // see what the plugin makes of this,
      pPlugin->ExecutePluginScript (ON_PLUGIN_MXP_CLOSETAG, 
                            pPlugin->m_dispid_plugin_OnMXP_CloseTag, 
                            CFormat ("%s,%s",
                            (LPCTSTR) strTag,
                            (LPCTSTR) strText)
                            );
      }   // end of doing each plugin
  m_CurrentPlugin = NULL;

  // if this tag had a FLAG directive, set the desired variable - prefixed with mxp_
  if (bHaveVariable)
    {
    CVariable * variable_item;

    // get rid of old variable, if any
    if (m_VariableMap.Lookup (strVariable, variable_item))
      delete variable_item;

    // create new variable item and insert in variable map
    m_VariableMap.SetAt (strVariable, variable_item = new CVariable);
    m_bVariablesChanged = true;
//    SetModifiedFlag (TRUE);
    variable_item->nUpdateNumber = App.GetUniqueNumber ();   // for concurrency checks

    // if auto-mapping, add to auto-map string

    if (m_bMapping && 
        strVariable == "mxp_roomname" &&
        !strText.IsEmpty ())
      {
      CString str = "{";
      CString strTemp = strText;
      strTemp.Replace ('{', '(');  // ensure no comment delimiters in room name
      strTemp.Replace ('}', ')');
      str += strTemp;
      str += "}";

      m_strMapList.AddTail (str);
	    
      // update status line
      DrawMappingStatusLine ();
      }

    // set up variable item contents
    variable_item->strLabel = strVariable;
    variable_item->strContents = strText;

    // call script if required
    if (m_dispidOnMXP_SetVariable != DISPID_UNKNOWN)
      {
      long nInvocationCount = 0;

      CString strType = "MXP set variable";
      CString strReason =  TFormat ("setting MXP variable %s", (LPCTSTR) strVariable);

      if (GetScriptEngine () && GetScriptEngine ()->IsLua ())
        {
        list<double> nparams;
        list<string> sparams;
        sparams.push_back ((LPCTSTR) strVariable);
        sparams.push_back ((LPCTSTR) strText);
        GetScriptEngine ()->ExecuteLua (m_dispidOnMXP_SetVariable, 
                                       m_strOnMXP_SetVariable, 
                                       eWorldAction,
                                       strType, 
                                       strReason, 
                                       nparams,
                                       sparams, 
                                       nInvocationCount); 
        }   // end of Lua
      else
        {
        // WARNING - arguments should appear in REVERSE order to what the sub expects them!

        enum
          {
          eContents,
          eVariableName,
          eArgCount,     // this MUST be last
          };    

        COleVariant args [eArgCount];
        DISPPARAMS params = { args, NULL, eArgCount, 0 };

        args [eVariableName] = strVariable;
        args [eContents] = strText;

        ExecuteScript (m_dispidOnMXP_SetVariable,  
                       m_strOnMXP_SetVariable,
                       eWorldAction,
                       strType, 
                       strReason,
                       params, 
                       nInvocationCount); 
        } // not Lua
      }  // end of script callback wanted


    // tell each plugin what we have received
    if (m_bPluginProcessesSetVariable)
      for (POSITION pluginpos = m_PluginList.GetHeadPosition(); pluginpos; )
        {
        CPlugin * pPlugin = m_PluginList.GetNext (pluginpos);

        if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
          continue;

        // see what the plugin makes of this,
        pPlugin->ExecutePluginScript (ON_PLUGIN_MXP_SETVARIABLE, 
                              pPlugin->m_dispid_plugin_OnMXP_SetVariable, 
                              CFormat ("%s=%s",
                              (LPCTSTR) strVariable,
                              (LPCTSTR) strText)
                              );
        }   // end of doing each plugin
    m_CurrentPlugin = NULL;
      
    if (strTag == "var"|| strTag == "v") // add entity to map
      {
      m_CustomEntityMap.SetAt (strVariable, strText);

      // tell each plugin what we have received
      if (m_bPluginProcessesSetEntity)
        for (POSITION pluginpos = m_PluginList.GetHeadPosition(); pluginpos; )
          {
          CPlugin * pPlugin = m_PluginList.GetNext (pluginpos);

          if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
            continue;

          // see what the plugin makes of this,
          pPlugin->ExecutePluginScript (ON_PLUGIN_MXP_SETENTITY, 
                                pPlugin->m_dispid_plugin_OnMXP_SetEntity, 
                                CFormat ("%s=%s",
                                (LPCTSTR) strVariable,
                                (LPCTSTR) strText)
                                );
          }   // end of doing each plugin
      m_CurrentPlugin = NULL;

      }

    } // end of setting the variable's contents

// see if we know of this element

CAtomicElement * pAtomicElement;
  
  // atomic element?
  if (App.m_ElementMap.Lookup (strTag, pAtomicElement))
    MXP_CloseAtomicTag (pAtomicElement->iAction, 
                        strText,
                        oldlinepos,
                        oldstylepos);
  else
    {
    CElement * pElement;

    // custom element?
    if (!m_CustomElementMap.Lookup (strTag, pElement))
      {
        MXP_error (DBG_ERROR, errMXP_ClosingUnknownTag,
                  TFormat ("Unknown MXP element: <%s>" ,
                          (LPCTSTR) m_strMXPstring));
      return;
      }

    CElementItem * pElementItem;

    for (POSITION pos = pElement->ElementItemList.GetHeadPosition (); pos; )
      {
      pElementItem = pElement->ElementItemList.GetNext (pos);
      MXP_CloseAtomicTag (pElementItem->pAtomicElement->iAction, 
                          strText,
                          oldlinepos,
                          oldstylepos);
      } // end of doing each one
    } // end of user-defined element

  // I don't need the opening style tag any more, and as it remembers the style at
  // the moment it was made it will make a good way of restoring existing styles 
  // eg. underline off 

  pLine->styleList.RemoveAt (oldstylepos);

  // however if the last thing on the current line is a zero-length style,
  // we may as well get rid of it.

  CStyle * pLastStyle = m_pCurrentLine->styleList.GetTail ();

  if (pLastStyle->iLength == 0 && (pLastStyle->iFlags & START_TAG) == 0)
    {
    DELETESTYLE (pLastStyle);
    m_pCurrentLine->styleList.RemoveTail ();
    }

  m_pCurrentLine->styleList.AddTail (pStyle);

  pStyle->iFlags &= ~START_TAG;   // isn't a start tag any more
  pStyle->pAction->Release ();     // get rid of style name
  pStyle->pAction = NULL;

  RememberStyle (pStyle);

  } // end of  CMUSHclientDoc::MXP_CloseTag



void CMUSHclientDoc::MXP_CloseOpenTags (void)
  {

// see if we know of this element

  while (!m_ActiveTagList.IsEmpty ())
    {
    CActiveTag * pTag = m_ActiveTagList.GetTail ();

    // don't close securely-opened tags here
    if (pTag->bSecure)
      return;

    CString strTag = pTag->strName;

    MXP_error (DBG_WARNING, wrnMXP_OpenTagClosedAtEndOfLine,
              TFormat ("End-of-line closure of open MXP tag: <%s>", 
              (LPCTSTR) strTag)); 
    MXP_CloseTag (strTag);

    m_ActiveTagList.RemoveTail ();  // get rid of it
    delete pTag;
    }

  }  // end of CMUSHclientDoc::MXP_CloseOpenTags

void CMUSHclientDoc::MXP_CloseAllTags (void)
  {
  while (!m_ActiveTagList.IsEmpty ())
    {
    CActiveTag * pTag = m_ActiveTagList.GetTail ();

    // if protected from reset, stop closing
    if (pTag->bNoReset)
      return;

    MXP_error (DBG_WARNING, wrnMXP_TagClosedAtReset,
              TFormat ("<reset> closure of MXP tag: <%s>", 
              (LPCTSTR) pTag->strName)); 
    MXP_CloseTag (pTag->strName);
    m_ActiveTagList.RemoveTail ();
    delete pTag;
    }
  } // end of CMUSHclientDoc::MXP_CloseAllTags

