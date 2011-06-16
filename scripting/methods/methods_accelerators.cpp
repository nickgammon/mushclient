// methods_accelerators.cpp - stuff for the Accelerator command and variants

#include "stdafx.h"
#include "..\..\MUSHclient.h"
#include "..\..\doc.h"
#include "..\..\MUSHview.h"
#include "..\..\mainfrm.h"
#include "..\..\flags.h"
#include "..\errors.h"
#include "..\..\childfrm.h"
#include "..\..\sendvw.h"

// Implements:

//    Accelerator
//    AcceleratorList
//    AcceleratorTo


// accelerator finding functor

struct fFindAccelerator : binary_function<ACCEL, ACCEL, bool>
  {
  bool operator() (const ACCEL & a1, const ACCEL & a2) const
    { 
    return (a1.fVirt | FNOINVERT) == (a2.fVirt | FNOINVERT) &&
           a1.key == a2.key;
    };
  }; // end of fFindAccelerator


long CMUSHclientDoc::AcceleratorTo(LPCTSTR Key, LPCTSTR Send, short SendTo) 
{
BYTE fVirt;
WORD key;

  // check they are sending to somewhere valid

  if (SendTo < 0 || SendTo >= eSendToLast)
    return eOptionOutOfRange;

  try
    {
    StringToKeyCode (Key, fVirt, key);
    }
  catch (CException* e)
    {
    e->Delete ();
    return  eBadParameter;
    } // end of catch

       
  fVirt |= FNOINVERT;   // no inverting of menus

	POSITION pos = NULL;
  
  pos = GetFirstViewPosition();
	if (!pos)
		return eBadParameter;
  

  CMDIChildWnd * pMDIframe = NULL;

	// work out what our MDI frame is - that will have the accelerators in it

	CView* pView = GetNextView(pos);
	CWnd * pFrame =  pView->GetParentFrame ();
	pMDIframe = (CMDIChildWnd *) pFrame;

  vector <ACCEL> vAccel;
  HACCEL hAccel;

  hAccel = GetDefaultAccelerator ();

  // none for the document? Try the MDI frame
  if (hAccel == 0 && pMDIframe)
    hAccel = pMDIframe->GetDefaultAccelerator ();

  // try the multi-doc template
  if (hAccel == 0)
      hAccel = App.m_pWorldDocTemplate->m_hAccelTable;

	// find number of them
	int iCount = CopyAcceleratorTable (hAccel, NULL, 10000);
  vAccel.resize (iCount);   // make room, set "size" to iCount

	CopyAcceleratorTable (hAccel, &vAccel [0], iCount);

  ACCEL aWanted;            // new accelerator
  aWanted.fVirt = fVirt;
  aWanted.key = key;
  aWanted.cmd = 0;
  
  // if accelerator already exists, remove it
  vAccel.erase (remove_if (vAccel.begin (), vAccel.end (), bind2nd (fFindAccelerator (), aWanted)),
         vAccel.end ());

  // new in version 3.75 - if no Send string, simply delete accelerator
  if (Send [0]) 
    {
    // the command we will allocate (eg. 12000, 12001 etc.)
    WORD command;

    // make a flags/keystroke combination                                                      
    long virt_plus_key = ((long) fVirt) << 16 | key;

    // see if we have previously allocated that accelerator
    map<long, WORD>::const_iterator it = m_AcceleratorToCommandMap.find (virt_plus_key);

    if (it == m_AcceleratorToCommandMap.end ())
      {
      // check not too many ;)
      if (m_nextAcceleratorCommand >= (ACCELERATOR_FIRST_COMMAND + ACCELERATOR_COUNT))
        return eBadParameter;   // too many of them

      // allocate new one
      command = m_nextAcceleratorCommand++;
      }
    else
      command = it->second;  // re-use command

    aWanted.cmd = command;

    // put command into accelerator table
    vAccel.push_back (aWanted);

    // remember what the command was for that keystroke
    m_AcceleratorToCommandMap [virt_plus_key] = command;

    // remember what to send if they use that command
    m_CommandToMacroMap [command] = Send;

    // and where to send it
    m_CommandToSendToMap [command] = SendTo;

    // remember which plugin did it
    m_CommandToPluginMap [command].erase ();

    if (m_CurrentPlugin)
      m_CommandToPluginMap [command] = m_CurrentPlugin->m_strID;

    }  // end of having something to do (ie. not deleting accelerator)

  // create new handle
  hAccel = CreateAcceleratorTable (&vAccel [0], vAccel.size ());

  // destroy old one, if we had one
  if (m_accelerator)
    DestroyAcceleratorTable (m_accelerator);

  // replace accelerator table in document
  m_accelerator = hAccel;

  // test test test

//  App.m_pWorldDocTemplate->m_hAccelTable = hAccel;

	return eOK;
}  // end of CMUSHclientDoc::AcceleratorTo

long CMUSHclientDoc::Accelerator(LPCTSTR Key, LPCTSTR Send) 
{
  return AcceleratorTo (Key, Send, eSendToExecute);
}   // end of CMUSHclientDoc::Accelerator




// /Accelerator ("F5", "sing")
// /Note (Accelerator ("Alt+Shift+Ctrl+V", "hello"))
// /Note (Accelerator ("Ctrl+F8", "hello"))
// /Note (Accelerator ("Shift+F1", "hello"))

/*

/Note (Accelerator ("Ctrl+Alt+Shift+K", "Ctrl+Alt+Shift+K"))
Note (Accelerator ("Ctrl+K", "Ctrl+K"))
Note (Accelerator ("Shift+Ctrl+K", "Shift+Ctrl+K"))
Note (Accelerator ("Shift+Alt+K", "Shift+Alt+K"))
Note (Accelerator ("Ctrl+Alt+K", "Ctrl+Alt+K"))

table.foreach (AcceleratorList (), print)
*/


VARIANT CMUSHclientDoc::AcceleratorList() 
{
  COleSafeArray sa;   // for wildcard list

  long iCount = m_AcceleratorToCommandMap.size ();

  if (iCount) // cannot create empty array dimension
    {
    sa.CreateOneDim (VT_VARIANT, iCount);
  
    map<long, WORD>::const_iterator it;

    for (iCount = 0, it = m_AcceleratorToCommandMap.begin (); 
         it != m_AcceleratorToCommandMap.end (); it++, iCount++)
      {
      CString key = KeyCodeToString (it->first >> 16, it->first);
      string command = m_CommandToMacroMap [it->second];

      CString strSendTo;
      strSendTo.Format ("\t[%i]", m_CommandToSendToMap [it->second]);

      // for backwards compatability, send to Execute is shown as empty
      if (m_CommandToSendToMap [it->second] == eSendToExecute)
        strSendTo = "";
      // the array must be a bloody array of variants, or VBscript kicks up
      CString s = CFormat ("%s = %s%s", (LPCTSTR) key, 
                          command.c_str (), 
                          (LPCTSTR) strSendTo);
      COleVariant v (s);
      sa.PutElement (&iCount, &v);
      }      // end of looping through each accelerator
    } // end of having at least one

	return sa.Detach ();
}   // end of CMUSHclientDoc::AcceleratorList


