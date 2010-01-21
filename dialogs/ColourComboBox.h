#if !defined(AFX_COLOURCOMBOBOX_H__3414F5A8_2628_11D4_BB0D_0080AD7972EF__INCLUDED_)
#define AFX_COLOURCOMBOBOX_H__3414F5A8_2628_11D4_BB0D_0080AD7972EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ColourComboBox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CColourComboBox window

class CColourComboBox : public CComboBox
{
// Construction
public:
	CColourComboBox();

// Attributes
public:

  // custom (user-defined) colours for triggers etc.
  COLORREF m_customtext [MAX_CUSTOM];
  COLORREF m_customback [MAX_CUSTOM];

  COLORREF m_iOtherForeground;
  COLORREF m_iOtherBackground;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColourComboBox)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CColourComboBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CColourComboBox)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLOURCOMBOBOX_H__3414F5A8_2628_11D4_BB0D_0080AD7972EF__INCLUDED_)
