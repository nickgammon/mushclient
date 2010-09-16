#include "stdafx.h"
#include "MUSHclient.h"
#include "doc.h"
#include "mxp\mxp.h"
#include "sendvw.h"
#include "Color.h"
#include "childfrm.h"
#include "MUSHview.h"
#include "TextDocument.h"

#include "png/png.h"  // for version

// my debugging

// memory state tracking
#ifdef _DEBUG
extern CMemoryState oldMemState, newMemState, diffMemState;
#endif

extern tConfigurationNumericOption OptionsTable [];
extern tConfigurationAlphaOption AlphaOptionsTable [];
extern tCommandIDMapping CommandIDs [];
extern CString strMacroDescriptions [MACRO_COUNT];
extern CString strKeypadNames [eKeypad_Max_Items];
extern tInfoTypeMapping InfoTypes [];

#define SHOW_TRUE(x) ((x) ? "yes" : "NO")
#define SHOW_TRUE_LC(x) ((x) ? "yes" : "no")

static char * sColourNames [8] = 
  {
  "Black",
  "Red",
  "Green",
  "Yellow",
  "Blue",
  "Magenta",
  "Cyan",
  "White"
   };

static char * sMxpModes [] = 
  {
  "On command",
  "On query",
  "Yes - always",
  "No - never"
  };


// compare-less for colours
struct colour_less : binary_function<COLORREF, COLORREF, bool>
  {

  bool operator() (const COLORREF & c1, const COLORREF & c2) const
    {
    CColor clr1, clr2;

    clr1.SetColor (c1);
    clr2.SetColor (c2);

    // compare hue

    float hue1 = clr1.GetHue ();
    float hue2 = clr2.GetHue ();

    if (hue1 < hue2)
      return true;
    else if (hue1 > hue2)
      return false;

    // hue the same, compare saturation

    float saturation1 = clr1.GetSaturation ();
    float saturation2 = clr2.GetSaturation ();
    if (saturation1 < saturation2)
      return true;
    else if (saturation1 > saturation2)
      return false;

    // saturation the same - compare luminance

    float luminance1 = clr1.GetLuminance ();
    float luminance2 = clr2.GetLuminance ();
    if (luminance1 < luminance2)
      return true;

    return false;

    }   // end of operator()
  }; // end of colour_less

static void ShowOneColour (CMUSHclientDoc * pDoc, 
                           CString strColourName,
                           const COLORREF iColour)

  {
  int iRed    = GetRValue (iColour);
  int iGreen  = GetGValue (iColour);
  int iBlue   = GetBValue (iColour);

  CString strName = CFormat ("%-24s ", (LPCTSTR) strColourName);

  CString strTextColour = "black";

   // if colour is dark, use white, otherwise use black
   if (((GetRValue (iColour) & 0xFF) +
       (GetGValue (iColour) & 0xFF) +
       (GetBValue (iColour) & 0xFF) ) < (128 * 3))
     strTextColour = "white";

  pDoc->ColourTell (strTextColour, ColourToName (iColour), strName);

  strName = CFormat (" R=%3i G=%3i B=%3i ",     // RGB
                     iRed,
                     iGreen,
                     iBlue);

  strName += CFormat ("#%02X%02X%02X ",     // MXP
                     iRed,
                     iGreen,
                     iBlue);

  strName += CFormat ("&h%02X%02X%02X ",    // VB
                     iBlue,
                     iGreen,
                     iRed);

  strName += CFormat ("0x%02X%02X%02X ",     // Jscript
                     iBlue,
                     iGreen,
                     iRed);

  strName += CFormat ("%08ld", iColour);     // Lua

  pDoc->Note (strName);

  } // end of ShowOneColour

static void ShowAnsiColour (CMUSHclientDoc * pDoc, 
                           CString strColourName,
                           const COLORREF iColour)

  {
  int iRed    = GetRValue (iColour);
  int iGreen  = GetGValue (iColour);
  int iBlue   = GetBValue (iColour);

  CString strName = CFormat ("%-24s ", (LPCTSTR) strColourName);

  CString strTextColour = "black";

   // if colour is dark, use white, otherwise use black
   if (((iRed & 0xFF) + (iGreen & 0xFF) + (iBlue & 0xFF) ) < (128 * 3))
     strTextColour = "white";

  pDoc->ColourTell (strTextColour, ColourToName (iColour), strName);
  pDoc->ColourTell (ColourToName (iColour), ColourToName (pDoc->m_normalcolour [0]), strName);
  pDoc->ColourTell (ColourToName (pDoc->m_normalcolour [7]), ColourToName (iColour),  strName);

  pDoc->Note ("");

  } // end of ShowAnsiColour


VARIANT CMUSHclientDoc::Debug(LPCTSTR Command) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);
  int iCount = 0;
  
#ifdef _DEBUG
//-----------------------------------------------------------------------
//          memdiff
//-----------------------------------------------------------------------
  if (strcmp (Command, "memdiff") == 0)
    {
    newMemState.Checkpoint();
    if (diffMemState.Difference(oldMemState, newMemState))
      {
      TRACE("Memory leaked.\n");
      diffMemState.DumpStatistics();
//      oldMemState.DumpAllObjectsSince ();
      }
    else
      TRACE("No memory leak.\n");
    }
//-----------------------------------------------------------------------
//          memchk
//-----------------------------------------------------------------------
   if (strcmp (Command, "memchk") == 0)
    {
    TRACE("Memory checkpoint taken.\n");
    oldMemState.Checkpoint();
    }
  else
#endif

//-----------------------------------------------------------------------
//          colours
//-----------------------------------------------------------------------
  if (strcmp (Command, "colours") == 0 || strcmp (Command, "colors") == 0)
    {

    multimap<COLORREF, string, colour_less> mColours;

    for (POSITION pos = App.m_ColoursMap.GetStartPosition(); pos; iCount++)
      {
      CColours * pColour;
      CString strColourName;
      App.m_ColoursMap.GetNextAssoc (pos, strColourName, pColour);
      mColours.insert (make_pair (pColour->iColour, strColourName));
      }

    for (multimap<COLORREF, string, colour_less>::const_iterator it = mColours.begin ();
         it != mColours.end ();
         it++)

      ShowOneColour (this, it->second.c_str (), it->first);
     
    Note (TFormat ("%i colour%s.", PLURAL (iCount)));

    } // end of colours

//-----------------------------------------------------------------------
//          colours256
//-----------------------------------------------------------------------
  else if (strcmp (Command, "colours256") == 0 || strcmp (Command, "colors256") == 0)
    {

    for (int i = 0; i < 256; i++)
      {
      CString strName;

      // see if we can find colour name in list
      for (POSITION pos = App.m_ColoursMap.GetStartPosition(); pos; )
        {

        CColours * pColour;
        CString strColourName;

        App.m_ColoursMap.GetNextAssoc (pos, strColourName, pColour);

        // note - colour might match more than one name
        if (pColour->iColour == xterm_256_colours [i])
          {
          if (!strName.IsEmpty ())
            strName += ", ";
          else 
            strName = " (";
          strName += strColourName;
          }
        } // end of loop

      if (!strName.IsEmpty ())
        strName += ")";

      ShowOneColour (this, CFormat ("%i%s", i, (LPCTSTR) strName), xterm_256_colours [i]);
      } // end of loop

    } // end of colours256
  
//-----------------------------------------------------------------------
//          ansi
//-----------------------------------------------------------------------
  else if (strcmp (Command, "ansi") == 0)
    {

    int i;

    Note (TFormat ("%-24s %-24s %-24s", " Name", " On normal background", " Under normal text"));
    Note ("------------------------ ------------------------ ------------------------ ");

    for (i = 0; i < 8; i++)
      {
      CString strName = TFormat (" Normal #%i (%s)", i + 1, sColourNames [i]);
      ShowAnsiColour (this, strName, m_normalcolour [i]);
      }   // end for loop

    for (i = 0; i < 8; i++)
      {
      CString strName = TFormat (" Bold   #%i (%s)", i + 1, sColourNames [i]);
      ShowAnsiColour (this, strName, m_boldcolour [i]);
      }   // end for loop

    } // end of ansi

//-----------------------------------------------------------------------
//          custom_colour
//-----------------------------------------------------------------------
  else if (strcmp (Command, "custom_colours") == 0 || strcmp (Command, "custom_colors") == 0)
    {
    int i;
    for (i = 0; i < MAX_CUSTOM; i++)
      {
      CString strName = TFormat (" Custom #%2i (%s)", i + 1, m_strCustomColourName [i]);
      ColourNote ( ColourToName (m_customtext [i]), ColourToName (m_customback [i]), strName);
      }     // end for loop
    } // end of custom_colour

//-----------------------------------------------------------------------
//          entities
//-----------------------------------------------------------------------
  else if (strcmp (Command, "entities") == 0)
    {

    for (POSITION pos = App.m_EntityMap.GetStartPosition(); pos; iCount++)
      {
      CString strEntityName;
      CString strEntityValue;
      App.m_EntityMap.GetNextAssoc (pos, strEntityName, strEntityValue);

      CString strName = CFormat ("%-10s = %s", 
                                (LPCTSTR) strEntityName,
                                (LPCTSTR) strEntityValue);


      Note (strName);
      }

    Note (TFormat ("%i entit%s.", PLURALIE (iCount)));

    } // end of entities
//-----------------------------------------------------------------------
//          server_entities
//-----------------------------------------------------------------------
  else if (strcmp (Command, "server_entities") == 0)
    {

    for (POSITION pos = m_CustomEntityMap.GetStartPosition(); pos; iCount++)
      {
      CString strEntityName;
      CString strEntityValue;
      m_CustomEntityMap.GetNextAssoc (pos, strEntityName, strEntityValue);

      CString strName = CFormat ("%-10s = %s", 
                                (LPCTSTR) strEntityName,
                                (LPCTSTR) strEntityValue);


      Note (strName);
      }

    Note (TFormat ("%i server entit%s.", PLURALIE (iCount)));

    } // end of server_entities
//-----------------------------------------------------------------------
//          elements
//-----------------------------------------------------------------------
  else if (strcmp (Command, "elements") == 0)
    {

    for (POSITION pos = App.m_ElementMap.GetStartPosition(); pos; iCount++)
      {
      CAtomicElement * pElement;
      CString strElementName;
      App.m_ElementMap.GetNextAssoc (pos, strElementName, pElement);

      CString strName = CFormat ("%-25s %3i", (LPCTSTR) strElementName,
                                pElement->iAction);

      if (pElement->iFlags & TAG_OPEN)
        strName += " open";
      if (pElement->iFlags & TAG_COMMAND)
        strName += " empty";
      if (pElement->iFlags & TAG_NO_RESET)
        strName += " no_reset";
      if (pElement->iFlags & TAG_NOT_IMP)
        strName += " not_imp";

      if (!pElement->strArgs.IsEmpty ())
        strName += CFormat (" (%s)", (LPCTSTR) pElement->strArgs);

      Note (strName);
      }

    Note (TFormat ("%i element%s.", PLURAL (iCount)));

    } // end of elements
