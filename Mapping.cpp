#include "stdafx.h"
#include "MUSHclient.h"

#include "doc.h"
#include "dialogs\MapDlg.h"
#include "dialogs\MapMoveDlg.h"
#include "dialogs\MapCommentDlg.h"
#include "mainfrm.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CMapDirectionsMap MapDirectionsMap;

void LoadMapDirections (void)
  {
  //-------------------------------------------------------------------
  //                direction                 log        full     reverse
                                                        
  MapDirectionsMap ["north"]  = CMapDirection ("n",     "north",  "s");
  MapDirectionsMap ["south"]  = CMapDirection ("s",     "south",  "n");
  MapDirectionsMap ["east"]   = CMapDirection ("e",     "east",   "w");
  MapDirectionsMap ["west"]   = CMapDirection ("w",     "west",   "e");
  MapDirectionsMap ["up"]     = CMapDirection ("u",     "up",     "d");
  MapDirectionsMap ["down"]   = CMapDirection ("d",     "down",   "u");
  MapDirectionsMap ["ne"]     = CMapDirection ("ne",    "ne",     "sw");
  MapDirectionsMap ["nw"]     = CMapDirection ("nw",    "nw",     "se");
  MapDirectionsMap ["se"]     = CMapDirection ("se",    "se",     "nw");
  MapDirectionsMap ["sw"]     = CMapDirection ("sw",    "sw",     "ne");

  // filler
  MapDirectionsMap ["f"]      = CMapDirection ("f",     "f",      "f");

  // now do abbreviations (copy above entries)
  MapDirectionsMap ["n"]      = MapDirectionsMap ["north"];
  MapDirectionsMap ["s"]      = MapDirectionsMap ["south"];
  MapDirectionsMap ["e"]      = MapDirectionsMap ["east"];
  MapDirectionsMap ["w"]      = MapDirectionsMap ["west"];
  MapDirectionsMap ["u"]      = MapDirectionsMap ["up"];
  MapDirectionsMap ["d"]      = MapDirectionsMap ["down"];

  } // end of LoadMapDirections


void CMUSHclientDoc::OnGameMapper() 
{
CMapDlg dlg;


  dlg.m_bEnable = m_bMapping;	
  dlg.m_bRemoveMapReverses = m_bRemoveMapReverses;	
  dlg.m_pDoc = this;
  dlg.m_bMapFailureRegexp = m_bMapFailureRegexp;
  dlg.m_strMappingFailure = m_strMappingFailure;

  // if they open the mapper with no path, assume they want to start one
  if (m_strMapList.IsEmpty ())
    dlg.m_bEnable = true;

  // what I am worried about here is that they will do a special move while
  // the map dialog is open, and that the error message won't be detected,
  // thus, I will force mapping on while we look at the dialog, so that any
  // moves made within it are removed from the list if they cause an error

  bool bSave = m_bMapping;
  
  m_bMapping = true;
  if (dlg.DoModal () != IDOK)
    {
    m_bMapping = bSave;

    // check for the case where they open the map dialog box, see enabled is checked
    // then press Cancel. This would in fact not enable it, so we should warn them.

    if (!m_bMapping && m_strMapList.IsEmpty ())
      if (::TMessageBox ("Warning - mapping has not been turned on because you pressed 'Cancel'.\n\n"
                       "Do you want mapping enabled now?",  MB_ICONQUESTION | MB_YESNO) == IDYES)
         m_bMapping = true;

    return;
    }

  m_bMapping = dlg.m_bEnable;	
  m_bRemoveMapReverses = dlg.m_bRemoveMapReverses;	

  // document is modified if they have changed these
  if (m_bMapFailureRegexp  != dlg.m_bMapFailureRegexp ||
      m_strMappingFailure != dlg.m_strMappingFailure)
        SetModifiedFlag ();

  m_bMapFailureRegexp  = dlg.m_bMapFailureRegexp;
  m_strMappingFailure = dlg.m_strMappingFailure;

  if (m_bMapFailureRegexp && !m_strMappingFailure.IsEmpty ())
    {
    try
      {
      m_MapFailureRegexp = new t_regexp (m_strMappingFailure, (m_bUTF_8 ? PCRE_UTF8 : 0));
      }
    catch (CException* e)
      {
      char sMessage [1000];
      e->GetErrorMessage (sMessage, sizeof sMessage);
      ::UMessageBox (TFormat ("Error \"%s\" processing mapping failure regular expression \"%s\"", 
                       sMessage,
                       (LPCTSTR) m_strMappingFailure));
      e->Delete ();
      m_bMapFailureRegexp = FALSE; 
      m_MapFailureRegexp = NULL;
      } // end of catch

    } // end compiling regular expression

}   // end of CMUSHclientDoc::OnGameMapper

