#include "stdafx.h"
#include "MUSHclient.h"

#include "doc.h"
#include "TextView.h"
#include "TextDocument.h"
#include <process.h>
#include "mainfrm.h"
#include "color.h"

#include "scripting\errors.h"

#define PNG_NO_CONSOLE_IO
#include "png\png.h"

#include "dialogs\RegexpProblemDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


extern char working_dir [_MAX_PATH];

// for returning window text as a CString

CString GetText (const CWnd & theWindow)
  {
  CString strText;
  theWindow.GetWindowText (strText);
  return strText;
  }



CString ConvertToRegularExpression (const CString & strMatchString,
                                    const bool bWholeLine,
                                    const bool bMakeAsterisksWildcards)
  {
CString strRegexp;

int iSize = 0;
const char * p;
char * pOut;

  // count places where the size will get larger
  for (p = strMatchString; *p; p++)
    if ((unsigned char) *p < ' ')
      iSize += 3;   // non-printable 01 to 1F become \xhh
    else if (isalnum ((unsigned char) *p) || 
             *p == ' ' || 
             (unsigned char) *p >= 0x80)
      continue; // A-Z, 0-9, space, hi-order bits set do not increase the size
    else if (*p == '*')
      iSize += 4;  // * becomes .*?  (non-greedy wildcard)
    else 
      iSize++;     // others are escaped, eg. ( becomes \(

  // work out new buffer size
  pOut = strRegexp.GetBuffer (strMatchString.GetLength () + 
                              iSize +   // escaped sequences
                              2 +       // ^ at start, and $ at end
                              10);       // 1 for null, plus 9 just in case

    
  // now copy across non-regexp, turning it into a regexp
  if (bWholeLine)
    *pOut++ = '^';    // start of buffer marker

  for (p = strMatchString; *p; p++)
    {
    if (*p == '\n')   // newlines become \n
      {
      *pOut++ = '\\';
      *pOut++ = 'n';
      }
    else if ((unsigned char) *p < ' ')
      {
      *pOut++ = '\\';
      *pOut++ = 'x';
      char buf [20];
      itoa ((unsigned char) *p, buf, 16);
      if (buf [1] == 0)   // add leading zero
        *pOut++ = '0';
      *pOut++ = buf [0];
      if (buf [1])        // 2 digits
        *pOut++ = buf [1];
      }
    else if (isalnum ((unsigned char) *p) || 
             *p == ' ' ||
             (unsigned char) *p >= 0x80)
      *pOut++ = *p;   // copy alphanumeric, spaces, across
    else if (*p == '*' && bMakeAsterisksWildcards)   // wildcard
      {
      *pOut++ = '(';
      *pOut++ = '.';
      *pOut++ = '*';
      *pOut++ = '?';
      *pOut++ = ')';
      }
    else 
      {     // non-alphanumeric are escaped out
      *pOut++ = '\\';
      *pOut++ = *p;
      }
    } // end of scanning input buffer

  if (bWholeLine)
    *pOut++ = '$';    // end of buffer marker

  *pOut++ = 0;    // trailing null

  strRegexp.ReleaseBuffer (-1);

  return strRegexp;
  }   // end of ConvertToRegularExpression



// Helper routine for setting the font in certain description windows to a
// fixed-pitch courier, to make editing easier.

void FixFont (ptrCFont & pFont, 
              CEdit & editctrl,
              const CString strName,
              const int iSize,
              const int iWeight,
              const DWORD iCharset)
  {

   delete pFont;         // get rid of old font

   pFont = new CFont;    // create new font

   if (pFont)
    {
    
    CDC dc;

    dc.CreateCompatibleDC (NULL);

     int lfHeight = -MulDiv(iSize, 
                    dc.GetDeviceCaps(LOGPIXELSY), 72);

     pFont->CreateFont(lfHeight, // int nHeight, 
            0, // int nWidth, 
            0, // int nEscapement, 
            0, // int nOrientation, 
            iWeight, // int nWeight, 
            0, // BYTE bItalic, 
            0, // BYTE bUnderline, 
            0, // BYTE cStrikeOut, 
            iCharset, // BYTE nCharSet, 
            0, // BYTE nOutPrecision, 
            0, // BYTE nClipPrecision, 
            0, // BYTE nQuality, 
            MUSHCLIENT_FONT_FAMILY, // BYTE nPitchAndFamily,  
            strName);// LPCTSTR lpszFacename );

      // Get the metrics of the font.

//      dc.SelectObject(pFont);

      editctrl.SetFont (pFont);
      /*
      editctrl.SendMessage (WM_SETFONT,
                                   (WPARAM) pFont->m_hObject,
                                   MAKELPARAM (TRUE, 0));
      */
     }  // end of having a font to select


  }   // end of FixFont


CString FixupEscapeSequences (const CString & strSource)
{
CString strDest = strSource;
char * p = strDest.GetBuffer (strSource.GetLength () + 1);
char * pNew = p;
char c;
int i;

  for ( ; *p; p++)
    {
    c = *p;

    // look for escape sequences ...
    if (c == '\\')
      {
      c = *(++p);
      switch (c)
        {
        case 'a'  : c = '\a'; break; // alert
        case 'b'  : c = '\b'; break; // backspace
        case 'f'  : c = '\f'; break; // formfeed
        case 'n'  : c = '\n'; break; // newline
        case 'r'  : c = '\r'; break; // carriage return
        case 't'  : c = '\t'; break; // horizontal tab
        case 'v'  : c = '\v'; break; // vertical tab
        case '\'' : c = '\''; break; // single quote
        case '\"' : c = '\"'; break; // double quote
        case '\\' : c = '\\'; break; // backslash
        case '?'  : c = '\?'; break; // question mark
        case 'x'  : c = 0;  // hex sequence
                  p++;

                  for (i = 0; *p && isxdigit (*p) && i < 2 ; p++, i++)
                    {
                    int  iNewDigit = toupper (*p);
                    if (iNewDigit >= 'A')
                      iNewDigit -= 7;
                    c = (c << 4) + iNewDigit - '0';
                    } // end of building up hex literal
                  p--;  // backtrack over trailing non-hex character
                  break; // end of hex sequence

        } // end of switch
      } // end of escape sequence
    *pNew++ = c;
    }

  *pNew++ = 0;   // null terminator

  // we have finished with it - release it
  strDest.ReleaseBuffer (-1);

  return strDest;

} // end of FixupEscapeSequences




bool CreateTextWindow (const char * sText,
                       const char * sTitle,
                       CMUSHclientDoc * pDoc,
                       __int64 iUniqueDocumentNumber,
                       const CString strFontName,
                       const LONG     iFontSize,
                       const LONG     iFontWeight,
                       const DWORD iFontCharset,
                       const COLORREF textColour,
                       const COLORREF backColour,
                       const CString  strSearchString,
                       const CString  strRecallLinePreamble,
                       const bool bMatchCase,
                       const bool bRegexp,
                       const bool bCommands,
                       const bool bOutput,
                       const bool bNotes,
                       const int iLines,
                       const int iNotepadType)
  {
  // during startup, may not exist
  if (App.m_pNormalDocTemplate == NULL)
     return false;

  CTextDocument * pNewDoc = (CTextDocument *)
    App.m_pNormalDocTemplate->OpenDocumentFile(NULL);

  // created it?
  if (!pNewDoc)
    return false;

  // replace characters it won't let me save under
  CString strTitle (sTitle);
  strTitle.Replace (" ", "");
  strTitle.Replace ("#", "");
  strTitle.Replace ("%", "");
  strTitle.Replace (";", "");
  strTitle.Replace ("/", "-");
  strTitle.Replace ("\\", "-");
  strTitle.Replace (":", "-");
  pNewDoc->SetTitle (strTitle); // for saving under the "title" name

  pNewDoc->m_strTitle              = sTitle;
  pNewDoc->m_pRelatedWorld         = pDoc;
  pNewDoc->m_iUniqueDocumentNumber = iUniqueDocumentNumber;   
  pNewDoc->m_strFontName           = strFontName;             
  pNewDoc->m_iFontSize             = iFontSize;               
  pNewDoc->m_iFontWeight           = iFontWeight;             
  pNewDoc->m_textColour            = textColour;              
  pNewDoc->m_backColour            = backColour;              
  pNewDoc->m_strSearchString       = strSearchString; 
  pNewDoc->m_strRecallLinePreamble = strRecallLinePreamble;
  pNewDoc->m_bMatchCase            = bMatchCase;
  pNewDoc->m_bRegexp               = bRegexp;                 
  pNewDoc->m_bCommands             = bCommands;               
  pNewDoc->m_bOutput               = bOutput;                 
  pNewDoc->m_bNotes                = bNotes;                  
  pNewDoc->m_iLines                = iLines;                  
  pNewDoc->m_iNotepadType          = iNotepadType;                  

  if (iNotepadType == eNotepadWorldLoadError)
    {
    pNewDoc->m_textColour = RGB (128, 0, 0);      // maroon
    pNewDoc->m_backColour = RGB (255, 228, 181);  // moccasin
    }
  else
  if (iNotepadType == eNotepadXMLcomments)
    {
    pNewDoc->m_textColour = RGB (0, 100, 0);      // darkgreen
    pNewDoc->m_backColour = RGB (176, 224, 230);  // powderblue
    }
  else    // otherwise use colours if user specified them
    if (App.m_cNotepadTextColour != App.m_cNotepadBackColour)
      {
      pNewDoc->m_textColour = App.m_cNotepadTextColour;
      pNewDoc->m_backColour = App.m_cNotepadBackColour;
      }

  // find first view
  POSITION pos = pNewDoc->GetFirstViewPosition();

  if (!pos)
    return false;

  // find view
  CTextView * pView = (CTextView *) pNewDoc->GetNextView(pos);

  // ensure font displayed correctly
  pView->SetTheFont ();

  // nothing to put there, bye bye
  if (sText [0] == 0)
    return true;      // OK

  pNewDoc->SetModifiedFlag (TRUE);    // modified
  return pView->SetText (sText); // send the text to it 

  }


CString FixHTMLString (const CString strToFix)
  {
  CString strOldString = strToFix;
  CString strNewString;

  int i;

  while ((i = strOldString.FindOneOf ("<>&\"")) != -1)
    {
    strNewString += strOldString.Left (i);

    switch (strOldString [i])
      {
      case '<': strNewString += "&lt;"; break;
      case '>': strNewString += "&gt;"; break;
      case '&': strNewString += "&amp;"; break;
      case '\"': strNewString += "&quot;"; break;
      }

    strOldString = strOldString.Mid (i + 1);
    }

  strNewString += strOldString;


  return strNewString;

  } // end of FixHTMLString 

// same as above, except quotes are left alone, and tabs are fixed up
CString FixHTMLMultilineString (const CString strToFix)
  {
  CString strOldString = strToFix;
  CString strNewString;

  int i;

  while ((i = strOldString.FindOneOf ("<>&\t")) != -1)
    {
    strNewString += strOldString.Left (i);

    switch (strOldString [i])
      {
      case '<': strNewString += "&lt;"; break;
      case '>': strNewString += "&gt;"; break;
      case '&': strNewString += "&amp;"; break;
      case '\t': strNewString += "&#9;"; break;
      }

    strOldString = strOldString.Mid (i + 1);
    }

  strNewString += strOldString;


  return strNewString;

  } // end of FixHTMLMultilineString 

CString ConvertSetTypeToString (const int set_type)
  {
  CString strType;

  switch (set_type)
    {
    case WORLD:   strType = "world";    break; 
    case TRIGGER: strType = "trigger";  break; 
    case ALIAS:   strType = "alias";    break; 
    case COLOUR:  strType = "colour";   break; 
    case MACRO:   strType = "macro";    break; 
    case STRING:  strType = "string";   break; 
    case TIMER:   strType = "timer";    break;

    default: strType = CFormat ("<Unknown type: %i>", set_type);
             break;
    } // end of switch

  return strType;
  }

unsigned long MakeActionHash (const CString & strAction,
                              const CString & strHint,
                              const CString & strVariable)
  {
  SHS_INFO shsInfo;
  // produce the hash for quick lookups of actions
  CString strHash = strAction;
  
  strHash += strHint;
  strHash += strVariable;

  shsInit   (&shsInfo);
  shsUpdate (&shsInfo, (unsigned char *) (const char *) strHash, 
              strHash.GetLength ());
  shsFinal  (&shsInfo);
  return shsInfo.digest [0];

  } // end of MakeActionHash