//-----------------------------------------------------------------------
//          server_elements
//-----------------------------------------------------------------------
  else if (strcmp (Command, "server_elements") == 0)
    {

    for (POSITION pos = m_CustomElementMap.GetStartPosition(); pos; iCount++)
      {
      CElement * pElement;
      CString strElementName;
      m_CustomElementMap.GetNextAssoc (pos, strElementName, pElement);

      CString strName = CFormat ("%s", (LPCTSTR) strElementName);

      if (pElement->bOpen)
        strName += " open";
      if (pElement->bCommand)
        strName += " empty";
      if (pElement->iTag)
        strName += CFormat (" tag=%i", pElement->iTag);
      if (!pElement->strFlag.IsEmpty ())
        strName += CFormat (" flag=%s", (LPCTSTR) pElement->strFlag);

      Note (strName);

      // list the attributes

      if (!pElement->AttributeList.IsEmpty ())
        {
        Note ("  Attributes:");

        for (POSITION attpos = pElement->AttributeList.GetHeadPosition (); attpos; )
          {
          CArgument * pArgument = pElement->AttributeList.GetNext (attpos);
      
          if (pArgument->strName.IsEmpty ())
            Note (CFormat ("    %s=''",
                          (LPCTSTR) pArgument->strValue));
          else
            Note (CFormat ("    %s='%s'",
                          (LPCTSTR) pArgument->strName,
                          (LPCTSTR) pArgument->strValue));

          }      // end of looping through each attribute
        } // end of having attributes
  

      for (POSITION elementpos = pElement->ElementItemList.GetHeadPosition (); elementpos; )
        {
        CElementItem * pItem = pElement->ElementItemList.GetNext (elementpos);
    
        CString strName = CFormat ("  <%s", (LPCTSTR) pItem->pAtomicElement->strName);

        for (POSITION attpos = pElement->AttributeList.GetHeadPosition (); attpos; )
          {
          CArgument * pArgument = pElement->AttributeList.GetNext (attpos);
      
          // empty ones we will put there by position
          if (pArgument->strName.IsEmpty ())
            strName += CFormat (" %s",
                          (LPCTSTR) pArgument->strValue);
          else
            strName +=  CFormat (" %s='%s'",
                          (LPCTSTR) pArgument->strName,
                          (LPCTSTR) pArgument->strValue);

          }      // end of looping through each attribute
        strName += ">";
        Note (strName);
        }      // end of looping through each atomic element


      Note ("");    // blank line between each one
      }

    Note (TFormat ("%i server element%s.", PLURAL (iCount)));

    } // end of server_elements
//-----------------------------------------------------------------------
//          actions
//-----------------------------------------------------------------------
  else if (strcmp (Command, "actions") == 0)
    {

    for (POSITION pos = m_ActionList.GetHeadPosition(); pos; iCount++)
      {
      CAction * pAction = m_ActionList.GetNext (pos);

      Note (CFormat ("Item %i, Count: %i", iCount + 1, pAction->GetReferenceCount ()));
      Note (CFormat ("Send: %s", (LPCTSTR) pAction->m_strAction));
      if (!pAction->m_strHint.IsEmpty ())
        Note (CFormat ("Hint: %s", (LPCTSTR) pAction->m_strHint));
      if (!pAction->m_strVariable.IsEmpty ())
        Note (CFormat ("Var : %s", (LPCTSTR) pAction->m_strVariable));


      Note ("");  // blank line
      }

    Note (TFormat ("%i action%s.", PLURAL (iCount)));

    } // end of actions
//-----------------------------------------------------------------------
//          commands
//-----------------------------------------------------------------------
  else if (strcmp (Command, "commands") == 0)
    {

    int iViewNumber = 0;

    for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	    {
	    CView* pView = GetNextView(pos);
	    
	    if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
  	    {
		    CSendView* pmyView = (CSendView*)pView;

        // show a message if we are showing multiple non-empty views
        if (!pmyView->m_msgList.IsEmpty ())
          if (++iViewNumber > 1)
            {
            Note ("");
            Note (TFormat ("--- Command Window %i ---", iViewNumber));
            Note ("");
            }
        for (POSITION pos = pmyView->m_msgList.GetHeadPosition(); pos; iCount++)
          Note (pmyView->m_msgList.GetNext (pos));

	      }	  // end of being a CSendView
      }   // end of loop through views


    Note (TFormat ("%i command%s.", PLURAL (iCount)));

    } // end of commands
//-----------------------------------------------------------------------
//          aliases
//-----------------------------------------------------------------------
  else if (strcmp (Command, "aliases") == 0)
    {

    CAlias * pAlias;
    CString strAliasName;

    for (POSITION pos = GetAliasMap ().GetStartPosition(); pos; iCount++)
       {
       GetAliasMap ().GetNextAssoc (pos, strAliasName, pAlias);
       CString strName = CFormat ("{{%s}}={{%s}}",
                                  (LPCTSTR) pAlias->name,
                                  (LPCTSTR) pAlias->contents);

       if (!pAlias->strLabel.IsEmpty ())
         strName += CFormat (" label=%s", (LPCTSTR) pAlias->strLabel);
       if (!pAlias->strProcedure.IsEmpty ())
         strName += CFormat (" script=%s", (LPCTSTR) pAlias->strProcedure);
       if (!pAlias->strGroup.IsEmpty ())
         strName += CFormat (" group=%s", (LPCTSTR) pAlias->strGroup);
       if (!pAlias->strVariable.IsEmpty ())
         strName += CFormat (" variable=%s", (LPCTSTR) pAlias->strVariable);
       
       if (!pAlias->bEnabled)
         strName += " disabled";
       if (pAlias->bExpandVariables)
         strName += " expand_variables";
       if (pAlias->bOmitFromLog)
         strName += " omit_log";
       if (pAlias->bOmitFromOutput)
         strName += " omit_output";
       if (pAlias->bRegexp)
         strName += " regexp";
       if (pAlias->bOneShot)
         strName += " one_shot";
       if (pAlias->bIgnoreCase)
         strName += " ignore_case";
       if (pAlias->bMenu)
         strName += " menu";
       if (pAlias->bTemporary)
         strName += " temporary";
       if (pAlias->bIncluded)
         strName += " included";
//       if (pAlias->bDelayed)
//         strName += " delayed";
       strName += CFormat (" send_to:%s", (LPCTSTR) GetSendToString (pAlias->iSendTo));
       strName += CFormat (" seq:%i", pAlias->iSequence);

       Note (strName);
       }


    Note (TFormat ("%i alias%s.", PLURALES (iCount)));

    } // end of aliases
//-----------------------------------------------------------------------
//          triggers
//-----------------------------------------------------------------------
  else if (strcmp (Command, "triggers") == 0)
    {

    CTrigger * pTrigger;
    CString strTriggerName;

    for (POSITION pos = GetTriggerMap ().GetStartPosition(); pos; iCount++)
       {
       GetTriggerMap ().GetNextAssoc (pos, strTriggerName, pTrigger);
       CString strName = CFormat ("{{%s}}={{%s}}",
                                  (LPCTSTR) pTrigger->trigger,
                                  (LPCTSTR) pTrigger->contents);

       if (!pTrigger->strLabel.IsEmpty ())
         strName += CFormat (" label=%s", (LPCTSTR) pTrigger->strLabel);
       if (!pTrigger->strProcedure.IsEmpty ())
         strName += CFormat (" script=%s", (LPCTSTR) pTrigger->strProcedure);
       if (!pTrigger->strGroup.IsEmpty ())
         strName += CFormat (" group=%s", (LPCTSTR) pTrigger->strGroup);
       if (!pTrigger->strVariable.IsEmpty ())
         strName += CFormat (" variable=%s", (LPCTSTR) pTrigger->strVariable);
#ifdef PANE
       if (!pTrigger->strPane.IsEmpty ())
         strName += CFormat (" pane=%s", (LPCTSTR) pTrigger->strPane);
#endif // PANE
       if (!pTrigger->sound_to_play.IsEmpty ())
         strName += CFormat (" sound=%s", (LPCTSTR) pTrigger->sound_to_play);
       if (pTrigger->bSoundIfInactive)
         strName += " sound_if_inactive";
       if (pTrigger->bLowercaseWildcard)
         strName += " lowercase_wildcard";
      
       if (pTrigger->colour != SAMECOLOUR)
         strName += CFormat (" colour=%i", pTrigger->colour + 1); // make 1-relative

       if (!pTrigger->bEnabled)
         strName += " disabled";
       if (pTrigger->bExpandVariables)
         strName += " expand_variables";
       if (pTrigger->omit_from_log)
         strName += " omit_log";
       if (pTrigger->bOmitFromOutput)
         strName += " omit_output";
       if (pTrigger->bRegexp)
         strName += " regexp";
       if (pTrigger->ignore_case)
         strName += " ignore_case";
       if (pTrigger->bKeepEvaluating)
         strName += " keep_evaluating";
       if (pTrigger->bOneShot)
         strName += " one_shot";
       if (pTrigger->bRepeat)
         strName += " repeat";
       if (pTrigger->bTemporary)
         strName += " temporary";
       if (pTrigger->bIncluded)
         strName += " included";

       strName += CFormat (" send_to:%s", (LPCTSTR) GetSendToString (pTrigger->iSendTo));
       strName += CFormat (" seq:%i", pTrigger->iSequence);

       Note (strName);
       }


    Note (TFormat ("%i trigger%s.", PLURAL (iCount)));

    } // end of triggers
//-----------------------------------------------------------------------
//          options
//-----------------------------------------------------------------------
  else if (strcmp (Command, "options") == 0)
    ShowOptions ();     // list world options
//-----------------------------------------------------------------------
//          alpha_options
//-----------------------------------------------------------------------
  else if (strcmp (Command, "alpha_options") == 0)
    ShowAlphaOptions ();     // list world options
//-----------------------------------------------------------------------
//          global_options
//-----------------------------------------------------------------------
  else if (strcmp (Command, "global_options") == 0)
    App.ShowGlobalOptions (this);     // list global options