void CMUSHclientDoc::OnUpdateGameMapper(CCmdUI* pCmdUI) 
{
  DoFixMenus (pCmdUI);  // remove accelerators from menus
  pCmdUI->Enable ();
  pCmdUI->SetCheck (m_bMapping);	
}   // end of CMUSHclientDoc::OnUpdateGameMapper


void CMUSHclientDoc::AddToMap (CString str)
  {
  MapDirectionsIterator i;

  str.MakeLower ();
  str.TrimLeft ();

  int iNewline;

  while ((iNewline = str.Find (ENDLINE)) != -1)
    {
    CString strDirection = str.Left (iNewline);

    // see if they have sent a "direction"
    i = MapDirectionsMap.find ((LPCTSTR) strDirection);

    // if not, give up
    if (i != MapDirectionsMap.end ())
      {

      // Remove reverses ...

      // If they have gone, for example, N then S, then the net result 
      // is that they haven't gone anywhere, so remove the N rather than adding the S
      
      // We let them disable this because sometimes rooms are not symmetrical, and
      // you might go North, and then South, and *not* end up where you were.


      if (m_bRemoveMapReverses &&
          !m_strMapList.IsEmpty ())
        {
        CString strComment = m_strMapList.GetTail ();

        // if last thing was comment get it and try second last thing
        if (strComment.GetLength () >= 2 &&
            strComment.Left (1) == "{" &&
            strComment.Right (1) == "}" &&
            m_strMapList.GetCount () > 1)
              m_strMapList.RemoveTail ();
        else
          strComment.Empty ();
        
        // if this direction had a non-standard reverse, extract that
        CString strLast = m_strMapList.GetTail ();

        int iSlashPos = strLast.Find ("/");
        if (iSlashPos != -1)
          {
          strLast = strLast.Mid (iSlashPos + 1);  // this is the reverse direction
        
          MapDirectionsIterator i2;

          i2 = MapDirectionsMap.find ((LPCTSTR) strLast);   // convert "west" to "w"

          // if this is the reverse direction, remove the earlier one rather than
          // adding this one
          if (i2 != MapDirectionsMap.end () && 
              i2->second.m_sDirectionToLog == i->second.m_sDirectionToLog)
           m_strMapList.RemoveTail ();
          else
            m_strMapList.AddTail (i->second.m_sDirectionToLog.c_str ());
          }
         else if (m_strMapList.GetTail () == i->second.m_sReverseDirection.c_str ())
             m_strMapList.RemoveTail ();
         else
           {
           // different direction? Put comment back if we had one
           if (!strComment.IsEmpty ())
            m_strMapList.AddTail (strComment);
           // and add the new direction
           m_strMapList.AddTail (i->second.m_sDirectionToLog.c_str ());
           }
        }
      else
        m_strMapList.AddTail (i->second.m_sDirectionToLog.c_str ());

      // update status line
      DrawMappingStatusLine ();
      } // end of finding the direction

    str = str.Mid (iNewline + strlen (ENDLINE));
    }   // end of breaking the text into lines

  }   // end of CMUSHclientDoc::AddToMap