// returns TRUE if the clipboard contains a colour in the format:
//  &h7280FA (VB) or #FA8072 (HTML) or 0x7280FA (Jscript)
// or a HTML colour (eg, red, blue, whitesmoke)
//  If so, the colour is converted and returned in "colour"
//    otherwise "colour" is set to black


bool GetClipboardColour (COLORREF & colour)
  {
CString strColour; 
int i;
bool bReverse;
CColours * colour_item;

  // see if we have a colour on the clipboard

  colour = RGB (0, 0, 0);
  if (!GetClipboardContents (strColour, false, false))  // not Unicode, no warning
    return false;
    
  strColour.TrimLeft ();
  strColour.TrimRight ();
  strColour.MakeLower ();

  // look up colour name

  if (App.m_ColoursMap.Lookup (strColour, colour_item))
    {
    colour = colour_item->iColour;
    return true;
    }
  
  strColour.MakeUpper ();
  
  if (strColour.Left (2) == "&H" ||     // VB
      strColour.Left (2) == "0X" )      // JScript
    {
    bReverse = false;
    i = 2;    // start two characters in
    }
  else
  if (strColour.Left (1) == "#")        // HTML
    {
    bReverse = true;
    i = 1;    // start one character in
    }
  else
  if (IsNumber (strColour, false))     // Lua (ordinary 32-bit number)
    {
    colour = atoi (strColour);
    if (colour < 0 || colour > 0xFFFFFF)
      return false;
    return true;
    }
  else
    return false;

  // must be followed by at least on hex digit
  if (!isxdigit (strColour [i]))
    return false;

  // compute hex value
  for ( ; i < strColour.GetLength (); i++)
    if (isxdigit (strColour [i]))
      {
      char c = strColour [i];
      colour <<= 4;    // previous value times 16
      if (c >= 'A')
        c -= 7;
      colour += (long) (c - '0');    // add in new value
      }
    else
      return false;

  if (colour < 0 || colour > 0xFFFFFF)
    return false;

  // for HTML colours, reverse byte order
  if (bReverse)
    colour = RGB (
                  GetBValue (colour),
                  GetGValue (colour),
                  GetRValue (colour)
                  );    

  return true;

  }

/**************************************************************************/
static char base64code[64]=
{
  'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
  'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f',
  'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v',
  'w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/'
};
/**************************************************************************/
// takes a base64 character, returns its value
// from RFC 2045 Table 1 - The Base64 Alphabet
int getBase64Value(char code){

    int val= (int)code;
    int result = 0;

    if (val >= 'A' && val <= 'Z')
        result= val - 65;
    else if (val >= 'a' && val <= 'z')
        result= val - 71;
    else if (val >= '0' && val <= '9')
        result= val + 4;
    else if (val == '+')
        result = 62;
    else if (val == '/')
        result = 63;
    else if (val == '=')
        result = 0;
    else
        ThrowErrorException ("Invalid non base64 character %c (%d)", code, code);
    return result;
}
/**************************************************************************/
CString decodeBase64(const char *coded)
{
  int j = 0;

  CString strResult;
  CString strEncoded = coded;

  // remove whitespace
  strEncoded.Remove (' ');
  strEncoded.Remove ('\t');
  strEncoded.Remove ('\n');
  strEncoded.Remove ('\r');
  strEncoded.Remove ('\f');
  strEncoded.Remove ('\v');

  const char * p = strEncoded;

  int i = strlen(p);

  // result will be 3/4 size of original
  char * d = strResult.GetBuffer (((i / 4) * 3) + 1);
  
  if (i % 4 != 0)
    ThrowErrorException("Not a valid base64 string - incorrect length (not mod 4).");

  // go thru converting each 4 base64 bytes into 3 8bit bytes   
    for (; *p; p +=4 )
      {
      d [j++] = (char)(getBase64Value(p[0])<<2
                | getBase64Value(p[1])>>4);
      d [j++] = (char)(getBase64Value(p[1])<<4
                | getBase64Value(p[2])>>2);
      d [j++] = (char)(getBase64Value(p[2])<<6 
                | getBase64Value(p[3]));
      }
    

  d[j] = 0;

  if (*(--p) == '=')
    {
    if (j>0)
      {
      if (*(--p)=='=')
        {
        d[j-2]='\0';
        }
      else
        {
        d[j-1]='\0';
        }
      }
    }

  strResult.ReleaseBuffer (-1);
  return (strResult);
}


// to wrap at column 76 we actually add a linefeed after doing 57 bytes
#define WRAP_COLUMN 76
#define WRAP_POINT ((WRAP_COLUMN / 4) * 3)

/**************************************************************************/
CString encodeBase64(const char * plaintext, const bool bMultiLine)
{
  unsigned char *pt=(unsigned char *)plaintext;
  int i;
  int j=0;
  int bytes;

  int len = strlen (plaintext);

  CString strResult;

  // result will be 4/3 size of original  
  char * result = strResult.GetBuffer (
        ((len / 3) * 4) +  // goes up in size by 4/3
        4 +   // padding for final couple of bytes, plus null at end
        (((len / WRAP_POINT) + 1) * 2)  // (allow 2 bytes for cr/lfs)
        );  

  // go thru converting each 3 bytes into 4 base64 bytes
  for (i=0; i < (len - 2); i+=3)
    {
    bytes= (pt[i]<<16)
         + (pt[i+1]<<8)
         + (pt[i+2]);
    result[j++] = base64code[(bytes>>18) & 0x3F];
    result[j++] = base64code[(bytes>>12) & 0x3F];
    result[j++] = base64code[(bytes>>6) & 0x3F];
    result[j++] = base64code[(bytes) & 0x3F];

  // Add linefeeds every WRAP_POINT characters

    if(bMultiLine && ((i % WRAP_POINT) == (WRAP_POINT - 3)))
      {                 
      result[j++]='\r';
      result[j++]='\n';
      }
    }

  // pad the remaining characters
  switch(len - i){
    case 0: // exact fit
      break;
    case 1: // need one more character
      bytes= pt[i]<<16;
      result[j++] = base64code[(bytes>>18) & 0x3F];
      result[j++] = base64code[(bytes>>12) & 0x3F];
      result[j++] = '=';
      result[j++] = '=';
      break;
    case 2: // need two more characters 
      bytes= (pt[i]<<16) + (pt[i+1]<<8);
      result[j++] = base64code[(bytes>>18) & 0x3F];
      result[j++] = base64code[(bytes>>12) & 0x3F];
      result[j++] = base64code[(bytes>>6) & 0x3F];
      result[j++] = '=';
      break;

  }
  result[j]='\0';
    

  strResult.ReleaseBuffer (-1);
  return (strResult);
}


CString ColourToName (const COLORREF colour)
  {
int iRed    = GetRValue (colour);
int iGreen  = GetGValue (colour);
int iBlue   = GetBValue (colour);

CString strName;

// see if we can find colour name in list
  for (POSITION pos = App.m_ColoursMap.GetStartPosition(); pos; )
    {

    CColours * pColour;
    CString strColourName;

    App.m_ColoursMap.GetNextAssoc (pos, strColourName, pColour);

    // note - colour might match more than one name
    if (pColour->iColour == colour)
      return strColourName;
    } // end of loop

 return CFormat ("#%02X%02X%02X",
                           iRed,
                           iGreen,
                           iBlue);

  } // end of ColourToName


CString GetGUID (void)
  {
  CString strResult;
  GUID guid;

  if (CoCreateGuid (&guid) == S_OK)
    strResult.Format ("%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                        guid.Data1, guid.Data2, guid.Data3,
                        guid.Data4 [0], guid.Data4 [1], guid.Data4 [2], guid.Data4 [3],
                        guid.Data4 [4], guid.Data4 [5], guid.Data4 [6], guid.Data4 [7]);


  return strResult;
  }   // end of GetGUID


CString GetUniqueID (void) 
{
CString strResult = ::Replace (GetGUID (), "-", "");

  SHS_INFO shsInfo;
  shsInit   (&shsInfo);
  shsUpdate (&shsInfo, (unsigned char *) (const char *) strResult, 
             strResult.GetLength ());
  shsFinal  (&shsInfo);

  strResult.Empty ();
  for (int i = 0; i < (PLUGIN_UNIQUE_ID_LENGTH / 8); i++)
    strResult += CFormat ("%08x", shsInfo.digest [i]);

  return strResult;
  } // end of GetUniqueID

CString GetSendToString (const unsigned short iWhere)
  {
  CString strWhere;

  strWhere = "unknown";
  switch (iWhere)
   {
    case eSendToWorld:        strWhere = "world";            break; 
    case eSendToCommand:      strWhere = "command";          break; 
    case eSendToOutput:       strWhere = "output";           break; 
    case eSendToStatus:       strWhere = "status";           break;
    case eSendToNotepad:      strWhere = "notepad";          break;
    case eAppendToNotepad:    strWhere = "notepad_append";   break;
    case eSendToLogFile:      strWhere = "log";              break;
    case eReplaceNotepad:     strWhere = "notepad_replace";  break;
    case eSendToCommandQueue: strWhere = "queue";            break;
    case eSendToVariable:     strWhere = "variable";         break;
    case eSendToExecute:      strWhere = "execute";          break;
    case eSendToSpeedwalk:    strWhere = "speedwalk";        break;
    case eSendToScript:       strWhere = "script";           break;
    case eSendImmediate:      strWhere = "immediate";        break;
    case eSendToScriptAfterOmit: strWhere = "script_after_omit"; break;
#ifdef PANE
    case eSendToPane:         strWhere = "pane";             break;
#endif // PANE

   }  // end of switch

  return strWhere;
  }     // end of GetSendToString


  // replace German sequences if required -- see bug request 240

CString FixUpGerman (const CString strMessage)
  {
  CString strResult = strMessage;

  strResult.Replace ("\xFC", "ue");    // u with 2 dots
  strResult.Replace ("\xDC", "Ue");    // U with 2 dots
  strResult.Replace ("\xE4", "ae");    // a with 2 dots
  strResult.Replace ("\xC4", "Ae");    // A with 2 dots
  strResult.Replace ("\xF6", "oe");    // o with 2 dots
  strResult.Replace ("\xD6", "Oe");    // O with 2 dots
  strResult.Replace ("\xDF", "ss");    // B

  return strResult;

  } // end of FixUpGerman

CString GetConnectionStatus (const int iStatus)
  {
  CString strResult = Translate ("<unknown>");
  
  switch (iStatus)
    {
    case eConnectNotConnected:          strResult = Translate ("Closed"); break; 
    case eConnectMudNameLookup:         strResult = Translate ("Look up world name"); break; 
    case eConnectProxyNameLookup:       strResult = Translate ("Look up proxy name"); break; 
    case eConnectConnectingToMud:       strResult = Translate ("Connecting to world"); break; 
    case eConnectConnectingToProxy:     strResult = Translate ("Connecting to proxy"); break; 
    case eConnectAwaitingProxyResponse1:strResult = Translate ("Awaiting proxy response (1)"); break; 
    case eConnectAwaitingProxyResponse2:strResult = Translate ("Awaiting proxy response (2)"); break; 
    case eConnectAwaitingProxyResponse3:strResult = Translate ("Awaiting proxy response (3)"); break; 
    case eConnectConnectedToMud:        strResult = Translate ("Open"); break; 
    case eConnectDisconnecting:         strResult = Translate ("Disconnecting"); break;
    } // end of switch

  return strResult;
  }    // end of GetConnectionStatus


void GetHostNameAndAddresses (CString & strHostName, CString & strAddresses)
  {
 char szHostName[256];

 if( gethostname(szHostName, sizeof szHostName) == 0 )
   {
   // Get host adresses
   struct hostent * pHost;
   int i;
   
   pHost = gethostbyname(szHostName);
   strHostName = szHostName;
   
   for( i = 0; pHost != NULL && pHost->h_addr_list [i] != NULL; i++ )
     {
     /*
     CString str;
     int j;
     
     for( j = 0; j < pHost->h_length; j++ )
       {
       CString addr;
       
       if( j > 0 )
         str += ".";
       
       addr.Format("%u", (unsigned int)((unsigned
         char*)pHost->h_addr_list[i])[j]);
       str += addr;

       }
*/
     if (!strAddresses.IsEmpty ())
       strAddresses += ", ";
     strAddresses += inet_ntoa ( *((in_addr *)pHost->h_addr_list[i]));

     
     }
   }
  } // end of GetHostNameAndAddresses