//-----------------------------------------------------------------------
//          option_info
//-----------------------------------------------------------------------
  else if (strcmp (Command, "option_info") == 0)
    {
    CString strType;
    CString strRange;
    CString strNote;
    long i;

    for (i = 0; OptionsTable [i].pName; i++)
      {
      int iMinimum = OptionsTable [i].iMinimum; 
      int iMaximum = OptionsTable [i].iMaximum; 

      if ((iMinimum | iMaximum) == 0)
        {
        strType = "Boolean"; 
        strRange = "y/n";
        }
      else
        {
        strType = "Number";
        strRange = CFormat ("%i to %i", iMinimum, iMaximum);
        }

      if (OptionsTable [i].iFlags & OPT_CUSTOM_COLOUR)
        strNote = ", Custom colour";
      else
      if (OptionsTable [i].iFlags & OPT_RGB_COLOUR)
        strNote = ", RGB colour";
      else
        strNote.Empty ();

      Note (CFormat ("%s, %s, %s, %0.0f%s",  // name, type, range, default, note
                     OptionsTable [i].pName,
                     (LPCTSTR) strType,
                     (LPCTSTR) strRange,
                     OptionsTable [i].iDefault,
                     (LPCTSTR) strNote
                     ));

      } // end of numeric options

    for (i = 0; AlphaOptionsTable [i].pName; i++)
      {
      strType = "Alpha"; 


      if (AlphaOptionsTable [i].iFlags & OPT_MULTLINE)
        strNote = "Multi-line";
      else
      if (AlphaOptionsTable [i].iFlags & OPT_PASSWORD)
        strNote = "Password";
      else
        strNote.Empty ();

      Note (CFormat ("%s, %s, , \"%s\", %s",  // name, type, , default, note
                     (LPCTSTR) AlphaOptionsTable [i].pName,
                     (LPCTSTR) strType,
                     AlphaOptionsTable [i].sDefault,
                     (LPCTSTR) strNote
                     ));

      } // end of alpha options

    
    } // end of option_info
//-----------------------------------------------------------------------
//          included_options
//-----------------------------------------------------------------------
  else if (strcmp (Command, "included_options") == 0)
    {
    long i;

    for (i = 0; OptionsTable [i].pName; i++)
      {
      if (m_NumericConfiguration [i]->bInclude)
        Note (CFormat ("%s = %i",  // name, included value
                       (LPCTSTR) OptionsTable [i].pName,
                       m_NumericConfiguration [i]->iValue
                       ));

      } // end of numeric options

    for (i = 0; AlphaOptionsTable [i].pName; i++)
      {

      if (m_AlphaConfiguration [i]->bInclude)
        Note (CFormat ("%s = %s ",  // name, included value
                       (LPCTSTR) AlphaOptionsTable [i].pName,
                       (LPCTSTR) m_AlphaConfiguration [i]->sValue
                       ));

      } // end of alpha options

    
    } // end of option_info
//-----------------------------------------------------------------------
//          variables
//-----------------------------------------------------------------------
  else if (strcmp (Command, "variables") == 0)
    {

    for (POSITION pos = GetVariableMap ().GetStartPosition(); pos; iCount++)
      {
      CString strVariableName;
      CVariable * variable_item;

      GetVariableMap ().GetNextAssoc (pos, strVariableName, variable_item);

      CString strName = CFormat ("%-10s = %s", 
                                (LPCTSTR) strVariableName,
                                (LPCTSTR) variable_item->strContents);


      Note (strName);
      }

    Note (TFormat ("%i variable%s.", PLURAL (iCount)));

    } // end of variables
//-----------------------------------------------------------------------
//          arrays
//-----------------------------------------------------------------------
  else if (strcmp (Command, "arrays") == 0)
    {

    // delete our arrays
    for (tStringMapOfMaps::iterator it = GetArrayMap ().begin (); 
         it != GetArrayMap ().end ();
         it++, iCount++)
           {
           Note (CFormat ("Array: \"%s\"", it->first.c_str ()));

          // now show key/value pairs
          for (tStringToStringMap::iterator i = it->second->begin ();
               i != it->second->end ();
               i++)
                 Note (CFormat ("  Key \"%s\" = \"%s\"", i->first.c_str (), i->second.c_str ()));

           }

    Note (TFormat ("%i array%s.", PLURAL (iCount)));

    } // end of arrays
//-----------------------------------------------------------------------
//          plugins
//-----------------------------------------------------------------------
  else if (strcmp (Command, "plugins") == 0)
    {
    CPlugin * p;
    POSITION ppos,  // plugin pos
             pos;   // other pos

    for (ppos = m_PluginList.GetHeadPosition (); ppos; iCount++)
      {
      p = m_PluginList.GetNext (ppos);
      Note (TFormat ("Name:       %s", (LPCTSTR) p->m_strName));
      Note (TFormat ("ID:         %s", (LPCTSTR) p->m_strID));
      Note (TFormat ("Purpose:    %s", (LPCTSTR) p->m_strPurpose));
      Note (TFormat ("Author:     %s", (LPCTSTR) p->m_strAuthor));
      Note (TFormat ("Disk file:  %s", (LPCTSTR) p->m_strSource));
      Note (TFormat ("Language:   %s", (LPCTSTR) p->m_strLanguage));
      Note (TFormat ("Enabled:    %s", SHOW_TRUE (p->m_bEnabled)));

      if (!p->m_strScript.IsEmpty ())
        {
        Note (Translate ("Script:"));
        Note (Translate ("-------(start script)----------"));
        Note (p->m_strScript);
        Note (Translate ("--------(end script)-----------"));
        }

      CString strName;
      CTrigger * pTrigger;
      CAlias * pAlias;
      CTimer * pTimer;
      CVariable * pVariable;
      int iCount2;

      for (pos = p->m_TriggerMap.GetStartPosition(), iCount2 = 1; pos; iCount2++)
         {
         p->m_TriggerMap.GetNextAssoc (pos, strName, pTrigger);
         Note (TFormat ("Trigger %i: %s=%s",
                          iCount2,
                          (LPCTSTR) pTrigger->trigger,
                          (LPCTSTR) pTrigger->contents));
         // warn if no dispid
         if (pTrigger->dispid == DISPID_UNKNOWN && !pTrigger->strProcedure.IsEmpty ())
           Note (TFormat ("--> Script sub %s NOT active <--", (LPCTSTR) pTrigger->strProcedure)); 
        }  // end of triggers

      for (pos = p->m_AliasMap.GetStartPosition(), iCount2 = 1; pos; iCount2++)
         {
         p->m_AliasMap.GetNextAssoc (pos, strName, pAlias);
         Note (TFormat ("Alias %i: %s=%s",
                        iCount2,
                        (LPCTSTR) pAlias->name,
                        (LPCTSTR) pAlias->contents));
         // warn if no dispid
         if (pAlias->dispid == DISPID_UNKNOWN && !pAlias->strProcedure.IsEmpty ())
           Note (CFormat ("--> Script sub %s NOT active <--", (LPCTSTR) pAlias->strProcedure)); 
         } // end of aliases

      for (pos = p->m_TimerMap.GetStartPosition(), iCount2 = 1; pos; iCount2++)
         {
         p->m_TimerMap.GetNextAssoc (pos, strName, pTimer);
         Note (TFormat ("Timer %i: %02i:%02i:%04.2f=%s",
                        iCount2,
                        pTimer->iAtHour + pTimer->iEveryHour,
                        pTimer->iAtMinute + pTimer->iEveryMinute,
                        pTimer->fAtSecond + pTimer->fEverySecond,
                        (LPCTSTR) pTimer->strContents));
         // warn if no dispid
         if (pTimer->dispid == DISPID_UNKNOWN && !pTimer->strProcedure.IsEmpty ())
           Note (TFormat ("--> Script sub %s NOT active <--", (LPCTSTR) pTimer->strProcedure)); 
         } // end of timers

      for (pos = p->m_VariableMap.GetStartPosition(), iCount2 = 1; pos; iCount2++)
         {
         p->m_VariableMap.GetNextAssoc (pos, strName, pVariable);
         Note (TFormat ("Variable %i: %s=%s",
                        iCount2,
                        (LPCTSTR) pVariable->strLabel,
                        (LPCTSTR) pVariable->strContents));
         } // end of variables

      Note (TFormat ("<--- (end plugin \"%s\") --->", (LPCTSTR) p->m_strName));  
      Note ("");
      } // end of doing each plugin

    Note (TFormat ("%i plugin%s.", PLURAL (iCount)));

    }
//-----------------------------------------------------------------------
//          internal commands
//-----------------------------------------------------------------------
  else if (strcmp (Command, "internal_commands") == 0)
    {

	  for (iCount = 0; CommandIDs [iCount].iCommandID; iCount++)
		  Note (CommandIDs [iCount].sCommandName);

    Note (TFormat ("%i internal command%s.", PLURAL (iCount)));

    } // end of internal commands
//-----------------------------------------------------------------------
//          info
//-----------------------------------------------------------------------
  else if (strcmp (Command, "info") == 0)
    {

	  for (iCount = 0; InfoTypes [iCount].iInfoType; iCount++)
      {
      COleVariant v;

      v = GetInfo (InfoTypes [iCount].iInfoType);

      if (v.vt == VT_NULL)
        Note (CFormat ("%s = (UNIMPLEMENTED INFO TYPE)", 
                InfoTypes [iCount].sDescription));
      else
        {
        v.ChangeType (VT_BSTR);

        Note (CFormat ("%03i: %s = %s", InfoTypes [iCount].iInfoType,
                       InfoTypes [iCount].sDescription,
                       (LPCTSTR) CString (v.bstrVal)));

        v.Clear ();   // get rid of string (is this necessary?)
        } // end of not VT_NULL
      }

    Note (TFormat ("%i info item%s.", PLURAL (iCount)));

    } // end of info
