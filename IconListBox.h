#if !defined(AFX_ICONLISTBOX_H__1AFDF05D_179F_4269_887D_B47D0D7A11FD__INCLUDED_)
#define AFX_ICONLISTBOX_H__1AFDF05D_179F_4269_887D_B47D0D7A11FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// IconListBox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CIconListBox window

class CIconListBox : public CListBox
{

// Construction
public:
	enum Item_Pos{ITEM_RIGHT,ITEM_LEFT};
	CIconListBox();
	void Add(CString strItemName, HICON hDataIcon);
	void SetTextPosition(Item_Pos Txt_Pos);
	void SetIconPosition(Item_Pos Ico_Pos);
	void Destroy();

protected:
	// This is the Struct which holds the List Box Data
	struct DataItems
	{
		CString strItemData;
		HICON hIcon;
	};

	DataItems *m_pListBoxDataItems;
	Item_Pos m_TextPos;
	Item_Pos m_IconPos;
	CRect m_rectListBoxItem;//, m_rectListBoxItemText, m_rectListBoxItemIcon;



// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIconListBox)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CIconListBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CIconListBox)
	afx_msg void OnDropFiles(HDROP hDropInfo);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ICONLISTBOX_H__1AFDF05D_179F_4269_887D_B47D0D7A11FD__INCLUDED_)
