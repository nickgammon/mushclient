// methods_panes.cpp

// Stuff for the not-implemented frames concept
//   (now effectively replaced by miniwindows)

#include "stdafx.h"
#include "..\..\MUSHclient.h"


#ifdef PANE
#if 0

// helper function to find a named pane
CPaneView * CMUSHclientDoc::GetPane (LPCTSTR Name)
  {
  PaneMapIterator pit = m_PaneMap.find (Name);
  if (pit != m_PaneMap.end ())
    return pit->second;

  return NULL;
  }

// test:  /world.CreatePane "nick", "mytitle", 10, 20, 100, 200, 0, 0, 16777215

long CMUSHclientDoc::CreatePane(LPCTSTR Name, 
                                LPCTSTR Title, 
                                long Left, 
                                long Top, 
                                long Width, 
                                long Height, 
                                long Flags, 
                                long TextColour, 
                                long BackColour, 
                                LPCTSTR FontName, 
                                short FontSize, 
                                long MaxLines, 
                                long WrapColumn) 
{

  if (GetPane (Name))
    return ePaneAlreadyExists;

  CRect r (Left, Top, Left + Width, Top + Height);

  /*
  CWnd * pWnd = new CWnd;
  pWnd->Create (NULL,     // class name
                Name, 
                WS_CAPTION  | WS_VISIBLE,
                r,       // rectangle
                &Frame,     // parent
                0,        // ID
                NULL);    // create context

  */

  /*
	CMDIChildWnd* pActiveChild = MDIGetActive();
	if (pActiveChild == NULL)
	{
		TRACE0("Warning: No active document for WindowNew command.\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return eBadParameter;     // command failed
	}

  */

	// otherwise we have a new frame !
	CFrameWnd* pFrame = App.m_pPaneTemplate->CreateNewFrame(this, NULL);
	if (pFrame == NULL)
	{
		TRACE0("Warning: failed to create new frame.\n");
		return eBadParameter;     // command failed
	}


	App.m_pPaneTemplate->InitialUpdateFrame(pFrame, this);

  CPaneView * pPane = (CPaneView *) pFrame->GetActiveView ();

  // remember pane parameters

  pPane->m_sTitle = Title;   
  pPane->m_iLeft = Left;    
  pPane->m_iTop = Top;     
  pPane->m_iWidth = Width;   
  pPane->m_iHeight = Height;  
  pPane->m_iFlags = Flags;
  pPane->m_sName = Name;
  pPane->m_cText = TextColour;
  pPane->m_cBack = BackColour;
  pPane->m_strFont = FontName;
  pPane->m_nHeight = FontSize;
  pPane->m_iMaxLines = MaxLines;
  pPane->m_iMaxWidth = WrapColumn;

  // get some reasonable values here
  if (pPane->m_sTitle.empty ())
    pPane->m_sTitle = Name;     // title defaults to name

  if (pPane->m_strFont.IsEmpty ())
    pPane->m_strFont = MUSHCLIENT_DEFAULT_FONT;

  if (pPane->m_nHeight < 3)
    pPane->m_nHeight = 12;

  if (pPane->m_iMaxLines < 1)
    pPane->m_iMaxLines = 1;

  if (pPane->m_iMaxWidth < 10)
    pPane->m_iMaxWidth = 10;

  // set flags
  pPane->m_bShowBold      = (pPane->m_iFlags & PANE_SHOWBOLD) != 0;
  pPane->m_bShowItalic    = (pPane->m_iFlags & PANE_SHOWITALIC) != 0;
  pPane->m_bShowUnderline = (pPane->m_iFlags & PANE_SHOWUNDERLINE) != 0;

  m_PaneMap [Name] = pPane;

  pFrame->SetWindowText (Title);

  // if a rectangle specified, move window
  if (Left | Top | Width | Height) 
    pFrame->MoveWindow (r);



  /*
  // testing
  if (pView)
    {

    pView->NoteLine (" __  __        ____ _ _            _ ");
    pView->NoteLine ("|  \\/  |_   _ / ___| (_) ___ _ __ | |_ ");
    pView->NoteLine ("| |\\/| | | | | |   | | |/ _ \\ '_ \\| __|");
    pView->NoteLine ("| |  | | |_| | |___| | |  __/ | | | |_ ");
    pView->NoteLine ("|_|  |_|\\__,_|\\____|_|_|\\___|_| |_|\\__|");
    }

  */

	return eOK;
}

long CMUSHclientDoc::ClosePane(LPCTSTR Name) 
{

  CPaneView * pPane = GetPane (Name);
  if (!pPane)
    return ePaneDoesNotExist;

  pPane->GetParentFrame ()->PostMessage (WM_CLOSE);

	return eOK;
}

// adds text to a pane, no linefeed
long CMUSHclientDoc::PaneText(LPCTSTR Name, 
                              LPCTSTR Text, 
                              long TextColour, 
                              long BackColour, 
                              long Style) 
{
  CPaneView * pPane = GetPane (Name);
  if (!pPane)
    return ePaneDoesNotExist;

  pPane->AddText (Text, TextColour, BackColour, Style, false); 

	return eOK;

}

// adds text to a pane, marks end of line
long CMUSHclientDoc::PaneLine(LPCTSTR Name, 
                              LPCTSTR Text, 
                              long TextColour, 
                              long BackColour, 
                              long Style) 
{
  CPaneView * pPane = GetPane (Name);
  if (!pPane)
    return ePaneDoesNotExist;

  pPane->AddText (Text, TextColour, BackColour, Style, true); 

	return eOK;
}


long CMUSHclientDoc::ClearPane(LPCTSTR Name) 
{
  CPaneView * pPane = GetPane (Name);
  if (!pPane)
    return ePaneDoesNotExist;

  pPane->Clear ();

	return eOK;
}

#endif // 0
#endif // PANE