//-----------------------------------------------------------------------
//          summary
//-----------------------------------------------------------------------
  else if (strcmp (Command, "summary") == 0)
    {

    POSITION pos;
    CString strName;
    CTrigger * pTrigger;
    CAlias * pAlias;
    CTimer * pTimer;
    CPlugin * pPlugin;

    __int64   nTotalMatches = 0;
    __int64   nTotalMatchAttempts = 0;
    long      nTotal = 0;
    long      nEnabled = 0;
    long      nRegexp = 0;
    LONGLONG  iTimeTaken = 0;
    double    elapsed_time;
    int i;

    CString enabledFore = "#0080FF";
    CString disabledFore = "gray";

    // work out colour for not-disabled messages
    if (m_iNoteTextColour >= 0 && m_iNoteTextColour < MAX_CUSTOM)
      enabledFore = ColourToName (m_customtext [m_iNoteTextColour]);


    Note ("");
    ColourNote  (SCRIPTERRORCONTEXTFORECOLOUR, "", "-------------- MUSHclient summary --------------");
    Note ("");
	  Note (TFormat ("MUSHclient version: %s", MUSHCLIENT_VERSION));
    // show compilation date
    Note (TFormat ("Compiled: %s.", __DATE__));
    Note (CTime::GetCurrentTime().Format (TranslateTime ("Time now: %A, %B %d, %Y, %#I:%M %p")));

    
    OSVERSIONINFO ver;
    // see which OS we are using
    memset(&ver, 0, sizeof(ver));
    ver.dwOSVersionInfoSize = sizeof(ver);
    ::GetVersionEx (&ver);

    // work out operating system

    CString sVersion = TFormat ("Unknown (Platform %ld, Major %ld, Minor %ld)",
                               ver.dwPlatformId, ver.dwMajorVersion, ver.dwMinorVersion);

    if (ver.dwPlatformId == 1)  // Windows 95-style versions
      {
      switch (ver.dwMinorVersion)
        {
        case  0: sVersion = "Windows 95"; break;
        case 10: sVersion = "Windows 98"; break;
        case 90: sVersion = "Windows ME"; break;
        } // end of switch on dwMinorVersion
      }  // end of dwPlatformId == 1
    else if  (ver.dwPlatformId == 2)  // Windows NT versions
      {
      switch (ver.dwMajorVersion)
        {
        case 3: sVersion = "Windows NT 3.51"; break;
        case 4: sVersion = "Windows NT";      break;
        case 5: 
          switch (ver.dwMinorVersion)
            {
            case 0: sVersion = "Windows 2000";        break;
            case 1: sVersion = "Windows XP";          break;
            case 2: sVersion = "Windows Server 2003"; break;
            } // end of switch on dwMinorVersion
          break;  // end case 5 of dwMinorVersion

        case 6: 
          switch (ver.dwMinorVersion)
            {
            case 0: sVersion = "Windows Vista"; break;
            case 1: sVersion = "Windows 7";     break;
            } // end of switch on dwMinorVersion
          break;  // end case 6 of dwMinorVersion

        }  // end of switch on dwMajorVersion
      }  // end of dwPlatformId == 2


    Note (TFormat ("Operating system: %s", (LPCTSTR) sVersion));

    // show included library versions
    Note (TFormat ("Libraries: %s, PCRE %s, PNG %s, SQLite3 %s, Zlib %s", 
          LUA_RELEASE, 
          XSTRING(PCRE_MAJOR.PCRE_MINOR), 
          PNG_LIBPNG_VER_STRING, 
          SQLITE_VERSION, 
          ZLIB_VERSION));

    Note (TFormat ("World name: '%s', ID: %s", 
                    (LPCTSTR) m_mush_name, (LPCTSTR) m_strWorldID));


    // scripting info

    ColourNote  (SCRIPTERRORCONTEXTFORECOLOUR, "", "-- Scripting --");
    Note (TFormat ("Script language: %s, enabled: %s", 
                  (LPCTSTR) m_strLanguage,
                  SHOW_TRUE (m_bEnableScripts)
                  ));
    Note (TFormat ("Scripting active: %s", SHOW_TRUE (GetScriptEngine ()) ));
    if (!m_strScriptFilename.IsEmpty ())
      {
      Tell (Translate ("Script file: "));

      Hyperlink ("!!" DEBUG_PLUGIN_ID ":editscript()", 
           m_strScriptFilename, "Click to edit script", "cyan", "", 0);

      Note ("");
      }
    Note (TFormat ("Lua sandbox is %i characters, DLL loading allowed: %s",  
                  App.m_strLuaScript.GetLength (),
                  SHOW_TRUE (App.m_bEnablePackageLibrary)));

    Note (TFormat ("Scripting prefix: '%s'. External editor in use: %s.",
          m_strScriptPrefix, SHOW_TRUE (!m_bEditScriptWithNotepad)));
    if (!m_bEditScriptWithNotepad)
      Note (TFormat ("Editor path: %s", (LPCTSTR) m_strScriptEditor));

    // time taken to execute scripts
    if (App.m_iCounterFrequency > 0)
      {
      elapsed_time = ((double) m_iScriptTimeTaken) / 
                      ((double) App.m_iCounterFrequency);
      Note (TFormat ("Scripting for: %1.6f seconds.", elapsed_time));
      }

    ColourNote  (SCRIPTERRORCONTEXTFORECOLOUR, "", "-- Triggers, aliases, timers, variables --");
   
    // count number of triggers matched
    for (pos = m_TriggerMap.GetStartPosition(); pos; )
      {
      m_TriggerMap.GetNextAssoc (pos, strName, pTrigger);
      nTotal++;

      if (pTrigger->regexp)
        {
        iTimeTaken += pTrigger->regexp->iTimeTaken;
        nTotalMatchAttempts += pTrigger->regexp->m_iMatchAttempts;
        }

      nTotalMatches += pTrigger->nMatched;

      if (pTrigger->bRegexp)
         nRegexp++;

      if (pTrigger->bEnabled)
         nEnabled++;
      }

    // time taken to execute triggers
    if (App.m_iCounterFrequency > 0)
      {
      elapsed_time = ((double) iTimeTaken) / 
                      ((double) App.m_iCounterFrequency);
      }
    else
      elapsed_time = 0;

    Tell (TFormat ("** Triggers: %ld in world file, triggers enabled: %s.", 
                  nTotal,
                  SHOW_TRUE (m_enable_triggers)));
    if (nTotal > 0)
      {
      Tell (" [");
      Hyperlink ("!!" DEBUG_PLUGIN_ID ":triggerlist()", 
                 "Triggers", "Click to list triggers", "cyan", "", 0);
      Note ("]");
      }
    else
      Note ("");

    Note (TFormat ("   %ld enabled, %ld regexp, %I64d attempts, %I64d matched, %1.6f seconds.",
                   nEnabled, nRegexp, nTotalMatchAttempts, nTotalMatches, elapsed_time));



    nTotalMatches = 0;
    nTotalMatchAttempts = 0;
    nTotal = 0;
    nEnabled = 0;
    nRegexp = 0;
    iTimeTaken = 0;
    elapsed_time;

    // count number of aliases matched
    for (pos = m_AliasMap.GetStartPosition(); pos; )
      {
      m_AliasMap.GetNextAssoc (pos, strName, pAlias);
      nTotal++;

      if (pAlias->regexp)
        {
        iTimeTaken += pAlias->regexp->iTimeTaken;
        nTotalMatchAttempts += pAlias->regexp->m_iMatchAttempts;
        }

      nTotalMatches += pAlias->nMatched;
      if (pAlias->bRegexp)
         nRegexp++;

      if (pAlias->bEnabled)
         nEnabled++;
      }

    // time taken to execute aliases
    if (App.m_iCounterFrequency > 0)
      {
      elapsed_time = ((double) iTimeTaken) / 
                      ((double) App.m_iCounterFrequency);
      }
    else
      elapsed_time = 0;

    Tell (TFormat ("** Aliases: %ld in world file, aliases enabled: %s.", 
                  nTotal,
                  SHOW_TRUE (m_enable_aliases)));
    if (nTotal > 0)
      {
      Tell (" [");
      Hyperlink ("!!" DEBUG_PLUGIN_ID ":aliaslist()", 
                 "Aliases", "Click to list aliases", "cyan", "", 0);
      Note ("]");
      }
    else
      Note ("");
    
    Note (TFormat ("   %ld enabled, %ld regexp, %I64d attempts, %I64d matched, %1.6f seconds.",
                   nEnabled, nRegexp, nTotalMatchAttempts, nTotalMatches, elapsed_time));

    nTotalMatches = 0;
    nTotal = 0;
    nEnabled = 0;

    // count number of timers fired
    for (pos = m_TimerMap.GetStartPosition(); pos; )
      {
      m_TimerMap.GetNextAssoc (pos, strName, pTimer);
      nTotal++;

      nTotalMatches += pTimer->nMatched;

      if (pTimer->bEnabled)
         nEnabled++;
      }


    Tell (TFormat ("** Timers: %ld in world file, timers enabled: %s.", 
                  nTotal,
                  SHOW_TRUE (m_bEnableTimers)));

    if (nTotal > 0)
      {
      Tell (" [");
      Hyperlink ("!!" DEBUG_PLUGIN_ID ":timerlist()", 
                 "Timers", "Click to list timers", "cyan", "", 0);
      Note ("]");
      }
    else
      Note ("");

    Note (TFormat ("   %ld enabled, %I64d fired.",
                   nEnabled, nTotalMatches));
    if (App.m_nTimerInterval > 0)
      Note (TFormat ("   Timers checked every %i second(s).", App.m_nTimerInterval));
    else
      Note (TFormat ("   Timers checked every %0.1f seconds.", (float) MIN_TIMER_INTERVAL / 1000.0));


    

    // time taken to do MCCP
    if (App.m_iCounterFrequency > 0)
      {
      elapsed_time = ((double) m_iCompressionTimeTaken) / 
                      ((double) App.m_iCounterFrequency);
      }
    else
      elapsed_time = 0;


    nTotal = 0;

    for (pos = m_VariableMap.GetStartPosition(); pos; nTotal++)
      {
      CString strVariableName;
      CVariable * variable_item;

      m_VariableMap.GetNextAssoc (pos, strVariableName, variable_item);
      }

    Tell (TFormat ("** Variables: %ld.", nTotal));

    if (nTotal > 0)
      {
      Tell (" [");
      Hyperlink ("!!" DEBUG_PLUGIN_ID ":variablelist()", 
                 "Variables", "Click to list variables", "cyan", "", 0);
      Note ("]");
      }
    else
      Note ("");

    ColourNote  (SCRIPTERRORCONTEXTFORECOLOUR, "", "-- MCCP --");

    if (m_bCompress)
      {
      Note (TFormat ("MCCP active, took %1.6f seconds to decompress", elapsed_time));
      Note (TFormat ("MCCP received %I64d compressed bytes, decompressed to %I64d bytes.", m_nTotalCompressed, m_nTotalUncompressed));
      if (m_nTotalUncompressed)
        {
        double fRatio = (double) m_nTotalCompressed / (double) m_nTotalUncompressed * 100.0;
        Note (TFormat ("MCCP compression ratio was: %6.1f%% (lower is better)", fRatio));
        }
      }
    else
      Note (Translate ("MCCP not active."));

    ColourNote  (SCRIPTERRORCONTEXTFORECOLOUR, "", "-- Plugins (Processing order) --");

    // count and display plugins

    nTotal = 0;
    nEnabled = 0;

    for (pos = m_PluginList.GetHeadPosition (); pos; iCount++)
      {
      pPlugin = m_PluginList.GetNext (pos);
      nTotal++;
      
      if (pPlugin->m_bEnabled)
         nEnabled++;

      CString strColour = enabledFore;
      const char * pActive = "Enabled";
      if (!pPlugin->m_bEnabled)
        {
        pActive = "DISABLED";
        strColour = disabledFore; 
        }

      ColourTell (strColour, "", TFormat ("ID: %s, '", 
                  (LPCTSTR) pPlugin->m_strID));

      Hyperlink (CFormat ("!!" DEBUG_PLUGIN_ID ":editplugin(_%s_)",(LPCTSTR) pPlugin->m_strID), 
                 pPlugin->m_strName, "Click to edit plugin", "cyan", "", 0);

      ColourTell (strColour, "", TFormat ("', %8s", 
            pActive));

      // no quick way of finding variables count
      int nTotalVariables = 0;
      for (POSITION varpos = pPlugin->m_VariableMap.GetStartPosition(); varpos; nTotalVariables++)
        {
        CString strVariableName;
        CVariable * variable_item;
        pPlugin->m_VariableMap.GetNextAssoc (varpos, strVariableName, variable_item);
        }

      // first time we find a non-zero count, we draw the left bracket
      // afterwards, we put a space between the previous item and this one
      int bDoneOne = false;

      // hyperlink for triggers
      if (pPlugin->m_TriggerArray.GetSize () > 0)
        {
        ColourTell (strColour, "", " [");
        Hyperlink (CFormat ("!!" DEBUG_PLUGIN_ID ":triggerlist(_%s_)",(LPCTSTR) pPlugin->m_strID), 
                   "Tr", "Click to list triggers", "cyan", "", 0);
        bDoneOne = true;
        }

      // hyperlink for aliases
      if (pPlugin->m_AliasArray.GetSize () > 0)
        {
        if (bDoneOne)
          Tell (" ");
        else
          ColourTell (strColour, "", " [");
        Hyperlink (CFormat ("!!" DEBUG_PLUGIN_ID ":aliaslist(_%s_)",(LPCTSTR) pPlugin->m_strID), 
                   "Al", "Click to list aliases", "cyan", "", 0);
        bDoneOne = true;
        }

      // hyperlink for timers
      if (pPlugin->m_TimerRevMap.size () > 0)
        {
        if (bDoneOne)
          Tell (" ");
        else
          ColourTell (strColour, "", " [");
        Hyperlink (CFormat ("!!" DEBUG_PLUGIN_ID ":timerlist(_%s_)",(LPCTSTR) pPlugin->m_strID), 
                   "Ti", "Click to list timers", "cyan", "", 0);
        bDoneOne = true;
        }

      // hyperlink for variables
      if (nTotalVariables > 0)
        {
        if (bDoneOne)
          Tell (" ");
        else
          ColourTell (strColour, "", " [");
        Hyperlink (CFormat ("!!" DEBUG_PLUGIN_ID ":variablelist(_%s_)",(LPCTSTR) pPlugin->m_strID), 
                   "Va", "Click to list variables", "cyan", "", 0);
        bDoneOne = true;
        }

      // wrap up line
      if (bDoneOne)
        ColourNote (strColour, "", "]");
      else
        Note ("");

      }  // end of for each plugin

    Note (TFormat ("** Plugins: %ld loaded, %ld enabled.", nTotal, nEnabled));

    ColourNote  (SCRIPTERRORCONTEXTFORECOLOUR, "", "-- Comms --");

    // show bytes sent/received

    CString strStatus = GetConnectionStatus (m_iConnectPhase);

    Note (TFormat ("Connect phase: %i (%s). NAWS wanted: %s", 
          m_iConnectPhase, (LPCTSTR) strStatus, SHOW_TRUE (m_bNAWS)));

    __int64 nInK = m_nBytesIn / (__int64) 1024;
    __int64 nOutK = m_nBytesOut / (__int64) 1024;
    Note (TFormat ("Received: %I64d bytes (%I64d Kb)", m_nBytesIn, nInK));
    Note (TFormat ("Sent: %I64d bytes (%I64d Kb)", m_nBytesOut, nOutK));
    Note (TFormat ("Received %I64d packets, sent %I64d packets.", m_iInputPacketCount, m_iOutputPacketCount));
    Note (TFormat ("Total lines received: %ld", m_total_lines));

    Note (TFormat ("This connection: Sent %ld lines, received %ld lines.", m_nTotalLinesSent, m_nTotalLinesReceived));

    // telnet negotiation

    Tell (TFormat ("Telnet (IAC) received: DO: %ld, DONT: %ld, WILL: %ld, WONT: %ld, SB: %ld",
                  m_nCount_IAC_DO,      
                  m_nCount_IAC_DONT,    
                  m_nCount_IAC_WILL,    
                  m_nCount_IAC_WONT,    
                  m_nCount_IAC_SB));

    if (m_nCount_IAC_DO || m_nCount_IAC_DONT || m_nCount_IAC_WILL || m_nCount_IAC_WONT)
      {
      Tell (" [");
      Hyperlink ("!!" DEBUG_PLUGIN_ID ":telnetlist()", 
                 "Telnet", "Click to list telnet IAC items", "cyan", "", 0);
      Tell ("]");
      }

    Note ("");

    ColourNote  (SCRIPTERRORCONTEXTFORECOLOUR, "", "-- MXP --");

    //MXP

    const char * sMode = "Unknown";

    if (m_iUseMXP >= eOnCommandMXP && eOnCommandMXP <= eNoMXP)
       sMode =  sMxpModes [m_iUseMXP];

    Note (TFormat ("MXP active: %s, Pueblo mode: %s, Activated: %s", 
                    SHOW_TRUE (m_bMXP ),
                    SHOW_TRUE (m_bPuebloActive),
                    sMode));

    Note (TFormat ("MXP tags received: %I64d", m_iMXPtags));        
    Note (TFormat ("MXP entities received: %I64d", m_iMXPentities));    
    Note (TFormat ("MXP errors: %I64d", m_iMXPerrors));              

    ColourNote  (SCRIPTERRORCONTEXTFORECOLOUR, "", "-- Commands --");

    // commands typed

    nTotal = 0;

    for(pos = GetFirstViewPosition(); pos != NULL; )
	    {
	    CView* pView = GetNextView(pos);
	    
	    if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
  	    {
		    CSendView* pmyView = (CSendView*)pView;

        for (POSITION pos = pmyView->m_msgList.GetHeadPosition(); pos; nTotal++)
            pmyView->m_msgList.GetNext (pos);

	      }	  // end of being a CSendView
      }   // end of loop through views

    Note (TFormat ("Commands in command history: %ld", nTotal));
    Note (TFormat ("Speed walking enabled: %s. Speed walking prefix: %s",
          SHOW_TRUE (m_enable_speed_walk), (LPCTSTR) m_speed_walk_prefix));
    Note (TFormat ("Command stacking enabled: %s. Command stack character: '%s'",
          SHOW_TRUE (m_enable_command_stack), (LPCTSTR) m_strCommandStackCharacter));

    // accelerators

    Tell (TFormat ("Accelerators defined: %ld", m_AcceleratorToCommandMap.size ()));

    if (m_AcceleratorToCommandMap.size () > 0)
      {
      Tell (" [");
      Hyperlink ("!!" DEBUG_PLUGIN_ID ":acceleratorlist()", 
                 "Accelerators", "Click to list accelerators", "cyan", "", 0);
      Note ("]");
      }
    else
      Note ("");

    ColourNote  (SCRIPTERRORCONTEXTFORECOLOUR, "", "-- Miniwindows --");

    // miniwindows
    nTotal = 0;
    nEnabled = 0;

    for (MiniWindowMapIterator win_it = m_MiniWindows.begin (); 
         win_it != m_MiniWindows.end ();
         win_it++)
           {
           CMiniWindow * pWindow = win_it->second;
           nTotal++;
           int nHotspots = 0;
           int nFonts = 0;
           int nImages = 0;

           if (pWindow->GetShow ())
             nEnabled++;

          // count hotspots
          for (HotspotMapIterator hit = pWindow->m_Hotspots.begin (); 
               hit != pWindow->m_Hotspots.end ();
               hit++)
                 nHotspots++;

          for (FontMap::const_iterator fit = pWindow->GetFonts ().begin (); 
               fit != pWindow->GetFonts ().end ();
               fit++)
                 nFonts++;

          for (ImageMap::const_iterator  imit = pWindow->GetImages ().begin (); 
               imit != pWindow->GetImages ().end ();
               imit++)
                 nImages++;

           Note (TFormat ("Window: '%s', at (%ld,%ld,%ld,%ld), shown: %s",
                win_it->first.c_str (), 
                pWindow->m_rect.left,
                pWindow->m_rect.top,
                pWindow->m_rect.right,
                pWindow->m_rect.bottom,
                SHOW_TRUE (pWindow->GetShow ())));

           Note (TFormat ("        width: %ld, height: %ld, position: %d, hotspots: %ld, fonts: %ld, images: %ld", 
                pWindow->GetWidth (),
                pWindow->GetHeight (),
                pWindow->GetPosition (),
                nHotspots,
                nFonts,
                nImages
                ));

           }

    Note (TFormat ("** Miniwindows: %ld loaded, %ld shown.", nTotal, nEnabled));

    ColourNote  (SCRIPTERRORCONTEXTFORECOLOUR, "", "-- Output window --");

    // output window info

    CRect r (0, 0, 0, 0);
    CMUSHView* pmyView = GetFirstOutputWindow ();

    if (pmyView)
        pmyView->GetClientRect(&r);

    Note (TFormat ("Output pixels: width %ld, height: %ld, font width: %ld, font height: %ld",
          r.right, r.bottom, m_FontWidth, m_FontHeight));

    if (m_FontWidth > 0 && m_FontHeight > 0)
      Note (TFormat ("               can show %ld characters, wrapping at column %ld, height %ld lines.",
            r.right / m_FontWidth, m_nWrapColumn, r.bottom / m_FontHeight));


    Note (TFormat ("Output buffer: %i of %ld lines.", 
                                  m_LineList.GetCount (),
                                  m_maxlines));

    ColourNote  (SCRIPTERRORCONTEXTFORECOLOUR, "", "-- Miscellaneous --");

    // logging?

    Note (TFormat ("Logging: %s, tracing: %s", 
          SHOW_TRUE (m_logfile), SHOW_TRUE (m_bTrace) ));

    // databases?

    for (tDatabaseMapIterator db_it = m_Databases.begin (); 
         db_it != m_Databases.end ();
         db_it++)
           {
           Note (TFormat ("Database: '%s', disk file: '%s'",
                db_it->first.c_str (), 
                db_it->second->db_name.c_str ()
                ));
           }

    Note (TFormat ("** SQLite3 databases: %i", m_Databases.size ()));

    // sound buffers?

    nTotal = 0;

    for (i = 0; i < MAX_SOUND_BUFFERS; i++)
      if (m_pDirectSoundSecondaryBuffer [i])
        nTotal++;

    Note (TFormat ("Sound buffers in use: %ld", nTotal));

    // end summary

    Note ("");
    ColourNote  (SCRIPTERRORCONTEXTFORECOLOUR, "", 
                  "---------------------- End summary ----------------------");

    } // end of summary

