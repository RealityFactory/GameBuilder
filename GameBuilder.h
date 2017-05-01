// GameBuilder.h : main header file for the GAMEBUILDER application
//

#if !defined(AFX_GAMEBUILDER_H__4FECD8A5_D9F3_11D5_B4D1_0060674A702D__INCLUDED_)
#define AFX_GAMEBUILDER_H__4FECD8A5_D9F3_11D5_B4D1_0060674A702D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CGameBuilderApp:
// See GameBuilder.cpp for the implementation of this class
//

class CGameBuilderApp : public CWinApp
{
public:
	CGameBuilderApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGameBuilderApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CGameBuilderApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GAMEBUILDER_H__4FECD8A5_D9F3_11D5_B4D1_0060674A702D__INCLUDED_)