extern tCommandIDMapping CommandIDs [];

CString CommandIDToString (const int ID)
  {
CString strFunction;

  // scan functions in the hope we find it :)

  if (ID)
    for (int i = 0; CommandIDs [i].iCommandID; i++)
      {
      if (ID == CommandIDs [i].iCommandID)
        {
        strFunction = CommandIDs [i].sCommandName;
        break;
        }
      } // end of finding which one

  return strFunction;
  }  // end of CommandIDToString


int StringToCommandID (const CString str)
  {
int ID = 0;

  // scan functions in the hope we find it :)

  for (int i = 0; CommandIDs [i].iCommandID; i++)
    {
    if (str.CompareNoCase (CommandIDs [i].sCommandName) == 0)
      {
      ID = CommandIDs [i].iCommandID;
      break;
      }
    } // end of finding which one

  return ID;

  } // end of StringToCommandID

void MakeRandomNumber (CMUSHclientDoc * pDoc, SHS_INFO & shsInfo)
  {

  CString strHash = pDoc->m_mush_name;

  strHash += CFormat ("%i %i", rand (), _getpid ()); // some random stuff

  struct tm *gmt;
  time_t ltime;
  time( &ltime );
  gmt = gmtime( &ltime );
  strHash += asctime( gmt );      // throw in the date/time

  strHash += CFormat ("%p", pDoc);    // this address will change a fair bit :)
  strHash += CFormat ("%p", pDoc->m_font);    // this address will change a fair bit :)


  // hash our rather bizarre string
  shsInit   (&shsInfo);
  shsUpdate (&shsInfo, (unsigned char *) (const char *) strHash, 
              strHash.GetLength ());
  shsFinal  (&shsInfo);


  }

extern tVirtualKeyMapping VirtualKeys [];

CString KeyCodeToString (const BYTE fVirt, const WORD key)
  {

  CString strKeyCode,
          strKey;
  
   if (fVirt & FSHIFT)
     strKey += "Shift+";
   if (fVirt & FCONTROL)
     strKey += "Ctrl+";
   if (fVirt & FALT)
     strKey += "Alt+";

   // default is key code itself, particularly if FVIRTKEY not specified
   strKeyCode = (char) key;

//  if (fVirt & FVIRTKEY)
    for (int i = 0; VirtualKeys [i].iVirtualKey; i++)
      {
      if (key == VirtualKeys [i].iVirtualKey)
        {
        strKeyCode = VirtualKeys [i].sKeyName;
        break;
        }
      } // end of finding key name

  strKey += strKeyCode;

  return strKey;

  } // end of KeyCodeToString

void StringToKeyCode (const CString strKey,
                      BYTE & fVirt,
                      WORD & key)
  {
  // break key up into individual parts (eg. Ctrl + Shift + blah )
  CStringList strList;
  StringToList (strKey, "+", strList);

  fVirt = FVIRTKEY;
  key = 0;

  for (POSITION pos = strList.GetHeadPosition (); pos; )
    {
    CString strPart = strList.GetNext (pos);
    strPart.TrimLeft ();
    strPart.TrimRight ();
    
    // look for special cases: Alt, Ctrl, Shift
    if (strPart.CompareNoCase ("Alt") == 0)
      {
      if (fVirt & FALT)
        ThrowErrorException ("You cannot have Alt+Alt");
      fVirt |= FALT;
      }
    else if (strPart.CompareNoCase ("Ctrl") == 0)
      {
      if (fVirt & FCONTROL)
        ThrowErrorException ("You cannot have Ctrl+Ctrl");
      fVirt |= FCONTROL;
      }
    else if (strPart.CompareNoCase ("Shift") == 0)
      {
      if (fVirt & FSHIFT)
        ThrowErrorException ("You cannot have Shift+Shift");
      fVirt |= FSHIFT;
      }
    else
      {
      int j;
      for (j = 0; VirtualKeys [j].iVirtualKey; j++)
        {
        if (strPart.CompareNoCase (VirtualKeys [j].sKeyName) == 0)
          {
          if (key != 0)
            ThrowErrorException ("You cannot specify two keystrokes");

          key = VirtualKeys [j].iVirtualKey;
          break;
          }
        } // end of finding key name

      if (VirtualKeys [j].iVirtualKey == 0)
        ThrowErrorException ("Keystoke %s unknown", (LPCTSTR) strPart);
        
      } // must be key itself
    } // end of working through part of key

  } // end of StringToKeyCode


// does a proper shutdown of a socket
// see: http://tangentsoft.net/wskfaq/newbie.html#howclose

void ShutDownSocket (CAsyncSocket & s)
  {


  TRACE ("In ShutDownSocket\n");

  // don't do it if socket invalid - probably already done
  if (s.m_hSocket != INVALID_SOCKET)
    {
    TRACE ("Socket is valid - closing it down\n");

    Frame.SetStatusMessageNow (Translate ("Closing network connection ..."));

    // stop asynchronous notifications (otherwise IOCtl will fail)
    if (!s.AsyncSelect (0))
      {
#ifdef _DEBUG
      int iError = s.GetLastError ();
      TRACE1 ("Error on AsyncSelect: %08X\n", iError);
#endif
      } // end of failure

/*

  --

  I have commented out the code below, which works well enough,
  however it has a major flaw - if the connection is to MUSHclient, then
  it hangs presumably because it needs to leave this thread and process
  the close, which it doesn't so the whole client hangs.

  Unless there is a way of stopping that, or detecting when we have called
  ourself (eg. in the chat system) it is too unreliable.

  --

  //  s.CancelBlockingCall ();  // maybe not, see help

    // make socket block now
    DWORD arg = 0;
    if (!s.IOCtl (FIONBIO, &arg))
      {
      int iError = s.GetLastError ();
      TRACE1 ("Error on IOCtl: %08X\n", iError);
      } // end of failure
 
    // shutdown sends - this will send a FIN to the other end
    if (!s.ShutDown (CAsyncSocket::sends))
      {
      int iError = s.GetLastError ();
      TRACE1 ("Error on ShutDown: %08X\n", iError);
      } // end of failure

    // pull out outstanding data 
    // (we should get a zero returned when the other end closes)

    char buff [100];
    int count;
    do
      {
      count = s.Receive (buff, sizeof buff);
      } while (count != SOCKET_ERROR && count > 0);

*/

    // this is the alternative to the commented-out code
    s.ShutDown (CAsyncSocket::both);

    // now close it
    s.Close ();
    Frame.SetStatusNormal (); 

    } // end of socket being valid

  // all done!

  TRACE ("ShutDownSocket completed OK.\n");

  }  // end of ShutDownSocket


// returns a lower case version of the string 
string tolower (const string & s)
  {
string d (s);

  transform (d.begin (), d.end (), d.begin (), (int(*)(int)) tolower);
  return d;
  }  // end of tolower

// split a line into the first word, and rest-of-the-line
string GetWord (string & s, 
                const string delim,
                const bool trim_spaces)
  {
    
  // find delimiter  
  string::size_type i (s.find (delim));

  // split into before and after delimiter
  string w (s.substr (0, i));

  // if no delimiter, remainder is empty
  if (i == string::npos)
    s.erase ();
  else
    // erase up to the delimiter
    s.erase (0, i + delim.size ());

  // trim spaces if required
  if (trim_spaces)
    {
    w = trim (w);
    s = trim (s);
    }

  // return first word in line
  return w;
  
  } // end of GetWord 

// To be symmetric, we assume an empty string (after trimming spaces)
// will give an empty vector.
// However, a non-empty string (with no delimiter) will give one item
// After that, you get an item per delimiter, plus 1.
// eg.  ""      => empty
//      "a"     => 1 item
//      "a,b"   => 2 items
//      "a,b,"  => 3 items (last one empty)

void StringToVector (const string s, 
                     vector<string> & v,
                     const string delim, 
                     const bool trim_spaces)
  {

  // start with initial string, trimmed of leading/trailing spaces if required
  string s1 (trim_spaces ? trim (s) : s);

  v.clear (); // ensure vector empty

  // no string? no elements
  if (s1.empty ())
    return;

  // add to vector while we have a delimiter
  while (!s1.empty () && s1.find (delim) != string::npos)
    v.push_back (GetWord (s1, delim, trim_spaces));

  // add final element
  v.push_back (s1);
  } // end of StringToVector 

// Takes a vector of strings and converts it to a string 
// like "apples,peaches,pears" 
// Should be symmetric with StringToVector (excepting any spaces that might have
//  been trimmed).

string VectorToString (const vector<string> & v, 
                       const string delim)
  {
  // vector empty gives empty string
  if (v.empty ())
    return "";

  // for copying results into
  ostringstream os;

  // copy all but last one, with delimiter after each one
  copy (v.begin (), v.end () - 1, 
        ostream_iterator<string> (os, delim.c_str ()));

  // return string with final element appended
  return os.str () + *(v.end () - 1);

  } // end of VectorToString

// tests a string to see if it is numeric
bool IsStringNumber (const string & s, const bool bSigned)
  {
  string::size_type pos = 0;

  if (s.empty ())
    return false;  // empty? can't be a number

  if (s [0] == '+' || s [0] == '-')
    {
    pos++;
    if (s.size () == 1)
      return false;   // can't just have the sign
    }

  return s.find_first_not_of ("0123456789", pos) == string::npos;
  }   // end of IsNumber


#define DELIMS "{[()]}"
#define START_DELIMS "{[("
#define END_DELIMS "}])"
#define MAX_NEST 1000

static char GetMatchingDelimiter (const char cDelim)
  {
  switch (cDelim)
    {
    case '[' : return ']'; 
    case '(' : return ')'; 
    case '{' : return '}'; 
    case ']' : return '['; 
    case ')' : return '('; 
    case '}' : return '{'; 
    }   // end of switch
  return 0;   // shouldn't get here :)
  } // end of GetMatchingDelimiter