#ifdef PANE
//-----------------------------------------------------------------------
//          panes
//-----------------------------------------------------------------------
  else if (strcmp (Command, "panes") == 0)
    {

    for (PaneMapIterator it = m_PaneMap.begin (); 
         it != m_PaneMap.end ();
         it++, iCount++)
           {
           CPaneView * pView = it->second;
           Note ("------------------------");
           Note (TFormat ("Pane name   = %s", pView->m_sName.c_str ())) ;
           Note (TFormat (" Pane title = %s", pView->m_sTitle.c_str ())) ;
           Note (TFormat (" Left = %i, Top = %i, Width = %i, "
                          "Height = %i, Flags = %08X, Lines = %i",
                  pView->m_iLeft,
                  pView->m_iTop,
                  pView->m_iWidth,
                  pView->m_iHeight,
                  pView->m_iFlags,
                  pView->m_lines.size ()
                  )) ;
          
          int iLine = 1;

           // show lines
          for (deque <CPaneLine *>::iterator line_it = pView->m_lines.begin (); 
               line_it != pView->m_lines.end (); 
               line_it++, iLine++)
                 {
                 CPaneLine * pLine = *line_it;
                 Note (TFormat ("Line %i, Width = %i, Styles = %i, newline = %i",
                          iLine,
                          pLine->m_iWidth,
                          pLine->m_vStyles.size (),
                          (int) pLine->GetNewline () ));
                 int iStyle = 1;
                 // show styles in line
                 for (CPaneStyleVector::iterator style_it = pLine->m_vStyles.begin (); 
                      style_it != pLine->m_vStyles.end (); 
                      style_it++, iStyle++)
                        {
                        CPaneStyle * pStyle = *style_it;
                        Note (CFormat (" %i(%i): %s", 
                              iStyle,
                              pStyle->m_sText.length (),
                              pStyle->m_sText.c_str ()));
                        } // end of each style

                 }  // end of each line

           }  // end of doing each pane

    Note (TFormat ("%i pane%s.", PLURAL (iCount)));

    } // end of panes
