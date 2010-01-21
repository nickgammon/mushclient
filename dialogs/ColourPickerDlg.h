#if !defined(AFX_COLOURPICKERDLG_H__7FB4E349_5865_11D5_BB6E_0080AD7972EF__INCLUDED_)
#define AFX_COLOURPICKERDLG_H__7FB4E349_5865_11D5_BB6E_0080AD7972EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ColourPickerDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CColourPickerDlg dialog

class CColourPickerDlg : public CDialog
{
// Construction
public:
	CColourPickerDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CColourPickerDlg)
	enum { IDD = IDD_COLOUR_PICKER };
	CButton	m_ctlColourName;
	CButton	m_ctlVBname;
	CButton	m_ctlMXPname;
	CButton	m_ctlLuaName;
	CButton	m_ctlJscriptName;
	CSliderCtrl	m_ctlBlueSlider;
	CSliderCtrl	m_ctlGreenSlider;
	CSliderCtrl	m_ctlRedSlider;
	CButton	m_ctlOKbutton;
	CStatic	m_ctlLuminance;
	CStatic	m_ctlSaturation;
	CStatic	m_ctlHue;
	CComboBox	m_ctlColourSort;
	CListCtrl	m_ctlColourList;
	CStatic	m_ctlBlue;
	CStatic	m_ctlGreen;
	CStatic	m_ctlRed;
	CColourButton	m_ctlSwatch;
	//}}AFX_DATA


  COLORREF m_iColour;
  bool     m_bPickColour; // true to choose variant on m_iColour
  COLORREF m_iCustColour [16];   // custom colours

  COLORREF  m_clipboardColour;

  CString m_strMXPname;

  void ShowName (void);
  void SelectCurrentColour (void);

  COLORREF GetColor (void) { return m_iColour; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColourPickerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CColourPickerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelectColour();
	afx_msg void OnColourSwatch();
	afx_msg void OnDblclkColourlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedColourlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeColourSort();
	afx_msg void OnPaste();
	afx_msg void OnRandom();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnMxpName();
	afx_msg void OnVbName();
	afx_msg void OnJscriptName();
	afx_msg void OnLuaName();
	afx_msg void OnColourName();
	//}}AFX_MSG
  afx_msg void OnCustomdrawColourList ( NMHDR* pNMHDR, LRESULT* pResult );
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateNeedClipboard(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNeedName(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLOURPICKERDLG_H__7FB4E349_5865_11D5_BB6E_0080AD7972EF__INCLUDED_)