void FindMatchingBrace (CEdit & ctlEdit, const bool bSelect)
{
int nStartChar,
    nEndChar,  // end of selection
    nLength;  // length of buffer
CString strBuf; // text of window
char cStartDelim;    // start delimiter, eg. [
char c;
const char * p;
const char * pStart;
const char * pStartBuf;
int iLevel = -1;
bool bBackwards = false;
unsigned int iFlags = App.m_nParenMatchFlags;
char cNestArray [MAX_NEST];   // so we know when nested group ends

  // find the selection range
  ctlEdit.GetSel(nStartChar, nEndChar);

   
  strBuf = GetText (ctlEdit);
  nLength = strBuf.GetLength ();

  pStartBuf = strBuf;   // to stop us going too far backwards

  // return if out of buffer
  if (nLength == 0 || nEndChar < 0 || nEndChar > nLength)
    {
    ::MessageBeep (MB_ICONEXCLAMATION);
    return;
    }

  // if at end of buffer, go back one character
  if (nEndChar == nLength)
    nEndChar--;
  else
    // if no delimiter on right, try going back one
    if (strchr (DELIMS, strBuf [nEndChar]) == NULL && 
        nEndChar > 0)
      nEndChar--;

  // character on right of cursor
  cStartDelim = strBuf [nEndChar];

   int iEscapeCount = 0;
   p = strBuf; 
   p += nEndChar;    // p points to start of search


   // if an odd number of escape characters, cannot use it
   while ((iFlags & PAREN_BACKSLASH_ESCAPES) && 
       p > pStartBuf && 
       *(p - 1) == '\\')
     {
     p--;
     iEscapeCount++;
     }

   if (iEscapeCount == 0)
     while ((iFlags & PAREN_PERCENT_ESCAPES) && 
         p > pStartBuf && 
         *(p - 1) == '%')
       {
       p--;
       iEscapeCount++;
       }

  // not on a delimiter, or delimiter preceded by an odd number of escapes? just exit
  if ((iEscapeCount & 1) || strchr (DELIMS, cStartDelim) == NULL)
    {
    ::MessageBeep (MB_ICONEXCLAMATION);
    return;
    }

  nStartChar = nEndChar;    // selection will start here
  p = strBuf; 
  p += nEndChar;    // p points to start of search
  pStart = p;       // where we started

  if (strchr (START_DELIMS, cStartDelim))
    {   // search forwards - work out end delimiter
     cNestArray [++iLevel] = GetMatchingDelimiter (cStartDelim);
     for (p ++ ; (c = *p) != 0; p++)
       {
       if (c == cNestArray [iLevel])    // reached end delimiter?
         {
         if (iLevel <= 0)
           break;   // found it!
         iLevel--;  // dropped back a level
         }
       else if ((iFlags & PAREN_NEST_BRACES) && (strchr (START_DELIMS, c)) ||
                !(iFlags & PAREN_NEST_BRACES) && (c == cStartDelim))
         {
         iLevel++;   // up a level, need another closing delimiter
         if (iLevel >= MAX_NEST)
          {
          ::MessageBeep (MB_ICONEXCLAMATION);
          return;
          }    // too many levels
         cNestArray [iLevel] = GetMatchingDelimiter (c);
         }
       else if (c == '\\' && (iFlags & PAREN_BACKSLASH_ESCAPES))
         {
         if (*(++p) == 0) // skip next character, if end of buffer, stop
           break;
         }    // end of backslash escape
       else if (c == '%' && (iFlags & PAREN_PERCENT_ESCAPES))
         {
         if (*(++p) == 0) // skip next character, if end of buffer, stop
           break;
         }    // end of percent escape
       else if ((c == '\"' && (iFlags & PAREN_DOUBLE_QUOTES)) || 
                (c == '\'' && (iFlags & PAREN_SINGLE_QUOTES)))
         {  // in quotes, skip quoted part
         for (p++; *p && *p != c; p++)
           {  // quote skipper
            if (*p == '\\' &&
               (iFlags & PAREN_BACKSLASH_ESCAPES) &&
             (((iFlags & PAREN_ESCAPE_SINGLE_QUOTES) && c == '\'') ||
              ((iFlags & PAREN_ESCAPE_DOUBLE_QUOTES) && c == '\"'))
               )   // can have escaped quotes inside quotes
             {
             if (*(++p) == 0) // skip next character, if end of buffer, stop
               break;
             }  // end of backslash escape
            else if (*p == '%' &&
               (iFlags & PAREN_PERCENT_ESCAPES) &&
             (((iFlags & PAREN_ESCAPE_SINGLE_QUOTES) && c == '\'') ||
              ((iFlags & PAREN_ESCAPE_DOUBLE_QUOTES) && c == '\"'))
               )   // can have escaped quotes inside quotes
             {
             if (*(++p) == 0) // skip next character, if end of buffer, stop
               break;
             }  // end of percent escape
           }    // end of skipping quotes
         }  // end of having a quote
       }  // end of searching for delimiter
    
     if (*p == 0)
        {
        ::MessageBeep (MB_ICONEXCLAMATION);
        return;
        }    // no end delimiter found

      
     // work out where end character is
     nEndChar += p - pStart;

    }  // end of searching forwards

  else
    {  // go backwards
    bBackwards = true;
    cNestArray [++iLevel] = GetMatchingDelimiter (cStartDelim);
    for (p--; p >= pStartBuf; p--)
       {
       int iEscapeCount = 0;
       c = *p;

       // if an odd number of escape characters, ignore it
       while ((iFlags & PAREN_BACKSLASH_ESCAPES) && 
           p > pStartBuf && 
           *(p - 1) == '\\')
         {
         p--;
         iEscapeCount++;
         }

       if (iEscapeCount == 0)
         while ((iFlags & PAREN_PERCENT_ESCAPES) && 
             p > pStartBuf && 
             *(p - 1) == '%')
           {
           p--;
           iEscapeCount++;
           }

       if (iEscapeCount & 1)
         continue;
       
       if (c == cNestArray [iLevel])
         {
         if (iLevel <= 0)
           {
           p += iEscapeCount;
           break;   // found it!
           }
         iLevel--;  // dropped back a level
         }
       else 
// ----- Begin Talek suggestion
        if ((iFlags & PAREN_NEST_BRACES) && (strchr (START_DELIMS, c)))
          {
          // we must have had an unmatched closing paren - remove it from the stack
         if (iLevel <= 0)  // no more levels - error in string
            {
            ::MessageBeep (MB_ICONEXCLAMATION);
            return;
            }    // too many levels
         iLevel--;  // dropped back a level
         p++;   // reconsider this character next time through the loop
         }
        else

// ----- End Talek suggestion         
         
         
         if ((iFlags & PAREN_NEST_BRACES) && (strchr (END_DELIMS, c)) ||
                !(iFlags & PAREN_NEST_BRACES) && (c == cStartDelim))
         {
         iLevel++;   // up a level, need another closing delimiter
         if (iLevel >= MAX_NEST)
          {
          ::MessageBeep (MB_ICONEXCLAMATION);
          return;
          }    // too many levels
         cNestArray [iLevel] = GetMatchingDelimiter (c);
         }
       else if ((c == '\"' && (iFlags & PAREN_DOUBLE_QUOTES)) || 
                (c == '\'' && (iFlags & PAREN_SINGLE_QUOTES)))
         {  // in quotes, skip quoted part
         for (p--; p >= pStartBuf; p--)
           {  // quote skipper
           // if escaped character, ignore it
            if ((p > pStartBuf && *(p - 1) == '\\') &&
               (iFlags & PAREN_BACKSLASH_ESCAPES) &&
             (((iFlags & PAREN_ESCAPE_SINGLE_QUOTES) && c == '\'') ||
              ((iFlags & PAREN_ESCAPE_DOUBLE_QUOTES) && c == '\"'))
               )   // can have escaped quotes inside quotes
              p--;
            else if ((p > pStartBuf && *(p - 1) == '%') &&
               (iFlags & PAREN_PERCENT_ESCAPES) &&
             (((iFlags & PAREN_ESCAPE_SINGLE_QUOTES) && c == '\'') ||
              ((iFlags & PAREN_ESCAPE_DOUBLE_QUOTES) && c == '\"'))
               )   // can have escaped quotes inside quotes
              p--;
           else
             if (*p == c)
               break;
           }    // end of skipping quotes
         }  // end of having a quote
       }  // end of searching for delimiter
    
     if (p < pStartBuf)
        {
        ::MessageBeep (MB_ICONEXCLAMATION);
        return;
        }    // no end delimiter found
      
     // work out where end character is
     nStartChar -= pStart - p;

    }   // end of searching backwards

  if (bSelect)
    nEndChar++;   // select end character as well
  else
    if (bBackwards)
      nEndChar = nStartChar;    // make insertion point only
    else
      nStartChar = nEndChar;    // make insertion point only

  ctlEdit.SetSel(nStartChar, nEndChar);

  //  (aaa[bbb]ccc)[ddd{e']'ee}]  
  //  (aaaa "bb \" bb" aaaa) 
}  // end of FindMatchingBrace


void GetButtonSize (CWnd & ctlWnd, int & iHeight, int & iWidth)
  {
  WINDOWPLACEMENT wndpl;

  // where is button?
  ctlWnd.GetWindowPlacement (&wndpl);

  // find vertical size
  iHeight = wndpl.rcNormalPosition.bottom - wndpl.rcNormalPosition.top;
  iWidth = wndpl.rcNormalPosition.right - wndpl.rcNormalPosition.left;

  } // end of GetButtonSize

// Lua helper utilities

// make string table item
void MakeTableItem (lua_State *L, const char * name, const CString & str)
  {
  lua_pushstring (L, name);
  lua_pushstring (L, str);
  lua_rawset(L, -3);
  }

// make string table item
void MakeTableItem (lua_State *L, const char * name, const string & str)
  {
  lua_pushstring (L, name);
  lua_pushlstring (L, str.c_str (), str.size ());
  lua_rawset(L, -3);
  }

// make number table item
void MakeTableItem (lua_State *L, const char * name, const double n)
  {
  lua_pushstring (L, name);
  lua_pushnumber (L, n);
  lua_rawset(L, -3);
  }

// make date table item
void MakeTableItem (lua_State *L, const char * name, const COleDateTime d)
  {
  lua_pushstring (L, name);
  lua_pushstring (L, d.Format (0, 0));
  lua_rawset(L, -3);
  }

// make boolean table item
void MakeTableItemBool (lua_State *L, const char * name, const bool b)
  {
  lua_pushstring (L, name);
  lua_pushboolean (L, b != 0);
  lua_rawset(L, -3);
  }

/*
 Methods:

  0 = nothing
  1 = invert
  2 = lighter
  3 = darker
  4 = less saturation
  5 = more saturation
  other = nothing
*/

long AdjustColour (long Colour, short Method) 
{
COLORREF c =  Colour & 0xFFFFFF;
CColor clr;

  clr.SetColor (c);

  switch (Method)
    {
    case ADJUST_COLOUR_INVERT: // invert
      return   RGB (255 - GetRValue (c),
                    255 - GetGValue (c),
                    255 - GetBValue (c));

    case ADJUST_COLOUR_LIGHTER: // lighter
      {
      float lum = clr.GetLuminance ();
      lum += 0.02f;
      if (lum > 1.0f)
        lum = 1.0f;
      clr.SetLuminance (lum);
      return clr;
      }
      break;

    case ADJUST_COLOUR_DARKER: // darker
      {
      float lum = clr.GetLuminance ();
      lum -= 0.02f;
      if (lum < 0.0f)
        lum = 0.0f;
      clr.SetLuminance (lum);
      return clr;
      }
      break;

    case ADJUST_COLOUR_LESS_COLOUR: // less colour
      {
      float sat = clr.GetSaturation ();
      sat -= 0.05f;
      if (sat < 0.0f)
        sat = 0.0f;
      clr.SetSaturation (sat);
      return clr;
      }
      break;

    case ADJUST_COLOUR_MORE_COLOUR: // more colour
      {
      float sat = clr.GetSaturation ();
      sat += 0.05f;
      if (sat > 1.0f)
        sat = 1.0f;
      clr.SetSaturation (sat);
      return clr;
      }
      break;

    default:
       return c;    // do nothing
      
    } // end of switch on Method


  return c;
}


void SetDefaultAnsiColours (COLORREF * normalcolour, COLORREF * boldcolour)
  {
  normalcolour [BLACK]   = RGB (0, 0, 0);
  normalcolour [RED]     = RGB (128, 0, 0);
  normalcolour [GREEN]   = RGB (0, 128, 0);
  normalcolour [YELLOW]  = RGB (128, 128, 0);
  normalcolour [BLUE]    = RGB (0, 0, 128);
  normalcolour [MAGENTA] = RGB (128, 0, 128);
  normalcolour [CYAN]    = RGB (0, 128, 128);
  normalcolour [WHITE]   = RGB (192, 192, 192);
                       
  boldcolour  [BLACK]    = RGB (128, 128, 128);            
  boldcolour  [RED]      = RGB (255, 0, 0);          
  boldcolour  [GREEN]    = RGB (0, 255, 0);          
  boldcolour  [YELLOW]   = RGB (255, 255, 0);        
  boldcolour  [BLUE]     = RGB (0, 0, 255);          
  boldcolour  [MAGENTA]  = RGB (255, 0, 255);        
  boldcolour  [CYAN]     = RGB (0, 255, 255);        
  boldcolour  [WHITE]    = RGB (255, 255, 255);      
  }   // SetDefaultAnsiColours

void SetDefaultCustomColours (COLORREF * customtext, COLORREF * customback)
  {

  for (int i = 0; i < MAX_CUSTOM; i++)
    {
    customtext [i] = RGB (255, 255, 255);   // white on black
    customback [i] = RGB (0, 0, 0);
    }

  // set up some nice defaults
  customtext [0] = RGB (255, 128, 128);
  customtext [1] = RGB (255, 255, 128);
  customtext [2] = RGB (128, 255, 128);
  customtext [3] = RGB (128, 255, 255);
  customtext [4] = RGB (0, 128, 255);
  customtext [5] = RGB (255, 128, 192);
  customtext [6] = RGB (255, 0, 0);
  customtext [7] = RGB (0, 128, 192);
  customtext [8] = RGB (255, 0, 255);
  customtext [9] = RGB (128, 64, 64);
  customtext [10] = RGB (255, 128, 64);
  customtext [11] = RGB (0, 128, 128);
  customtext [12] = RGB (0, 64, 128);
  customtext [13] = RGB (255, 0, 128);
  customtext [14] = RGB (0, 128, 0);
  customtext [15] = RGB (0, 0, 255);

  } // end of SetDefaultCustomColours