#endif // PANE
  else
    {
    Note (Translate ("----- Debug commands available -----"));
    Note ("actions");
    Note ("aliases");
    Note ("alpha_options");
    Note ("ansi");
    Note ("arrays");
    Note ("colours (or colors)");
    Note ("colours256 (or colors256)");
    Note ("commands");
    Note ("custom_colours (or custom_colors)");
    Note ("entities");
    Note ("global_options");
    Note ("included_options");
    Note ("info");
    Note ("internal_commands");
#ifdef _DEBUG
    Note ("memchk");
    Note ("memdiff");
#endif
    Note ("options");
    Note ("option_info");
    Note ("plugins");
    Note ("server_elements");
    Note ("server_entities");
    Note ("summary");
    Note ("triggers");
    Note ("variables");
    }   // end of invalid/none entered


//-----------------------------------------------------------------------
//          end of debugging
//-----------------------------------------------------------------------

  // we don't use result yet, but might one day :)

	return vaResult;
}   // end of CMUSHclientDoc::Debug


void CMUSHclientDoc::ShowOptions (void)
  {
  for (long i = 0; OptionsTable [i].pName; i++)
    if (!m_CurrentPlugin ||
    ((OptionsTable [i].iFlags & OPT_PLUGIN_CANNOT_READ) == 0))
      Note (CFormat ("%35s = %ld",
                     (LPCTSTR) OptionsTable [i].pName, 
                     GetOptionItem (i)));
  } // end of CMUSHclientDoc::ShowOptions

void CMUSHclientDoc::ShowAlphaOptions (void)
  {
  for (long i = 0; AlphaOptionsTable [i].pName; i++)
    if (!(AlphaOptionsTable [i].iFlags & OPT_PASSWORD)) // except the password <eg>
      if (!m_CurrentPlugin ||
      ((AlphaOptionsTable [i].iFlags & OPT_PLUGIN_CANNOT_READ) == 0))
        Note (CFormat ("%35s = %s",
                       (LPCTSTR) AlphaOptionsTable [i].pName, 
                       (LPCTSTR) GetAlphaOptionItem (i)));
  } // end of CMUSHclientDoc::ShowAlphaOptions


//-----------------------------------------------------------------------
//   This is called in response to hyperlinks generated by Debug 'summary' 
//     above. (eg. you can get a list of triggers, or info on one trigger)
//-----------------------------------------------------------------------

