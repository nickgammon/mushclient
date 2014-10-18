// methods_colours.cpp

// To do with colour management

#include "stdafx.h"
#include "..\..\MUSHclient.h"
#include "..\..\doc.h"
#include "..\errors.h"

#include "..\..\dialogs\ColourPickerDlg.h"


// Implements:

//    ANSI
//    AdjustColour
//    ColourNameToRGB
//    GetCustomColourName
//    GetMapColour
//    MapColour
//    MapColourList
//    PickColour
//    RGBColourToName
//    SetCustomColourName


long CMUSHclientDoc::ColourNameToRGB(LPCTSTR Name) 
{
COLORREF cValue;

  if (SetColour (Name, cValue)) 
    return -1;

	return cValue;
}  // end of CMUSHclientDoc::ColourNameToRGB

BSTR CMUSHclientDoc::RGBColourToName(long Colour) 
{
	CString strResult = ColourToName (Colour);

	return strResult.AllocSysString();
}   // end of CMUSHclientDoc::RGBColourToName



/*

  Generate colour code ESC [ code m

  - intended for use with chat system where you send ANSI codes to other clients

// Misc
 0: reset
 1: bold
 4: underline
 7: inverse

22: cancel bold
24: cancel underline
27: cancel inverse

// Foreground
30: Black  
31: Red    
32: Green  
33: Yellow 
34: Blue   
35: Magenta
36: Cyan   
37: White  

// Background
40: Black  
41: Red    
42: Green  
43: Yellow 
44: Blue   
45: Magenta
46: Cyan   
47: White 
 
// 256 colour ANSI foreground
38;5;n  where n is 0 to 255

// 256 colour ANSI background
48;5;n  where n is 0 to 255

See: http://www.gammon.com.au/forum/?id=7761

*/


BSTR CMUSHclientDoc::ANSI(short Code) 
{
CString strResult = CFormat ("%c[%dm", ESC, Code); 

	return strResult.AllocSysString();
}  // end of MUSHclientDoc::ANSI


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

long CMUSHclientDoc::AdjustColour(long Colour, short Method) 
{
return ::AdjustColour (Colour, Method);
}   // end of CMUSHclientDoc::AdjustColour



long CMUSHclientDoc::GetNormalColour(short WhichColour) 
{
  if (WhichColour < 1 || WhichColour > 8)
    return 0;  
	return m_normalcolour [WhichColour - 1];
}  // end of CMUSHclientDoc::GetNormalColour

void CMUSHclientDoc::SetNormalColour(short WhichColour, long nNewValue) 
{
  if (WhichColour < 1 || WhichColour > 8)
    return;  
	m_normalcolour [WhichColour - 1] = nNewValue & 0x00FFFFFF;
}  // end of CMUSHclientDoc::SetNormalColour

long CMUSHclientDoc::GetBoldColour(short WhichColour) 
{
  if (WhichColour < 1 || WhichColour > 8)
    return 0;  
	return m_boldcolour [WhichColour - 1];
}   // end of CMUSHclientDoc::GetBoldColour

void CMUSHclientDoc::SetBoldColour(short WhichColour, long nNewValue) 
{
  if (WhichColour < 1 || WhichColour > 8)
    return;  
	m_boldcolour [WhichColour - 1] = nNewValue & 0x00FFFFFF;
}   // end of CMUSHclientDoc::SetBoldColour

long CMUSHclientDoc::GetCustomColourText(short WhichColour) 
{
  if (WhichColour < 1 || WhichColour > MAX_CUSTOM)
    return 0;  
	return m_customtext [WhichColour - 1];
}    // end of CMUSHclientDoc::GetCustomColourText

void CMUSHclientDoc::SetCustomColourText(short WhichColour, long nNewValue) 
{
  if (WhichColour < 1 || WhichColour > MAX_CUSTOM)
    return;  
	m_customtext [WhichColour - 1] = nNewValue & 0x00FFFFFF;
}   // end of CMUSHclientDoc::SetCustomColourText

long CMUSHclientDoc::GetCustomColourBackground(short WhichColour) 
{
  if (WhichColour < 1 || WhichColour > MAX_CUSTOM)
    return 0;  
	return m_customback [WhichColour - 1];
}   // end of CMUSHclientDoc::GetCustomColourBackground

void CMUSHclientDoc::SetCustomColourBackground(short WhichColour, long nNewValue) 
{
  if (WhichColour < 1 || WhichColour > MAX_CUSTOM)
    return;  
	m_customback [WhichColour - 1] = nNewValue & 0x00FFFFFF;
}    // end of CMUSHclientDoc::SetCustomColourBackground


void CMUSHclientDoc::MapColour(long Original, long Replacement) 
{
m_ColourTranslationMap [Original] = Replacement;
}    // end of CMUSHclientDoc::MapColour

// /MapColour (ColourNameToRGB ("blue"), ColourNameToRGB ("gold"))
// /MapColour (0, ColourNameToRGB ("red"))

long CMUSHclientDoc::GetMapColour(long Which) 
{
	return TranslateColour (Which);
}   // end of CMUSHclientDoc::GetMapColour

VARIANT CMUSHclientDoc::MapColourList() 
{
  COleSafeArray sa;   // for wildcard list

  long iCount = m_ColourTranslationMap.size ();

  if (iCount) // cannot create empty array dimension
    {
    sa.CreateOneDim (VT_VARIANT, iCount);
  
    map<COLORREF, COLORREF>::const_iterator it;

    for (iCount = 0, it = m_ColourTranslationMap.begin (); 
         it != m_ColourTranslationMap.end (); it++, iCount++)
      {
      // the array must be a bloody array of variants, or VBscript kicks up
      CString s = CFormat ("%s = %s", 
                          (LPCTSTR) ColourToName (it->first), 
                          (LPCTSTR) ColourToName (it->second));
      COleVariant v (s);
      sa.PutElement (&iCount, &v);
      }      // end of looping through each colour
    } // end of having at least one

	return sa.Detach ();

}  // end of CMUSHclientDoc::MapColourList


long CMUSHclientDoc::PickColour(long Suggested) 
{

  CColourPickerDlg dlg;

  if (Suggested != -1)
    {
    dlg.m_iColour = Suggested;
    dlg.m_bPickColour = true;
    }


  if (dlg.DoModal () != IDOK)
    return -1;

  return dlg.GetColor ();
}  // end of CMUSHclientDoc::PickColour


BSTR CMUSHclientDoc::GetCustomColourName(short WhichColour) 
{
	CString strResult;
  if (WhichColour >= 1 && WhichColour <= MAX_CUSTOM)
    strResult = m_strCustomColourName [WhichColour - 1];

	return strResult.AllocSysString();
}   // end of CMUSHclientDoc::GetCustomColourName

long CMUSHclientDoc::SetCustomColourName(short WhichColour, LPCTSTR Name) 
{
  if (WhichColour < 1 || WhichColour > MAX_CUSTOM)
    return eOptionOutOfRange;  

  if (strlen (Name) <= 0)
    return eNoNameSpecified;

  if (strlen (Name) > 30)
    return eInvalidObjectLabel;

  if (m_strCustomColourName [WhichColour - 1] != Name)
    SetModifiedFlag (TRUE);   // document has changed

  m_strCustomColourName [WhichColour - 1] = Name;

	return eOK;
}   // end of CMUSHclientDoc::SetCustomColourName