void CMUSHclientDoc::OnGameDomapperspecial() 
{
CMapMoveDlg dlg;

  dlg.m_bSendToMUD = true;
  dlg.m_strAction  =  m_strSpecialForwards;
  dlg.m_strReverse =  m_strSpecialBackwards;
  
  if (dlg.DoModal () != IDOK)
    return;

  // remember it in case they misspelt it and want to try again
  m_strSpecialForwards  = dlg.m_strAction;     
  m_strSpecialBackwards = dlg.m_strReverse;

  if (dlg.m_bSendToMUD && !dlg.m_strAction.IsEmpty ())
    {
    // don't log it twice (eg. if they enter "ne")
    bool bSave = m_bMapping;
    m_bMapping = false;
    SendMsg (dlg.m_strAction, m_display_my_input, false, LoggingInput ());
    m_bMapping = bSave;
    }

  CString strDirection;

  strDirection = dlg.m_strAction + "/" + dlg.m_strReverse;

  m_strMapList.AddTail (strDirection);
	
  // update status line
  DrawMappingStatusLine ();

}

void CMUSHclientDoc::OnUpdateGameDomapperspecial(CCmdUI* pCmdUI) 
{
  DoFixMenus (pCmdUI);  // remove accelerators from menus
  pCmdUI->Enable (m_bMapping);	
	
}


void CMUSHclientDoc::OnGameDomappercomment() 
{
CMapCommentDlg dlg;
  
  if (dlg.DoModal () != IDOK)
    return;

  CString str = "{";
  str += dlg.m_strComment;
  str += "}";

  m_strMapList.AddTail (str);
	
  // update status line
  DrawMappingStatusLine ();
	
}

void CMUSHclientDoc::OnUpdateGameDomappercomment(CCmdUI* pCmdUI) 
{
  DoFixMenus (pCmdUI);  // remove accelerators from menus
  pCmdUI->Enable (m_bMapping);	
}

#define OUTPUT_PREVIOUS_ONE  \
do { \
  if (strLastDir.GetLength () > 1)  \
    strLastDir = "(" + strLastDir + ")";  \
  if (iCount == 1)  \
    strForwards += strLastDir + " ";  \
  else if (iCount > 1)  \
    strForwards += CFormat ("%i%s ", iCount, (LPCTSTR) strLastDir);  \
  strLastDir = str;  \
  iCount = 1;  \
  } while (false)

CString CMUSHclientDoc::CalculateSpeedWalkString (const bool bOmitComments)
  {

  CString str;
  CString strLastDir;
  CString strForwards;

  int iCount = 0;

  for (POSITION pos = m_strMapList.GetHeadPosition (); pos; )
    {
    // get next direction from list
    str = m_strMapList.GetNext (pos);

    if (str.GetLength () > 2 &&
        str.Left (1) == "{" &&
        str.Right (1) == "}")
      {
      OUTPUT_PREVIOUS_ONE;    // output directions up to comment
      if (!bOmitComments)
        {
        if (!strForwards.IsEmpty ())
          strForwards += ENDLINE;  // put comments on their own lines
        strForwards += str;
        strForwards += ENDLINE;
        }
      strLastDir.Empty ();
      iCount = 0;
      }
    else    // not comment
      // if same as before, count them
      if (str == strLastDir && iCount <= 98)
        iCount++;
      else
        OUTPUT_PREVIOUS_ONE;


    }   // end of processing each direction

  // output final one
  OUTPUT_PREVIOUS_ONE;

  return strForwards;
  }  // end of CMUSHclientDoc::CalculateSpeedWalkString 


void CMUSHclientDoc::DrawMappingStatusLine (void)
  {
  CString strSpeedWalk = CalculateSpeedWalkString (true); // omit comments
  if (!strSpeedWalk.IsEmpty ())
    {
    if (strSpeedWalk.GetLength () > 50)
      strSpeedWalk = strSpeedWalk.Left (50) + " ...";
    Frame.SetStatusMessageNow (TFormat ("Mapper: %s", (LPCTSTR) strSpeedWalk));
    m_tStatusDisplayed = CTime::GetCurrentTime ();
    m_bShowingMapperStatus = true;
    }
  }   // end of CMUSHclientDoc::DrawMappingStatusLine 