void CMUSHclientDoc::DebugHelper (const CString strAction, CString strArgument)
  {

  CString strPluginID;

  CPlugin * pSavedPlugin = m_CurrentPlugin;
  m_CurrentPlugin = NULL;

  // if the argument starts with _<plugin_id>_
  // eg. _584f2e481b30ea09e4a32537_triggerlist
  // then switch to that plugin

  if (strArgument.GetLength () >= PLUGIN_UNIQUE_ID_LENGTH + 2 &&
      strArgument.Left (1) == "_" &&
      strArgument.Mid (PLUGIN_UNIQUE_ID_LENGTH + 1, 1) == "_" && 
      IsPluginID (strArgument.Mid (1, PLUGIN_UNIQUE_ID_LENGTH)) 
      )
    { // correct syntax for plugin call
    strPluginID = strArgument.Mid (0, PLUGIN_UNIQUE_ID_LENGTH + 2);
    strArgument = strArgument.Mid (PLUGIN_UNIQUE_ID_LENGTH + 2);  // drop the plugin ID stuff

    m_CurrentPlugin = GetPlugin (strPluginID.Mid (1, PLUGIN_UNIQUE_ID_LENGTH));

    if (!m_CurrentPlugin)
      {
      Note (TFormat ("Plugin ID %s does not exist.", (LPCTSTR) strPluginID.Mid (1, PLUGIN_UNIQUE_ID_LENGTH)));
      m_CurrentPlugin = pSavedPlugin;
      return;
      }    // if not found

    if (strAction != "editplugin")
      {
      Note ("");
      if (m_CurrentPlugin)
        ColourNote  (SCRIPTERRORCONTEXTFORECOLOUR, "", TFormat ("(For plugin: %s)", (LPCTSTR) m_CurrentPlugin->m_strName ));

      if (!m_CurrentPlugin->m_bEnabled)
        ColourNote  (SCRIPTERRORFORECOLOUR, "", TFormat ("Warning: Plugin '%s' disabled.", (LPCTSTR) m_CurrentPlugin->m_strName ));
      }

    }  // seeing if plugin info wanted
  else
    Note ("");

  strArgument.MakeLower ();   // have to do this for lookups to work (eg. triggers)

  CString enabledFore = "#0080FF";
  CString disabledFore = "gray";

  // work out colour for not-disabled messages
  if (m_iNoteTextColour >= 0 && m_iNoteTextColour < MAX_CUSTOM)
    enabledFore = ColourToName (m_customtext [m_iNoteTextColour]);

  POSITION pos;

//-----------------------------------------------------------------------
//          triggerlist
//-----------------------------------------------------------------------

  if (strAction == "triggerlist")
    {

    ColourNote  (SCRIPTERRORCONTEXTFORECOLOUR, "", "------ Trigger List (evaluation order) ------");
    if (!m_enable_triggers)
      ColourNote  (SCRIPTERRORFORECOLOUR, "", "Warning: All triggers disabled.");


    Note ("");
    int iTrigger;

    for (iTrigger = 0; iTrigger < GetTriggerArray ().GetSize (); iTrigger++)
      {
      CTrigger * pTrigger = GetTriggerArray () [iTrigger];

      // get unlabelled trigger's internal name
      const char * pLabel = pTrigger->strLabel;
      if (pLabel [0] == 0)
         pLabel = GetTriggerRevMap () [pTrigger].c_str ();


      const char * pType = "Normal";
      if (pTrigger->bRegexp)
        pType = "Regexp";

      CString strColour = enabledFore;

      const char * pActive = "Enabled";
      if (!pTrigger->bEnabled)
        {
        pActive = "DISABLED";
        strColour = disabledFore; 
        }

      CString strMatch = pTrigger->trigger;
      if (strMatch.GetLength () > 38)
        {
        strMatch = strMatch.Left (35);
        strMatch = strMatch + "...";
        }

      int iSpaces = 14 - strlen (pLabel);
      if (iSpaces < 0)
        iSpaces = 0;

      ColourTell (strColour, "", CFormat ("%3i. ", iTrigger + 1));
      ColourTell (strColour, "", CFormat ("%6s, %8s : ", pType, pActive));
      Hyperlink (CFormat ("!!" DEBUG_PLUGIN_ID ":showtrigger(%s%s)", (LPCTSTR) strPluginID, pLabel), 
                 pLabel, "Click to view this trigger", "cyan", "", 0);

      ColourNote (strColour, "", CFormat ("%*s %s", iSpaces, "", (LPCTSTR) strMatch));

      }   // end of for loop

    ColourNote  (SCRIPTERRORCONTEXTFORECOLOUR, "", TFormat ("%i trigger%s.", PLURAL (iTrigger)));

    }   // end of triggerlist


//-----------------------------------------------------------------------
//          aliaslist
//-----------------------------------------------------------------------

  else if (strAction == "aliaslist")
    {

    ColourNote  (SCRIPTERRORCONTEXTFORECOLOUR, "", "------ Alias List (evaluation order) ------");
    if (!m_enable_aliases)
      ColourNote  (SCRIPTERRORFORECOLOUR, "", "Warning: All aliases disabled.");
    Note ("");

    int iAlias;
    for (iAlias = 0; iAlias < GetAliasArray ().GetSize (); iAlias++)
      {
      CAlias * pAlias = GetAliasArray () [iAlias];

      // get unlabelled alias's internal name
      const char * pLabel = pAlias->strLabel;
      if (pLabel [0] == 0)
         pLabel = GetAliasRevMap () [pAlias].c_str ();


      const char * pType = "Normal";
      if (pAlias->bRegexp)
        pType = "Regexp";

      CString strColour = enabledFore;

      const char * pActive = "Enabled";
      if (!pAlias->bEnabled)
        {
        pActive = "DISABLED";
        strColour = disabledFore; 
        }

      CString strMatch = pAlias->name;
      if (strMatch.GetLength () > 38)
        {
        strMatch = strMatch.Left (35);
        strMatch = strMatch + "...";
        }

      int iSpaces = 14 - strlen (pLabel);
      if (iSpaces < 0)
        iSpaces = 0;

      ColourTell (strColour, "", CFormat ("%3i. ", iAlias + 1));
      ColourTell (strColour, "", CFormat ("%6s, %8s : ", pType, pActive));

      Hyperlink (CFormat ("!!" DEBUG_PLUGIN_ID ":showalias(%s%s)", (LPCTSTR) strPluginID, pLabel), 
                 pLabel, "Click to view this alias", "cyan", "", 0);


      ColourNote (strColour, "", CFormat ("%*s %s", iSpaces, "", (LPCTSTR) strMatch));

      }   // end of for loop

    ColourNote  (SCRIPTERRORCONTEXTFORECOLOUR, "", TFormat ("%i alias%s.", PLURALES (iAlias)));

    }   // end of aliaslist

//-----------------------------------------------------------------------
//          timerlist
//-----------------------------------------------------------------------

  else if (strAction == "timerlist")
    {

    ColourNote  (SCRIPTERRORCONTEXTFORECOLOUR, "", "------ Timer List (firing order) ------");
    if (!m_bEnableTimers)
      ColourNote  (SCRIPTERRORFORECOLOUR, "", "Warning: All timers disabled.");

    if (m_iConnectPhase != eConnectConnectedToMud)
      ColourNote  (SCRIPTERRORFORECOLOUR, "", "Warning: Not connected - some timers may not fire.");

    Note ("");
    int iCount = 0;
    CmcDateTime timeNow = CmcDateTime::GetTimeNow ();

    map<double,CTimer *> sortedTimers;

    // put into map for sorting into firing order
    for (pos = GetTimerMap ().GetStartPosition(); pos; )
      {
      CTimer * pTimer;
      CString strName;

      GetTimerMap ().GetNextAssoc (pos, strName, pTimer);

      sortedTimers [pTimer->tFireTime.GetTime ()] = pTimer;
      } // end of for loop

      
    // now pull out of map and display
    for (map<double,CTimer *>::const_iterator it = sortedTimers.begin ();
         it != sortedTimers.end ();
         it++, iCount++)
           {

      CTimer * pTimer = it->second;

      // get unlabelled timer's internal name
      const char * pLabel = pTimer->strLabel;
      if (pLabel [0] == 0)
         pLabel = GetTimerRevMap () [pTimer].c_str ();


      CString strColour = enabledFore;

      const char * pActive = "Enabled";
      if (!pTimer->bEnabled)
        {
        pActive = "DISABLED";
        strColour = disabledFore; 
        }


      const char * pType = "At";
      if (pTimer->iType == CTimer::eInterval)
        pType = "Every";

      ColourTell (strColour, "", CFormat ("%3i. ", iCount + 1));
      ColourTell (strColour, "", CFormat ("%8s : ", pActive));

      Hyperlink (CFormat ("!!" DEBUG_PLUGIN_ID ":showtimer(%s%s)", (LPCTSTR) strPluginID, pLabel), 
                 pLabel, "Click to view this timer", "cyan", "", 0);



      ColourTell (strColour, "", TFormat (" %5s %02i:%02i:%04.2f",
                      pType,
                      pTimer->iAtHour + pTimer->iEveryHour,
                      pTimer->iAtMinute + pTimer->iEveryMinute,
                      pTimer->fAtSecond + pTimer->fEverySecond));

      if (pTimer->tFireTime.GetTime () > 0)
        {
        CmcDateTimeSpan ts = pTimer->tFireTime - timeNow;
        ColourTell (strColour, "", TFormat (" - firing in %8.1f seconds.", ts.GetTotalSeconds () ));
        }

      Note ("");

      }   // end of for loop

    ColourNote  (SCRIPTERRORCONTEXTFORECOLOUR, "", TFormat ("%i timer%s.", PLURAL (iCount)));

    }   // end of timerlist

//-----------------------------------------------------------------------
//          variablelist
//-----------------------------------------------------------------------

  else if (strAction == "variablelist")
    {

    ColourNote  (SCRIPTERRORCONTEXTFORECOLOUR, "", "------ Variable List (alphabetic order) ------");
    Note ("");
    map<string,string> sortedVariables;

    for (POSITION pos = GetVariableMap ().GetStartPosition(); pos; )
      {
      CString strVariableName;
      CVariable * pVariable;

      GetVariableMap ().GetNextAssoc (pos, strVariableName, pVariable);

      // first get rid of carriage-returns
      CString strContents = Replace (pVariable->strContents, "\r", "", true);
      // now show newlines as \n
      strContents = Replace (strContents, "\n", "\\n", true);

      if (strContents.GetLength () > 46)
        {
        strContents = strContents.Left (43);
        strContents = strContents + "...";
        }

      sortedVariables [(LPCTSTR) strVariableName] = strContents;

      }   // end of for loop


    // this will get them into alphabetic order

    for (map<string,string>::const_iterator it = sortedVariables.begin ();
         it != sortedVariables.end ();
         it++)

      Note ( CFormat ("%-30s = %s", 
                                it->first.c_str (),
                                it->second.c_str ()));

    ColourNote  (SCRIPTERRORCONTEXTFORECOLOUR, "", TFormat ("%i variable%s.", PLURAL (sortedVariables.size ())));


    }   // end of variablelist


//-----------------------------------------------------------------------
//          acceleratorlist
//-----------------------------------------------------------------------

  else if (strAction == "acceleratorlist")
    {

    ColourNote  (SCRIPTERRORCONTEXTFORECOLOUR, "", "------ Accelerator List (alphabetic order) ------");
    Note ("");
    map<string,string> sortedAccelerators;

    map<long, WORD>::const_iterator sit;

    for (sit = m_AcceleratorToCommandMap.begin (); 
         sit != m_AcceleratorToCommandMap.end (); sit++)
      {
      CString key = KeyCodeToString (sit->first >> 16, sit->first);
      string command = m_CommandToMacroMap [sit->second];

      CString strSendTo;
      strSendTo.Format (" [%s]", (LPCTSTR) GetSendToString (m_CommandToSendToMap [sit->second]));

      // first get rid of carriage-returns
      CString strContents = Replace (command.c_str (), "\r", "", true);
      // now show newlines as \n
      strContents = Replace (strContents, "\n", "\\n", true);
      
      if (strContents.GetLength () > 40)
        {
        strContents = strContents.Left (37);
        strContents = strContents + "...";
        }

      strContents += strSendTo;
      sortedAccelerators [(LPCTSTR) key] = strContents;
      }      // end of looping through each accelerator

    // this will get them into alphabetic order

    for (map<string,string>::const_iterator it = sortedAccelerators.begin ();
         it != sortedAccelerators.end ();
         it++)

      Note ( CFormat ("%-20s = %s", 
                                it->first.c_str (),
                                it->second.c_str ()));

    ColourNote  (SCRIPTERRORCONTEXTFORECOLOUR, "", TFormat ("%i accelerator%s.", PLURAL (sortedAccelerators.size ())));


    }   // end of acceleratorlist

//-----------------------------------------------------------------------
//          telnetlist
//-----------------------------------------------------------------------

  else if (strAction == "telnetlist")
    {

    ColourNote  (SCRIPTERRORCONTEXTFORECOLOUR, "", "------ Telnet negotiation List (numeric order) ------");
    Note ("");

    map<int,string> telnet_meanings;

    // got these from  Code_Chart.xml plugin

    telnet_meanings [0x01] = "Echo";                    //  1 Echo
    telnet_meanings [0x03] = "Suppress Go-ahead (SGA)"; //  3 Suppress go ahead
    telnet_meanings [0x05] = "Status";                  //  5 Status
    telnet_meanings [0x06] = "Timing Mark";             //  6 Timing mark
    telnet_meanings [0x18] = "Termtype";                // 24 Terminal type
    telnet_meanings [0x19] = "End of record (EOR)";     // 25 EOR
    telnet_meanings [0x1F] = "Window Size (NAWS)";      // 31 Window size
    telnet_meanings [0x20] = "Terminal Speed";          // 32 Terminal speed
    telnet_meanings [0x21] = "RFC";                     // 33 Remote flow control
    telnet_meanings [0x22] = "Line Mode";               // 34 Line mode
    telnet_meanings [0x24] = "EV";                      // 36 Environment variables
    telnet_meanings [0x2A] = "Charset";                 // 42 Character set
    telnet_meanings [0x55] = "MCCP1";       //  85 MUD Compression Protocol v1
    telnet_meanings [0x56] = "MCCP2";       //  86 MUD Compression Protocol v2
    telnet_meanings [0x5A] = "MSP";         //  90 (MUD Sound Protocol)
    telnet_meanings [0x5B] = "MXP";         //  91 (MUD eXtension Protocol)
    telnet_meanings [0x5D] = "ZMP";         //  93 (ZMP Protocol)
    telnet_meanings [0x66] = "Aardwolf";    // 102 (Aardwolf telnet protocol)
    telnet_meanings [0xC8] = "ATCP";        // 200 ATCP (Achaea Telnet Protocol)
    telnet_meanings [0xC9] = "ATCP2/GMCP";  // 201 ATCP2/GMCP (Generic Mud Control Protocol)
    telnet_meanings [0xFF] = "Extended Options"; // for future expansion


    for (int i = 0; i < 256; i++)
      {
      if (m_bClient_sent_IAC_DO   [i] ||
          m_bClient_sent_IAC_DONT [i] ||
          m_bClient_sent_IAC_WILL [i] ||
          m_bClient_sent_IAC_WONT [i] ||
          m_bClient_got_IAC_DO    [i] ||
          m_bClient_got_IAC_DONT  [i] ||
          m_bClient_got_IAC_WILL  [i] ||
          m_bClient_got_IAC_WONT  [i])
        {

        string sMeaning = telnet_meanings [i];
        CString strMeaning;
        if (!sMeaning.empty ())
          {
          strMeaning = " (";
          strMeaning += sMeaning.c_str ();
          strMeaning += ")";
          }

        Note (CFormat ("%3i%s", i, (LPCTSTR) strMeaning));
        ColourTell ("cyan", "", "     Got:  ");
        Note (CFormat ("DO:   %-3s, DONT: %-3s, WILL: %-3s, WONT: %-3s ",
              SHOW_TRUE_LC (m_bClient_got_IAC_DO   [i]), 
              SHOW_TRUE_LC (m_bClient_got_IAC_DONT [i]), 
              SHOW_TRUE_LC (m_bClient_got_IAC_WILL [i]), 
              SHOW_TRUE_LC (m_bClient_got_IAC_WONT [i])));

        ColourTell ("cyan", "", "     Sent: ");
        Note (CFormat ("WILL: %-3s, WONT: %-3s, DO:   %-3s, DONT: %-3s ",
              SHOW_TRUE_LC (m_bClient_sent_IAC_WILL [i]), 
              SHOW_TRUE_LC (m_bClient_sent_IAC_WONT [i]), 
              SHOW_TRUE_LC (m_bClient_sent_IAC_DO   [i]), 
              SHOW_TRUE_LC (m_bClient_sent_IAC_DONT [i])));

        } // end of something true

      }  // end of loop

    }   // end of telnetlist

//-----------------------------------------------------------------------
//          showtrigger
//-----------------------------------------------------------------------

  else if (strAction == "showtrigger")
    {

    CTrigger * pTrigger;

    if (!GetTriggerMap ().Lookup (strArgument, pTrigger))
      {
      Note (TFormat ("Trigger %s does not exist.", (LPCTSTR) strArgument));
      return;
      }    // if not found

    char * p = NULL;

    try
      {
      CMemFile f;      // open memory file for writing
      CArchive ar(&f, CArchive::store);
      Save_One_Trigger_XML (ar, pTrigger);
      ar.Close();

      int nLength = f.GetLength ();
      p = (char *) f.Detach ();

      CString strXML (p, nLength);

      free (p);   // remove memory allocated in CMemFile
      p = NULL;
    
      ColourNote  (SCRIPTERRORCONTEXTFORECOLOUR, "", CFormat ("------ Trigger: %s ------",(LPCTSTR) strArgument) );
      Note ("");
      Note (strXML);
      Note ("");

      }  // end of try block

    catch (CException* e)
	    {
      if (p)
        free (p);   // remove memory allocated in CMemFile
	    e->ReportError();
	    e->Delete();
	    }

    DebugShow (Translate ("Matched count"),       pTrigger->nMatched);
    DebugShow (Translate ("Has script"),          pTrigger->dispid != DISPID_UNKNOWN);
    DebugShow (Translate ("Times script called"), pTrigger->nInvocationCount);
    DebugShow (Translate ("When last matched"),   pTrigger->tWhenMatched);
    DebugShow (Translate ("Send to"),             GetSendToString (pTrigger->iSendTo));
    DebugShow (Translate ("Temporary"),           pTrigger->bTemporary);

    if (pTrigger->regexp)
      {
      if (App.m_iCounterFrequency > 0)
        {
        DebugShowD (Translate ("Time to match"), ((double) pTrigger->regexp->iTimeTaken) / 
                        ((double) App.m_iCounterFrequency));
        }

      DebugShow  (Translate ("Match attempts"), pTrigger->regexp->m_iMatchAttempts);
      }

    Note ("");
    
    }   // end of showtrigger

//-----------------------------------------------------------------------
//          showalias
//-----------------------------------------------------------------------


  else if (strAction == "showalias")
    {

    CAlias * pAlias;

    if (!GetAliasMap ().Lookup (strArgument, pAlias))
      {
      Note (TFormat ("Alias %s does not exist.", (LPCTSTR) strArgument));
      return;
      }    // if not found

    char * p = NULL;

    try
      {
      CMemFile f;      // open memory file for writing
      CArchive ar(&f, CArchive::store);
      Save_One_Alias_XML (ar, pAlias);
      ar.Close();

      int nLength = f.GetLength ();
      p = (char *) f.Detach ();

      CString strXML (p, nLength);

      free (p);   // remove memory allocated in CMemFile
      p = NULL;
    
      ColourNote  (SCRIPTERRORCONTEXTFORECOLOUR, "", CFormat ("------ Alias: %s ------",(LPCTSTR) strArgument) );
      Note ("");
      Note (strXML);
      Note ("");

      }  // end of try block

    catch (CException* e)
	    {
      if (p)
        free (p);   // remove memory allocated in CMemFile
	    e->ReportError();
	    e->Delete();
	    }

    DebugShow (Translate ("Matched count"),       pAlias->nMatched);
    DebugShow (Translate ("Has script"),          pAlias->dispid != DISPID_UNKNOWN);
    DebugShow (Translate ("Times script called"), pAlias->nInvocationCount);
    DebugShow (Translate ("When last matched"),   pAlias->tWhenMatched);
    DebugShow (Translate ("Send to"),             GetSendToString (pAlias->iSendTo));
    DebugShow (Translate ("Temporary"),           pAlias->bTemporary);
    if (pAlias->regexp)
      {
      if (App.m_iCounterFrequency > 0)
        {
        DebugShowD (Translate ("Time to match"), ((double) pAlias->regexp->iTimeTaken) / 
                        ((double) App.m_iCounterFrequency));
        }
      DebugShow  (Translate ("Match attempts"), pAlias->regexp->m_iMatchAttempts);
      }

    Note ("");
    
    }   // end of showalias


//-----------------------------------------------------------------------
//          showtimer
//-----------------------------------------------------------------------

  else if (strAction == "showtimer")
    {

    CTimer * pTimer;

    if (!GetTimerMap ().Lookup (strArgument, pTimer))
      {
      Note (TFormat ("Timer %s does not exist.", (LPCTSTR) strArgument));
      return;
      }    // if not found

    char * p = NULL;

    try
      {
      CMemFile f;      // open memory file for writing
      CArchive ar(&f, CArchive::store);
      Save_One_Timer_XML (ar, pTimer);
      ar.Close();

      int nLength = f.GetLength ();
      p = (char *) f.Detach ();

      CString strXML (p, nLength);

      free (p);   // remove memory allocated in CMemFile
      p = NULL;
    
      ColourNote  (SCRIPTERRORCONTEXTFORECOLOUR, "", CFormat ("------ Timer: %s ------",(LPCTSTR) strArgument) );
      Note ("");
      Note (strXML);
      Note ("");

      }  // end of try block

    catch (CException* e)
	    {
      if (p)
        free (p);   // remove memory allocated in CMemFile
	    e->ReportError();
	    e->Delete();
	    }

    CmcDateTimeSpan ts = pTimer->tFireTime - CmcDateTime::GetTimeNow ();

    DebugShow  (Translate ("Fired count"),           pTimer->nMatched);
    DebugShow  (Translate ("Has script"),            pTimer->dispid != DISPID_UNKNOWN);
    DebugShow  (Translate ("Times script called"),   pTimer->nInvocationCount);
    DebugShow  (Translate ("When to fire next"),     pTimer->tFireTime);
    DebugShowD (Translate ("Seconds to fire next"),  ts.GetTotalSeconds ());
    DebugShow  (Translate ("When last reset/fired"), pTimer->tWhenFired);
    DebugShow  (Translate ("Send to"),               GetSendToString (pTimer->iSendTo));
    DebugShow  (Translate ("Temporary"),             pTimer->bTemporary);


    Note ("");
    
    }   // end of showtimer

//-----------------------------------------------------------------------
//          editplugin
//-----------------------------------------------------------------------

  else if (strAction == "editplugin")
    {

    if (m_CurrentPlugin)
      {
      CString strName = m_CurrentPlugin->m_strSource;

      if (m_bEditScriptWithNotepad)
        {
        CTextDocument * pNewDoc =
          (CTextDocument *) App.OpenDocumentFile (strName);
        if (pNewDoc)
          {
          pNewDoc->SetTheFont ();
          pNewDoc->m_pRelatedWorld = this;
          pNewDoc->m_iUniqueDocumentNumber = m_iUniqueDocumentNumber;
          }
        else
          ::UMessageBox(TFormat ("Unable to edit the plugin file %s.",
                      (LPCTSTR) strName), 
                          MB_ICONEXCLAMATION);
        return;
        }   // end of using inbuilt notepad

      EditFileWithEditor (strName);
      }   // end of having a plugin

    }   // end of editplugin

//-----------------------------------------------------------------------
//          editscript
//-----------------------------------------------------------------------

  else if (strAction == "editscript")
    {

    if (m_bEditScriptWithNotepad)
      {
      CTextDocument * pNewDoc =
        (CTextDocument *) App.OpenDocumentFile (m_strScriptFilename);
      if (pNewDoc)
        {
        pNewDoc->SetTheFont ();
        pNewDoc->m_pRelatedWorld = this;
        pNewDoc->m_iUniqueDocumentNumber = m_iUniqueDocumentNumber;
        }
      else
        ::UMessageBox(TFormat ("Unable to edit the script file %s.",
                    (LPCTSTR) m_strScriptFilename), 
                        MB_ICONEXCLAMATION);
      return;
      }   // end of using inbuilt notepad

    EditFileWithEditor (m_strScriptFilename);

    }   // end of editscript

//-----------------------------------------------------------------------
//          unexpected command
//-----------------------------------------------------------------------

  else
    Note (TFormat ("DebugHelper: %s, %s", (LPCTSTR) strAction,(LPCTSTR) strArgument));

  // put current plugin back
  m_CurrentPlugin = pSavedPlugin;

  } // end of CMUSHclientDoc::DebugHelper