bool GetNestedFunction (lua_State * L, const char * sName, const bool bRaiseError)
  {
  vector<string> v;
  StringToVector (sName, v, ".", true);
  lua_pushvalue(L, LUA_GLOBALSINDEX);
  vector<string>::const_iterator iter = v.begin ();
  string sPrev, sItem;

  sPrev = "_G";

  for (;
       iter != v.end ();
       iter++)
         {
         sItem = *iter;     // remember current part of a.b.c.d (eg. b)

         if (!lua_istable (L, -1))  // exit loop if not table (eg. nil)
           break;
         lua_pushstring (L, (*iter).c_str ());    // get next element
         lua_gettable (L, -2);  // get from previous table
         lua_remove (L, -2);    // remove previous table (leaves new one on stack)
         sPrev = *iter;         // in case this is not a table
         }


  bool bResult = lua_isfunction (L, -1);

  // handle case of something like: print.blah
  //  -- loop above will exit and print is a function, but we wanted print.blah
  if (!bResult || (bResult && iter != v.end ()))
    {
    if (bResult)
      sItem = sPrev;

    if (bRaiseError)
      ::UMessageBox (TFormat ("Cannot find the function '%s' - item '%s' is %s",
                      sName, 
                      sItem.c_str (),
                      lua_typename(L, lua_type (L,-1))
                        ), 
                        MB_ICONEXCLAMATION); 
    return false;
    }

  return true;    // found it

  }

// look to see if a function in the form string.gsub exists in the Lua space

bool FindLuaFunction (lua_State * L, const char * sName)

  {
  bool bResult = GetNestedFunction (L, sName, false);
  lua_settop(L, 0); // pop function and possible error message from stack
  return bResult;
  } // end of FindLuaFunction


// get a pathname and extract the directory from it
CString ExtractDirectory (CString str)
  {
  int iSlash = str.ReverseFind ('\\');
  if (iSlash == -1)
     str = ".\\";
  else
    str = str.Left (iSlash + 1);
  return str;
  } // end of ExtractDirectory

// if string ends in backslash, get rid of it
CString RemoveFinalSlash (CString str)
  {

  if (!str.IsEmpty ())
    {
    if (str.Right (1) == '\\')
      str = str.Left (str.GetLength () - 1);
    }

  return str;

  } // end of RemoveFinalSlash


/***************************************************************

 Metaphone Algorithm

   Created by Lawrence Philips (location unknown). Metaphone presented
   in article in "Computer Language" December 1990 issue.

   Converted from Pick BASIC, as demonstrated in article, to C by
   Michael J. Kuhn (Baltimore, Maryland)

   My original intention was to replace SOUNDEX with METAPHONE in
   order to get lists of similar sounding names that were more precise.
   SOUNDEX maps "William" and "Williams" to the same values. METAPHONE
   as it turns out DOES THE SAME.  There are going to be problems
   that you need to resolve with your own set of data.

   Basically, for my problem with S's I think that if

      IF metaphone[strlen(metaphone)] == "S"
                                  AND strlen(metaphone) >= 4  THEN

           metaphone[strlen(metaphone)] = ""

   You can add you own rules as required.

   Also, Lawrence Philips suggests that for practical reasons only the
   first 4 characters of the metaphone be used. This happens to be the
   number of characters that Soundex produces. This is indeed practical
   if you already have reserved exactly 4 characters in your database.

   In addition an analysis of your data may show that names are split
   into undesirable "metaphone groups" as the number of metaphone characters
   increases.

             *********** BEGIN METAPHONE RULES ***********

 Lawrence Philips' RULES follow:

 The 16 consonant sounds:
                                             |--- ZERO represents "th"
                                             |
      B  X  S  K  J  T  F  H  L  M  N  P  R  0  W  Y

 Exceptions:

   Beginning of word: "ae-", "gn", "kn-", "pn-", "wr-"  ----> drop first letter
                      "Aebersold", "Gnagy", "Knuth", "Pniewski", "Wright"

   Beginning of word: "x"                                ----> change to "s"
                                      as in "Deng Xiaopeng"

   Beginning of word: "wh-"                              ----> change to "w"
                                      as in "Whalen"

 Transformations:

   B ----> B      unless at the end of word after "m", as in "dumb", "McComb"

   C ----> X      (sh) if "-cia-" or "-ch-"
           S      if "-ci-", "-ce-", or "-cy-"
                  SILENT if "-sci-", "-sce-", or "-scy-"
           K      otherwise, including in "-sch-"

   D ----> J      if in "-dge-", "-dgy-", or "-dgi-"
           T      otherwise

   F ----> F

   G ---->        SILENT if in "-gh-" and not at end or before a vowel
                            in "-gn" or "-gned"
                            in "-dge-" etc., as in above rule
           J      if before "i", or "e", or "y" if not double "gg"
           K      otherwise

   H ---->        SILENT if after vowel and no vowel follows
                         or after "-ch-", "-sh-", "-ph-", "-th-", "-gh-"
           H      otherwise

   J ----> J

   K ---->        SILENT if after "c"
           K      otherwise

   L ----> L

   M ----> M

   N ----> N

   P ----> F      if before "h"
           P      otherwise

   Q ----> K

   R ----> R

   S ----> X      (sh) if before "h" or in "-sio-" or "-sia-"
           S      otherwise

   T ----> X      (sh) if "-tia-" or "-tio-"
           0      (th) if before "h"
                  silent if in "-tch-"
           T      otherwise

   V ----> F

   W ---->        SILENT if not followed by a vowel
           W      if followed by a vowel

   X ----> KS

   Y ---->        SILENT if not followed by a vowel
           Y      if followed by a vowel

   Z ----> S

 **************************************************************/

/*

  NOTE: This list turned out to be various issues that I passed over
        while trying to discern this algorithm. The final outcome
        of these items may or may not be reflected in the code.

  There where some discrepancies between the Pick BASIC code in the
  original article and the verbal discription of the transformations:

     1. CASE SYMB = "G"

              AND ENAME[N +3] = "D" AND (N + 3) = L)) and ENAM
                                                             ^
                  this was cut off in the magazine listing   |

         I used the verbal discription in the transformation list
         to add the appropriate code.

     2.  H ---->        SILENT if after vowel and no vowel follows
                 H      otherwise

         This is the transformation description, however, the BASIC
         routine HAS code do this:

                      SILENT if after "ch-", "sh-", "ph-", "th", "gh"

         which is the correct behaviour if you look at c,s,p,t,g

         If did not, however, have "after vowel" coded even though this
         was in the description. I added it.

    3.   The BASIC code appears to skip double letters except "C" yet
         the transformation code for "G" looks at previous letter to
         see if we have "GG". This is inconsistent.

         I am making the assumption that "C" was a typo in the BASIC
         code. It should have been "G".

     4.  Transformation notation. "-..-" where .. are letters; means that
         the letters indicated are bounded by other letters. So "-gned"
         means at the end and "ch-" means at the beginning. I have noticed
         that the later is not explicity stated in the verbal description
         but it is coded in the BASIC.

     5.  case 'C'    K otherwise, including in "-sch-"
         this implies that "sch" be bounded by other letters. The BASIC
         code, however, has: N > 1
         It should have N > 2 for this to be correct.
               SCH-
               123    greater than 1 means that C can be 2nd letter

         I coded it as per the verbal description and not what was in
         the code.

     6.  as of 11-20-95 I am still trying to understand "H". The BASIC
         code seems to indicate that if "-.h" is at the end it is not
         silent. But if it is at the end there is no way a vowel could
         follow the "h". I am looking for examples.

      7. ok now I am really confused. Case "T". There is code in BASIC
         that says if next = "H" and previous != "T" . There is no
         way that a double T goes through the code. Double letters
         are dumped in the beginning.

              MATTHEW, MATTHIES, etc

         The first T goes through the second is skipped so the
         "th" is never detected.

         Modified routine to allow "G,T" duplicates through the switch.

       8. case "D"  -dge- is indicated in transformation
                    -dge- or -dge is coded.

            STEMBRIDGE should have "j" on end and not "t"

           I am leaving the code as is, verbal must be wrong.

       9. Regarding duplicate letters. "C" must be allowed through
          as in all of the McC... names.

          The way to handle "GG and "TT" I think is to pass over the
          first duplicate. The transformation rules would then handle
          duplicates of themselves by looking at the PREVIOUS letter.

          This solves the problems of "TTH" where you want the "th"
          sound.

       10. Change "CC" so that the metaphone character is "C", they
           way it is now for McComb and such you get "MKK", which
           unnecessiarly eats up and extra metaphone character.

       11. "TH" at the beginning as in Thomas. The verbal was not
           clear about this. I think is should be "T" and not "0"
           so I am changing code.

           After the first test I think that "THvowel" should be
           "0" and "TH(!vowel)" should be "T"

       12. I think throwing away 1 "S" and the end would be good.
           Since I am doing this anyway after the fact. If I
           do it before then names like. ..
                   BURROUGHS & BURROUGH would be the same
           because the GH would map to the same value in
           both cases.

       13. Case "Y", Brian and Bryan give different codes
           Don't know how to handle this yet.

       14. Comments on metaphone groups. Metaphone actually
           makes groups bigger. Names like:

                 C...R...  G...R...  K...R...  Q...R...

           will map to "KR". Soundex would have produced for example

           C600,C620,G600,G620,K600,K620,Q600,Q620

           the names from these 8 groups would have been collapsed into 1.

           Another way to look at this is for a more exact initial
           guess of a name Soundex would give you a smaller list of
           posibilities. If you don't know how to spell it at all
           however, your success at finding the right match with
           Metaphone is much greater than with Soundex.

      15. After some tests decided to leave S's at the end of the
          Metaphone. #12 takes care of my problems with plurals and
          then S gets used to help make distinct metaphone.

    Lawrence Philips is no longer at the company indicated in the
    article. So I was unable to verify these items.
*/


#define NULLCHAR (char *) 0

static char *VOWELS="AEIOU",
            *FRONTV="EIY",   /* special cases for letters in FRONT of these */
            *VARSON="CSPTG", /* variable sound--those modified by adding an "h"    */
            *ALLOW_DOUBLE=".";     /* let these double letters through */

static char *excpPAIR="AGKPW", /* exceptions "ae-", "gn-", "kn-", "pn-", "wr-" */
            *nextLTR ="ENNNR";
static char *chrptr, *chrptr1;

void metaphone (const char *name, char * metaph, int metalen)
  {
  
  int  ii, jj, silent, hard, Lng, lastChr;
  
  char curLtr, prevLtr, nextLtr, nextLtr2, nextLtr3;
  
  int vowelAfter, vowelBefore, frontvAfter;
  
  char wname[61];
  char *ename=wname;
  
  // initially empty
  metaph[0] = '\0';

  // don't allow overflow here
  if (metalen > (sizeof (wname) - 1))
    metalen = sizeof (wname) - 1;
  
  jj = 0;
  for (ii=0; name[ii] != '\0'; ii++) {
    if ( isalpha(name[ii]) ) {
      ename[jj] = toupper(name[ii]);
      jj++;
      }
    }
  ename[jj] = '\0';
  
  if (strlen(ename) == 0) 
    return;
  
  /* if ae, gn, kn, pn, wr then drop the first letter */
  if ( (chrptr=strchr(excpPAIR,ename[0]) ) != NULLCHAR ) {
    chrptr1 = nextLTR + (chrptr-excpPAIR);
    if ( *chrptr1 == ename[1] )  
      strcpy(ename,&ename[1]);
    }
  /* change x to s */
  if  (ename[0] == 'X') 
    ename[0] = 'S';

  /* get rid of the "h" in "wh" */
  if ( strncmp(ename,"WH",2) == 0 ) 
    strcpy(&ename[1], &ename[2]);
  
  Lng = strlen(ename);
  lastChr = Lng -1;   /* index to last character in string makes code easier*/
  
  /* Remove an S from the end of the string */
  if ( ename[lastChr] == 'S' ) {
    ename[lastChr] = '\0';
    Lng = strlen(ename);
    lastChr = Lng -1;
    }
  
  for (ii=0; ( (strlen(metaph) < metalen) && (ii < Lng) ); ii++) {
    
    curLtr = ename[ii];
    
    vowelBefore = FALSE;  prevLtr = ' ';
    if (ii > 0) {
      prevLtr = ename[ii-1];
      if ( strchr(VOWELS,prevLtr) != NULLCHAR ) 
        vowelBefore = TRUE;
      }
    /* if first letter is a vowel KEEP it */
    if (ii == 0 && (strchr(VOWELS,curLtr) != NULLCHAR) ) {
      strncat(metaph,&curLtr,1);
      continue;
      }
    
    vowelAfter = FALSE;   
    frontvAfter = FALSE;   
    nextLtr = ' ';

    if ( ii < lastChr ) {
      nextLtr = ename[ii+1];
      if ( strchr(VOWELS,nextLtr) != NULLCHAR ) 
        vowelAfter = TRUE;
      if ( strchr(FRONTV,nextLtr) != NULLCHAR ) 
        frontvAfter = TRUE;
      }

    /* skip double letters except ones in list */
    if (curLtr == nextLtr && (strchr(ALLOW_DOUBLE,nextLtr) == NULLCHAR) ) 
      continue;
    
    nextLtr2 = ' ';
    if (ii < (lastChr-1) ) 
      nextLtr2 = ename[ii+2];
    
    nextLtr3 = ' ';
    if (ii < (lastChr-2) ) 
      nextLtr3 = ename[ii+3];
    
    switch (curLtr) {
      
      case 'B': silent = FALSE;
        if (ii == lastChr && prevLtr == 'M') 
          silent = TRUE;
        if (! silent) 
          strncat(metaph,&curLtr,1);
        break;
        
        /*silent -sci-,-sce-,-scy-;  sci-, etc OK*/
      case 'C': if (! (ii > 1 && prevLtr == 'S' && frontvAfter) )
                  if ( ii > 0 && nextLtr == 'I' && nextLtr2 == 'A' )
                    strncat(metaph,"X",1);
                  else if (frontvAfter)
                      strncat(metaph,"S",1);
                  else if (ii > 1 && prevLtr == 'S' && nextLtr == 'H')
                    strncat(metaph,"K",1);
                  else if (nextLtr == 'H')
                      if (ii == 0 && (strchr(VOWELS,nextLtr2) == NULLCHAR) )
                        strncat(metaph,"K",1);
                      else
                        strncat(metaph,"X",1);
                      else if (prevLtr == 'C')
                        strncat(metaph,"C",1);
                      else
                        strncat(metaph,"K",1);
                      break;
                            
      case 'D': if (nextLtr == 'G' && (strchr(FRONTV,nextLtr2) != NULLCHAR))
                  strncat(metaph,"J",1);
        else
          strncat(metaph,"T",1);
        break;
        
      case 'G': silent=FALSE;
        /* SILENT -gh- except for -gh and no vowel after h */
        if ( (ii < (lastChr-1) && nextLtr == 'H')
          && (strchr(VOWELS,nextLtr2) == NULLCHAR) )
          silent=TRUE;
        
        if ( (ii == (lastChr-3) )
          && nextLtr == 'N' && nextLtr2 == 'E' && nextLtr3 == 'D')
          silent=TRUE;
        else
          if ( (ii == (lastChr-1)) && nextLtr == 'N') 
            silent=TRUE;
          
          if (prevLtr == 'D' && frontvAfter) 
            silent=TRUE;
          
          if (prevLtr == 'G')
            hard=TRUE;
          else
            hard=FALSE;
          
          if (!silent)
            if (frontvAfter && (! hard) )
              strncat(metaph,"J",1);
            else
              strncat(metaph,"K",1);
            break;
            
      case 'H': silent = FALSE;
        if ( strchr(VARSON,prevLtr) != NULLCHAR ) 
          silent = TRUE;
        
        if ( vowelBefore && !vowelAfter) 
          silent = TRUE;
        
        if (!silent) strncat(metaph,&curLtr,1);
        break;
        
      case 'F':
      case 'J':
      case 'L':
      case 'M':
      case 'N':
      case 'R': strncat(metaph,&curLtr,1);
        break;
        
      case 'K': if (prevLtr != 'C') 
                  strncat(metaph,&curLtr,1);
        break;
        
      case 'P': if (nextLtr == 'H')
                  strncat(metaph,"F",1);
        else
          strncat(metaph,"P",1);
        break;
        
      case 'Q': strncat(metaph,"K",1);
        break;
        
      case 'S': if (ii > 1 && nextLtr == 'I'
                  && ( nextLtr2 == 'O' || nextLtr2 == 'A') )
                  strncat(metaph,"X",1);
        else
          if (nextLtr == 'H')
            strncat(metaph,"X",1);
          else
            strncat(metaph,"S",1);
          break;
          
      case 'T': if (ii > 1 && nextLtr == 'I'
                  && ( nextLtr2 == 'O' || nextLtr2 == 'A') )
                  strncat(metaph,"X",1);
        else
          if (nextLtr == 'H')         /* The=0, Tho=T, Withrow=0 */
            if (ii > 0 || (strchr(VOWELS,nextLtr2) != NULLCHAR) )
              strncat(metaph,"0",1);
            else
              strncat(metaph,"T",1);
            else
              if (! (ii < (lastChr-2) && nextLtr == 'C' && nextLtr2 == 'H'))
                strncat(metaph,"T",1);
              break;
              
      case 'V': strncat(metaph,"F",1);
        break;
        
      case 'W':
      case 'Y': if (ii < lastChr && vowelAfter) 
                  strncat(metaph,&curLtr,1);
        break;
        
      case 'X': strncat(metaph,"KS",2);
        break;
        
      case 'Z': strncat(metaph,"S",1);
        break;
    }
    
  }
  
  /*  DON'T DO THIS NOW, REMOVING "S" IN BEGINNING HAS the same effect
  with plurals, in addition imbedded S's in the Metaphone are included
  Lng = strlen(metaph);
  lastChr = Lng -1;
  if ( metaph[lastChr] == 'S' && Lng >= 3 ) metaph[lastChr] = '\0';
  */
  
  return;
}


// for forum posts, where I am quoting code, I want to fix up
// [ ] and \ to have a \ in front of them

CString QuoteForumCodes(const CString & strText) 
{
CString strNewText;
const char * pOld;
char * pNew;
int iChanges = 0; 

  // count how many characters we see that need converting
  for (pOld = strText; *pOld; pOld++)
    if (*pOld == '[' ||
        *pOld == ']' ||
        *pOld == '\\')
      iChanges++;

  // allocate memory for new buffer
  pNew = strNewText.GetBuffer (strText.GetLength () + iChanges + 1);

  for (pOld = strText; *pOld; pOld++)
    {
    if (*pOld == '[' ||
        *pOld == ']' ||
        *pOld == '\\')
      *pNew++ = '\\';

    // now copy over whatever it was
    *pNew++ = *pOld;
    }
  
  *pNew++ = 0;    // terminator at end
  strNewText.ReleaseBuffer ();
  
  if (::UMessageBox (TFormat ("Clipboard converted for use with the Forum, %i change%s made",
      PLURAL (iChanges)), MB_ICONINFORMATION | MB_OKCANCEL) != IDOK)
      return strText;

  return strNewText;
  
  }  // end of QuoteForumCodes


// Levenshtein Distance Algorithm
// see: http://www.merriampark.com/ldcpp.htm

int EditDistance (const std::string source, const std::string target) 
  {

// keep maximum down in case they feed in a ridiculously long word
// runtime is proportional to O(mn)

const int MAX_LENGTH = 20;

  // Step 1

  const int n = minimum<int> (source.length(), MAX_LENGTH);
  const int m = minimum<int> (target.length(), MAX_LENGTH);

  int i, j;

  if (n == 0)
      return m;

  if (m == 0)
      return n;
  
  // Good form to declare a TYPEDEF

  typedef std::vector< std::vector<int> > Tmatrix; 

  Tmatrix matrix(n+1);

  // Size the vectors in the 2.nd dimension. Unfortunately C++ doesn't
  // allow for allocation on declaration of 2.nd dimension of vec of vec

  for (i = 0; i <= n; i++) {
    matrix[i].resize(m+1);
  }

  // Step 2

  for (i = 0; i <= n; i++) {
    matrix[i][0]=i;
  }

  for (j = 0; j <= m; j++) {
    matrix[0][j]=j;
  }

  // Step 3

  for (i = 1; i <= n; i++) {

    const char s_i = source[i-1];

    // Step 4

    for (j = 1; j <= m; j++) {

      const char t_j = target[j-1];

      // Step 5

      int cost;
      if (s_i == t_j) {
        cost = 0;
      }
      else {
        cost = 1;
      }

      // Step 6

      const int above = matrix[i-1][j];
      const int left = matrix[i][j-1];
      const int diag = matrix[i-1][j-1];
      int cell = min( above + 1, min(left + 1, diag + cost));

      // Step 6A: Cover transposition, in addition to deletion,
      // insertion and substitution. This step is taken from:
      // Berghel, Hal ; Roach, David : "An Extension of Ukkonen's 
      // Enhanced Dynamic Programming ASM Algorithm"
      // (http://www.acm.org/~hlb/publications/asm/asm.html)

      if (i>2 && j>2) {
        int trans=matrix[i-2][j-2]+1;
        if (source[i-2]!=t_j) trans++;
        if (s_i!=target[j-2]) trans++;
        if (cell>trans) cell=trans;
      }

      matrix[i][j]=cell;
    }
  }

  // Step 7

  return matrix[n][m];

}   // end of edit_distance



static void *l_alloc (void *ud, void *ptr, size_t osize, size_t nsize) {
  (void)ud;
  (void)osize;
  if (nsize == 0) {
    free(ptr);
    return NULL;
  }
  else
    return realloc(ptr, nsize);
}


// don't translate this, I think :)
static int panic (lua_State *L) {
  (void)L;  /* to avoid warnings */  
  ::AfxMessageBox (CFormat ("PANIC: unprotected error in call to Lua API (%s)",
                   lua_tostring(L, -1)));
  return 0;
}


lua_State *MakeLuaState (void) {
  lua_State *L = lua_newstate(l_alloc, NULL);
  if (L) lua_atpanic(L, &panic);
  return L;
}

// see lengthy forum post at: http://www.gammon.com.au/forum/?id=7776
//    (hello, Shadowfyr!)

/*

Basically the idea here is, that if the user has specified:
  * the default worlds directory
  * the default plugins directory; or
  * the default log file directory

  to have a *relative* path (eg. ./logs), then we will prepend the MUSHclient
  *startup* directory to the file name.

  Otherwise, if they happen to change directories, and then go to log something
  the default log file directory (if a relative path) won't work as expected.

  */

const char * Make_Absolute_Path (CString strFileName)
  {

// if the path does *not* start with one of:
//    x:  (ie. a drive);
//    /something (ie. a top level directory) ; or
//    \something (ie. a top level directory)
//  then we prepend the MUSHclient working directory

  if (!(
      isalpha ((unsigned char) strFileName [0]) &&
      strFileName [1] == ':'               // eg. c: blah
      ) &&      
      strFileName [0] != '\\' &&           // eg. \mydir\blah or \\server\blah
      strFileName [0] != '/'            // eg. /mydir/blah or //server/blah
     )  
    {
    // if the filename starts with ./ or .\ then drop it, otherwise
    //   we get junk like:  c:\mushclient\.\logs\filename

    if (strFileName.Left (2) == "./" ||
        strFileName.Left (2) == ".\\")
      strFileName = strFileName.Mid (2);

    strFileName = working_dir +  strFileName;
    }

// this is declared static for a very good reason - some of the places this is used
// the *pointer* is copied into a structure (for CFileDialog) and thus it must persist
// long enough for the dialog to run.
static char sPathName [_MAX_PATH];

  // ensure not too long
  strFileName = strFileName.Left (sizeof (sPathName) - 1);
  
  // copy to retain
  strcpy (sPathName, (const char *) strFileName);

  return sPathName;  

  } // end of Make_Absolute_Path