#define DEBUG_LABEL_WIDTH 22

void CMUSHclientDoc::DebugShow (const char * sTitle, const char * sString)
  {
  ColourTell  (SCRIPTERRORCONTEXTFORECOLOUR, "", CFormat ("%*s: ", DEBUG_LABEL_WIDTH, sTitle) );
  Note (CFormat ("%s", sString));
  } // end of CMUSHclientDoc::DebugShow  (string)

void CMUSHclientDoc::DebugShow (const char * sTitle, const long iNumber)
  {
  ColourTell  (SCRIPTERRORCONTEXTFORECOLOUR, "", CFormat ("%*s: ", DEBUG_LABEL_WIDTH, sTitle) );
  Note (CFormat ("%ld", iNumber));
  } // end of CMUSHclientDoc::DebugShow  (number)

void CMUSHclientDoc::DebugShow (const char * sTitle, const bool iBoolean)
  {
  ColourTell  (SCRIPTERRORCONTEXTFORECOLOUR, "", CFormat ("%*s: ", DEBUG_LABEL_WIDTH, sTitle) );
  if (iBoolean)
    Note ("Yes");
  else
    Note ("No");
  } // end of CMUSHclientDoc::DebugShow  (boolean)

void CMUSHclientDoc::DebugShow (const char * sTitle, const CmcDateTime theDate)
  {
  ColourTell  (SCRIPTERRORCONTEXTFORECOLOUR, "", CFormat ("%*s: ", DEBUG_LABEL_WIDTH, sTitle) );
  if (theDate.GetTime () == 0)
    Note (Translate ("Never"));
  else
    Note (CFormat ("%02i:%02i:%02i", theDate.GetHour (), theDate.GetMinute (), theDate.GetSecond ()));
  } // end of CMUSHclientDoc::DebugShow  (date)

void CMUSHclientDoc::DebugShow (const char * sTitle, const CTime theDate)
  {
  ColourTell  (SCRIPTERRORCONTEXTFORECOLOUR, "", CFormat ("%*s: ", DEBUG_LABEL_WIDTH, sTitle) );
  if (theDate.GetTime () == 0)
    Note (Translate ("Never"));
  else
    Note (CFormat ("%02i:%02i:%02i", theDate.GetHour (), theDate.GetMinute (), theDate.GetSecond ()));
  } // end of CMUSHclientDoc::DebugShow  (date)

void CMUSHclientDoc::DebugShowD (const char * sTitle, const double fNumber)
  {
  ColourTell  (SCRIPTERRORCONTEXTFORECOLOUR, "", CFormat ("%*s: ", DEBUG_LABEL_WIDTH, sTitle) );
  Note (CFormat ("%1.6f", fNumber));
  } // end of CMUSHclientDoc::DebugShowD  (double)