// checks a regular expression, raises a dialog if bad
bool CheckRegularExpression (const CString strRegexp, const int iOptions)
{
  const char* error;
  int erroroffset;
  if (t_regexp::CheckPattern(strRegexp, iOptions, &error, &erroroffset))
    return true; // It's valid!

  CRegexpProblemDlg dlg;
  dlg.m_strErrorMessage = Translate (error);
  dlg.m_strErrorMessage += ".";   // end the sentence

  // make first character upper-case, so it looks like a sentence. :)
  dlg.m_strErrorMessage.SetAt (0, toupper (dlg.m_strErrorMessage [0]));

  dlg.m_iColumn = erroroffset + 1;
  dlg.m_strColumn = TFormat ("Error occurred at column %i.", dlg.m_iColumn);

  dlg.m_strText = strRegexp;
  dlg.m_strText += ENDLINE;
  if (erroroffset > 0)
    dlg.m_strText += CString ('-', erroroffset - 1);
  dlg.m_strText += '^';

  dlg.DoModal ();
  return false;   // bad
}


// i18n (Internationalization) stuff

// translate message (eg. "File cannot be opened") into locale-specific language     
const char * TranslateGeneric (const char * sText, const char * sSection)
  {
  static bool bInTranslateGeneric = false;  // stop stack overflow if we are re-called

  const char * sResult = sText;   // default if not found is the source string

  // no translation file?
  if (App.m_Translator_Lua == NULL || bInTranslateGeneric)
    return sText;  // no file - just return source text

  bInTranslateGeneric = true;

  lua_settop (App.m_Translator_Lua, 0); // pop everything from last time

  lua_getglobal (App.m_Translator_Lua, sSection);  
  if (!lua_istable (App.m_Translator_Lua, -1))
    {
    bInTranslateGeneric = false;
    return sText;  // no messages table - just return source text
    }

  lua_getfield (App.m_Translator_Lua, -1, sText); 

  // if we found it, take result
  if (lua_isstring (App.m_Translator_Lua, -1))
    sResult = lua_tostring (App.m_Translator_Lua, -1);

  bInTranslateGeneric = false;

  // if string is not empty, use it
  if (sResult [0])
    return sResult;
  else
    return sText;
  }    // end of Translate

// translate message (eg. "File %s cannot be opened") into locale-specific language     

// This parses the format string, and works out how many arguments to supply to the Lua function

// Characters and strings will be supplied as string type, others will be numbers.

/*

    ["You have loaded %i %s%s"] = 
    function (a, b, c)
      return "Thou hast loaded " .. a .. " " .. b .. c
    end -- function 

*/

/* valid flags in a format specification */
#define FORMATFLAGS	"-+ #0"

const char * TFormat (const char * sFormat, ...)
  {

  static bool bInTFormat = false;   // stop stack overflow if we are re-called

  static CString strTranslated;

  bool bNotFound = App.m_Translator_Lua == NULL || bInTFormat;

  bInTFormat = true;

  if (!bNotFound)
    {
    lua_settop (App.m_Translator_Lua, 0); // pop everything from last time
    lua_getglobal (App.m_Translator_Lua, "formatted");
    }
  else 
    bNotFound = true;


  if (!bNotFound && lua_istable (App.m_Translator_Lua, -1))
    lua_getfield (App.m_Translator_Lua, -1, sFormat); 
  else 
    bNotFound = true;


  const char * sResult = NULL;

  // if we found it, pass arguments, and call it
  if (!bNotFound && lua_isfunction (App.m_Translator_Lua, -1))
    {

    va_list sstrings;
    va_start(sstrings, sFormat);
    int iArgCount = 0;
    
    // scan format string looking for % symbols
    const char * sFormatItem = sFormat;

    while (sFormatItem = strchr (sFormatItem, '%')) 
      {
      sFormatItem++;    // skip the %

      // skip special flags (
      while (strchr(FORMATFLAGS, *sFormatItem)) 
        sFormatItem++;  /* skip flags */
      if (isdigit(unsigned char (*sFormatItem))) 
        sFormatItem++;  /* skip width */
      if (isdigit(unsigned char (*sFormatItem))) 
        sFormatItem++;  /* (2 digits at most) */
      if (*sFormatItem == '.') 
        {
        sFormatItem++;
        if (isdigit(unsigned char (*sFormatItem))) 
          sFormatItem++;  /* skip precision */
        if (isdigit(unsigned char (*sFormatItem))) 
          sFormatItem++;  /* (2 digits at most) */
        }

      // Microsoft extensions which specify data width
      char    cWidth = 0;  // h = short, l = long, L = __int64

      if (*sFormatItem == 'l' || *sFormatItem == 'h')
        cWidth = *sFormatItem++;
      else
        if (memcmp (sFormatItem, "I64", 3) == 0)
          {
          cWidth = 'L';
          sFormatItem += 3; // skip I64
          }

      char cType = *sFormatItem++;    // type of field

      // having %% in the string does not require an argument
      if (cType == '%')
        continue;

      iArgCount++;

      // for very long sequences, check stack has enough room
      // Lua stack is supposed to have room for 20 items at least
      if (iArgCount >= 15)
        if (!lua_checkstack (App.m_Translator_Lua, 5))   // need at least 5 more, for safety
          {
          bNotFound = true;   // ack! no room
          break;
          }

      char*   sArg;
      char    cArg;
      double  dArg;

      switch (cType)
        {
        // single character (pushed as a number - see definition for %c)
        case 'c':  
          cArg = va_arg (sstrings, char);
          lua_pushinteger (App.m_Translator_Lua, cArg);
          break;

        // signed integers
        case 'd':  case 'i':
          switch (cWidth)
            {
            case 'h':
              dArg = va_arg (sstrings, short);
              break;

            case 'L':
              dArg = va_arg (sstrings, __int64);
              break;

            default:
              dArg = va_arg (sstrings, int);
              break;
            }
          lua_pushnumber (App.m_Translator_Lua, dArg);
          break;

        // unsigned integers
        case 'o':  case 'u':  case 'x':  case 'X': 
          switch (cWidth)
            {
            case 'h':
              dArg = va_arg (sstrings, unsigned short);
              break;

            case 'L':
              dArg = va_arg (sstrings, /* unsigned */ __int64);  // not implemented in this version
              break;

            default:
              dArg = va_arg (sstrings, unsigned int);
              break;
            }
          lua_pushnumber (App.m_Translator_Lua, dArg);
          break;

        case 'e':  case 'E': case 'f':
        case 'g': case 'G':
          dArg = va_arg (sstrings, double);
          lua_pushnumber (App.m_Translator_Lua, dArg);
          break;

        case 's':
          sArg = va_arg (sstrings, char *);
          lua_pushstring (App.m_Translator_Lua, sArg);
          break;

        // unknown, don't count it
        default:
          iArgCount--;
          break;

        } // end of switch

      } // end while loop
      

    va_end (sstrings);

    if (!bNotFound && lua_pcall (App.m_Translator_Lua, iArgCount, 1, 0))   // call with n args and 1 result
      {
      LuaError (App.m_Translator_Lua, "Calling TFormat");
      bNotFound = true;   // if error, ignore it
      }
    else
      {
      // function better have returned a string
      if (lua_isstring (App.m_Translator_Lua, -1))
        {
        sResult = lua_tostring (App.m_Translator_Lua, -1);
        // only use if it evaluates to a non-empty string
        if (sResult [0] == 0)
          bNotFound = true;
        }
      else
        bNotFound = true;   // not a string on stack

      }   // end of executed OK

    }   // end of function found
  else 
    bNotFound = true;

  // failure ? use standard formatting on string
  if (sResult == NULL || bNotFound)
    {
	  va_list argList;
	  va_start(argList, sFormat);
	  strTranslated = CFormat (sFormat, argList);
	  va_end(argList);
    bInTFormat = false;
    return strTranslated;
    }

  bInTFormat = false;
  return sResult;
  }    // end of TFormat

// display message box - using UTF-8
int UMessageBox (const char * sText, UINT nType, const char * sTitle)
  {

  int iBad;     // column in error in UTF8 validation
  int iLength;  // length of UTF8 message
  
  // convert message text to Unicode

  iBad = _pcre_valid_utf8 ((unsigned char  *) sText, strlen (sText));

  // use PCRE to validate the string first - if bad, fall back to standard box
  if (iBad >= 0)
    return ::AfxMessageBox (sText, nType);

  // find how big table has to be
  iLength = MultiByteToWideChar (CP_UTF8, 0, sText, -1, NULL, 0);

  // vector to hold Unicode message
  vector<WCHAR> vMessage;

  // adjust size
  vMessage.resize (iLength);

  // do the conversion now
  MultiByteToWideChar (CP_UTF8, 0, sText, -1, &vMessage [0], iLength);


  // now do title

  iBad = _pcre_valid_utf8 ((unsigned char  *) sTitle, strlen (sTitle));

  // use PCRE to validate the title next - if bad, use "MUSHclient" title
  if (iBad >= 0)
     sTitle = "MUSHclient";

  iLength = MultiByteToWideChar (CP_UTF8, 0, sTitle, -1, NULL, 0);

  // vector to hold Unicode title
  vector<WCHAR> vTitle;

  // adjust size
  vTitle.resize (iLength);
  
  // do the conversion now
  MultiByteToWideChar (CP_UTF8, 0, sTitle, -1, &vTitle [0], iLength);

  	// disable windows for modal dialog
	App.EnableModeless(FALSE);
	HWND hWndTop;
	HWND hWnd = CWnd::GetSafeOwner_(NULL, &hWndTop);

	// determine icon based on type specified
	if ((nType & MB_ICONMASK) == 0)
	{
		switch (nType & MB_TYPEMASK)
		{
		case MB_OK:
		case MB_OKCANCEL:
			nType |= MB_ICONEXCLAMATION;
			break;

		case MB_YESNO:
		case MB_YESNOCANCEL:
			nType |= MB_ICONEXCLAMATION;
			break;

		case MB_ABORTRETRYIGNORE:
		case MB_RETRYCANCEL:
			// No default icon for these types, since they are rarely used.
			// The caller should specify the icon.
			break;
		}
	}

  int nResult = ::MessageBoxW (hWnd, &vMessage [0], &vTitle [0], nType);

	// re-enable windows
	if (hWndTop != NULL)
		::EnableWindow(hWndTop, TRUE);
	App.EnableModeless(TRUE);

	return nResult;

  }   // end of UMessageBox

// display message box - translated
int TMessageBox (const char * sText, UINT nType)
  {
  return UMessageBox (Translate (sText), nType);
  }


bool SendToFrontFound;

/* callback function for EnumWindows */

static BOOL CALLBACK EnumWindowsProc (HWND hwnd, LPARAM lParam)
 {
 const char * name = (const char *) lParam;  /* wanted title */
 char buf [1024];

 /* find title of this window */

 if (GetWindowText (hwnd, buf, sizeof buf) == 0)
   return TRUE;  /* keep going */
 
 /* if title shorter than our string, can't match */
 if (strlen (buf) < strlen (name))
   return TRUE;  /* keep going */

 /* see if leading part of title matches */
 if (memcmp (buf, name, strlen (name)) == 0)
   {
   SetForegroundWindow (hwnd);  /* bring to foreground */
   SendToFrontFound = true;
   return FALSE;  
   }  /* found match */
 
 return TRUE;
   
 } /* end of EnumWindowsProc */

/* sends the nominated window to the front, based on the 
   leading characters in its name (first match) 

Example:

  SendToFront ("Crimson Editor")

*/

bool SendToFront (const char * name)
  {
  SendToFrontFound = false;
  EnumWindows (&EnumWindowsProc, (int) name);
  return SendToFrontFound;
  } /* end of SendToFront */


static void user_error_fn(png_structp png_ptr,
        png_const_charp error_msg)
  {
//  AfxMessageBox (error_msg);
  }

static void user_warning_fn(png_structp png_ptr,
    png_const_charp warning_msg)
  {
//  AfxMessageBox (warning_msg);

  }

static int BytesPerLine (int nWidth, int nBitsPerPixel)
  {
  return ( (nWidth * nBitsPerPixel + 31) & (~31) ) / 8;
  }

long LoadPng (LPCTSTR FileName, HBITMAP & hbmp)  // load a PNG file into a bitmap
  {
  // open file
  FILE *fp = fopen(FileName, "rb");
  if (!fp)
      return (eFileNotFound);

unsigned char header [8];
                  
  // read signature
  if (fread(header, 1, sizeof header, fp) != sizeof header)
    {
    fclose (fp);
    return (eUnableToLoadImage);    // file too small
    }

  // check signature
  if (png_sig_cmp(header, 0, sizeof header))
    {
    fclose (fp);
    return (eUnableToLoadImage);   // bad signature
    }

  // create PNG structure
  png_structp png_ptr = png_create_read_struct
       (PNG_LIBPNG_VER_STRING, NULL,
        user_error_fn, user_warning_fn);

  if (!png_ptr)
    {
    fclose (fp);
    return eUnableToLoadImage;
    }

  // create info structure
  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
    {
    png_destroy_read_struct(&png_ptr,
       (png_infopp)NULL, (png_infopp)NULL);
    fclose (fp);
    return eUnableToLoadImage;
    }

  // create end info structure
  png_infop end_info = png_create_info_struct(png_ptr);
  if (!end_info)
    {
    png_destroy_read_struct(&png_ptr, &info_ptr,
      (png_infopp)NULL);
    fclose (fp);
    return eUnableToLoadImage;
    }

#pragma warning (push)
#pragma warning (disable : 4611)  // interaction between '_setjmp' and C++ object destruction is non-portable

  // if png fails it will longjmp back to here, so we destroy the structure,
  // close the file, and wrap up
  if (setjmp(png_jmpbuf(png_ptr)))
    {
    png_destroy_read_struct(&png_ptr, &info_ptr,
       &end_info);
    fclose (fp);
    return eUnableToLoadImage;
    }

#pragma warning (pop)

  // initialize IO
  png_init_io (png_ptr, fp);

  // tell it we have read the sig
  png_set_sig_bytes(png_ptr, sizeof header);

  int png_transforms = PNG_TRANSFORM_STRIP_16 |    // Strip 16-bit samples to 8 bits
                       PNG_TRANSFORM_PACKING |     // Expand 1, 2 and 4-bit samples to bytes
                       PNG_TRANSFORM_EXPAND  |     // expand out to 8 bits if less
                       PNG_TRANSFORM_BGR;          // Flip RGB to BGR, RGBA to BGRA

  // read the file
  png_read_png(png_ptr, info_ptr, png_transforms, NULL);

  // note: png_read_end not needed with png_read_png (done already)

  // get the row pointers
  png_bytep * row_pointers = png_get_rows(png_ptr, info_ptr);

  // make a bitmap
  CDC B_DC;
  B_DC.CreateCompatibleDC (NULL);
  CBitmap B_bmp;

  BITMAPINFO bmiB;
  ZeroMemory (&bmiB, sizeof bmiB);

  int bit_per_pixel = 24;

  if (png_ptr->color_type & PNG_COLOR_MASK_ALPHA)
     bit_per_pixel = 32;

  long bpl = BytesPerLine (info_ptr->width, bit_per_pixel);

  bmiB.bmiHeader.biSize = sizeof bmiB;
  bmiB.bmiHeader.biWidth =          info_ptr->width;       
  bmiB.bmiHeader.biHeight =         info_ptr->height;
  bmiB.bmiHeader.biPlanes =         1;
  bmiB.bmiHeader.biBitCount =       bit_per_pixel;
  bmiB.bmiHeader.biCompression =    BI_RGB;
  bmiB.bmiHeader.biSizeImage =      info_ptr->height * bpl;

  unsigned char * pB = NULL;

  hbmp = CreateDIBSection(NULL, &bmiB, DIB_RGB_COLORS, (void**) &pB, NULL, 0);

  if (!hbmp)
    return eUnableToLoadImage;

  long row;
  unsigned char * p = pB;

  // have to reverse row order
  for (row = 0; row < info_ptr->height; row++, p += bpl)
     memcpy (p, row_pointers [info_ptr->height - row - 1], bpl);

  // done with data
  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

  // done with file
  fclose(fp);

  return eOK;
  }     // end of  LoadPng

typedef struct 
  {
  unsigned char * BufferPos;
  size_t LengthRemaining;
  } tPngBufferInfo;

void ReadDataFromInputStream(png_structp png_ptr, png_bytep outBytes,
                      png_size_t byteCountToRead)
{
   if(png_ptr->io_ptr == NULL)
     {
     png_error(png_ptr, "No io_ptr structure");
     return;   // oops
     }

   // get our structure with buffer and length left in it
   tPngBufferInfo * PngBufferInfo = (tPngBufferInfo*)png_ptr->io_ptr;

   if (byteCountToRead > PngBufferInfo->LengthRemaining)
     {
     png_error(png_ptr, "Insufficient data in buffer");
     return; 
     }

   // copy wanted bytes
   memcpy (outBytes, PngBufferInfo->BufferPos, byteCountToRead); 
   PngBufferInfo->BufferPos += byteCountToRead;
   PngBufferInfo->LengthRemaining -= byteCountToRead;

}  // end of ReadDataFromInputStream

long LoadPngMemory (unsigned char * Buffer, const size_t Length, HBITMAP & hbmp, const bool bAlpha)  // load a PNG file into a bitmap
  {

  if (Length < 8)
    return (eUnableToLoadImage);    // too small for signature

  // check signature
  if (png_sig_cmp (Buffer, 0, 8))
    return (eUnableToLoadImage);   // bad signature

  // create PNG structure
  png_structp png_ptr = png_create_read_struct
       (PNG_LIBPNG_VER_STRING, NULL,
        user_error_fn, user_warning_fn);

  if (!png_ptr)
    return eUnableToLoadImage;

  // create info structure
  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
    {
    png_destroy_read_struct(&png_ptr,
       (png_infopp)NULL, (png_infopp)NULL);
    return eUnableToLoadImage;
    }

  // create end info structure
  png_infop end_info = png_create_info_struct(png_ptr);
  if (!end_info)
    {
    png_destroy_read_struct(&png_ptr, &info_ptr,
      (png_infopp)NULL);
    return eUnableToLoadImage;
    }

#pragma warning (push)
#pragma warning (disable : 4611)  // interaction between '_setjmp' and C++ object destruction is non-portable

  // if png fails it will longjmp back to here, so we destroy the structure,
  // and wrap up
  if (setjmp(png_jmpbuf(png_ptr)))
    {
    png_destroy_read_struct(&png_ptr, &info_ptr,
       &end_info);
    return eUnableToLoadImage;
    }

#pragma warning (pop)

  tPngBufferInfo PngBufferInfo;
  PngBufferInfo.BufferPos = Buffer;
  PngBufferInfo.LengthRemaining = Length;

  // initialize IO
  png_set_read_fn(png_ptr, &PngBufferInfo, ReadDataFromInputStream);
  

  int png_transforms = PNG_TRANSFORM_STRIP_16 |    // Strip 16-bit samples to 8 bits
                       PNG_TRANSFORM_PACKING |     // Expand 1, 2 and 4-bit samples to bytes
                       PNG_TRANSFORM_BGR;          // Flip RGB to BGR, RGBA to BGRA


  if (bAlpha)
    png_transforms |= PNG_TRANSFORM_SWAP_ALPHA;

  // read the file
  png_read_png(png_ptr, info_ptr, png_transforms, NULL);

  // note: png_read_end not needed with png_read_png (done already)

  // get the row pointers
  png_bytep * row_pointers = png_get_rows(png_ptr, info_ptr);

  // make a bitmap
  CDC B_DC;
  B_DC.CreateCompatibleDC (NULL);
  CBitmap B_bmp;

  BITMAPINFO bmiB;
  ZeroMemory (&bmiB, sizeof bmiB);

  int bit_per_pixel = 24;

  if (png_ptr->color_type & PNG_COLOR_MASK_ALPHA)
     bit_per_pixel = 32;
  
  long bpl = BytesPerLine (info_ptr->width, bit_per_pixel);

  bmiB.bmiHeader.biSize = sizeof bmiB;
  bmiB.bmiHeader.biWidth =          info_ptr->width;       
  bmiB.bmiHeader.biHeight =         info_ptr->height;
  bmiB.bmiHeader.biPlanes =         1;
  bmiB.bmiHeader.biBitCount =       bit_per_pixel;
  bmiB.bmiHeader.biCompression =    BI_RGB;
  bmiB.bmiHeader.biSizeImage =      info_ptr->height * bpl;

  unsigned char * pB = NULL;

  hbmp = CreateDIBSection(NULL, &bmiB, DIB_RGB_COLORS, (void**) &pB, NULL, 0);

  if (!hbmp)
    return eUnableToLoadImage;

  long row;
  unsigned char * p = pB;

  // have to reverse row order
  for (row = 0; row < info_ptr->height; row++, p += bpl)
     memcpy (p, row_pointers [info_ptr->height - row - 1], bpl);

  // done with data
  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

  return eOK;
  }     // end of  LoadPngMemory


#define LONGDOUBLE_TYPE long double
#define u8 unsigned char


// based on sqlite3AtoF - converts string to double, regardless of locale

int myAtoF(const char *z, double *pResult)
  {
  int sign = 1;
  const char *zBegin = z;
  LONGDOUBLE_TYPE v1 = 0.0;
  int nSignificant = 0;
  while( isspace(*(u8*)z) ) z++;
  if( *z=='-' ){
    sign = -1;
    z++;
  }else if( *z=='+' ){
    z++;
  }
  while( z[0]=='0' ){
    z++;
  }
  while( isdigit(*(u8*)z) ){
    v1 = v1*10.0 + (*z - '0');
    z++;
    nSignificant++;
  }
  if( *z=='.' ){
    LONGDOUBLE_TYPE divisor = 1.0;
    z++;
    if( nSignificant==0 ){
      while( z[0]=='0' ){
        divisor *= 10.0;
        z++;
      }
    }
    while( isdigit(*(u8*)z) ){
      if( nSignificant<18 ){
        v1 = v1*10.0 + (*z - '0');
        divisor *= 10.0;
        nSignificant++;
      }
      z++;
    }
    v1 /= divisor;
  }
  if( *z=='e' || *z=='E' ){
    int esign = 1;
    int eval = 0;
    LONGDOUBLE_TYPE scale = 1.0;
    z++;
    if( *z=='-' ){
      esign = -1;
      z++;
    }else if( *z=='+' ){
      z++;
    }
    while( isdigit(*(u8*)z) ){
      eval = eval*10 + *z - '0';
      z++;
    }
    while( eval>=64 ){ scale *= 1.0e+64; eval -= 64; }
    while( eval>=16 ){ scale *= 1.0e+16; eval -= 16; }
    while( eval>=4 ){ scale *= 1.0e+4; eval -= 4; }
    while( eval>=1 ){ scale *= 1.0e+1; eval -= 1; }
    if( esign<0 ){
      v1 /= scale;
    }else{
      v1 *= scale;
    }
  }
  *pResult = (double)(sign<0 ? -v1 : v1);
  return (int)(z - zBegin);
}  // end of myAtoF


#include <direct.h>

// change to the current file browsing directory, browse for a file, put it back

extern char file_browsing_dir [_MAX_PATH];

void ChangeToFileBrowsingDirectory ()
  {

  _chdir(file_browsing_dir);

  }  // end of ChangeToFileBrowsingDirectory

void ChangeToStartupDirectory ()
  {

// first, remember the file_browsing directory 

  _getdcwd (0, file_browsing_dir, sizeof (file_browsing_dir) - 1);

// make sure directory name ends in a slash

  file_browsing_dir [sizeof (file_browsing_dir) - 2] = 0;

  if (file_browsing_dir [strlen (file_browsing_dir) - 1] != '\\')
    strcat (file_browsing_dir, "\\");  


  // now change back to startup directory
  _chdir(working_dir);

  } // end of ChangeToStartupDirectory


// quick check to see if a string is a plugin ID
bool IsPluginID (const char * sID)
  {
  // return if id wrong length
  if (strlen (sID) != PLUGIN_UNIQUE_ID_LENGTH)
    return false;

  for (const char * p = sID; *p; p++)
    if (!isxdigit (*p))
      return false;

  return true;
  }  // end of IsPluginID

// quick check to see if a string is a subroutine name
bool IsSubName (const char * sName)
  {
const char * p = sName;

  int i;
// check for A-Z, 0-9 or underscore
  for (i = 0; *p && *p != '('; p++, i++)
    if (!isalnum (*p) && *p != '_' && *p != '.')
      return false;

// can't be zero length
  if (i == 0)
    return false;

  return true;
  }  // end of IsSubName
