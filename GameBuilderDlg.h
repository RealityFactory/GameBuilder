// GameBuilderDlg.h : header file
//

#if !defined(AFX_GAMEBUILDERDLG_H__4FECD8A7_D9F3_11D5_B4D1_0060674A702D__INCLUDED_)
#define AFX_GAMEBUILDERDLG_H__4FECD8A7_D9F3_11D5_B4D1_0060674A702D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "vfile.h"
#include <afxtempl.h>
#include "TextProgressCtrl.h"
#include "IconListBox.h"

/////////////////////////////////////////////////////////////////////////////
// CGameBuilderDlg dialog

class CGameBuilderDlg : public CDialog
{
// Construction
public:
	CGameBuilderDlg(CWnd* pParent = NULL);	// standard constructor

	BOOL EmptyDirectory(CString &sPath);
	void GetRFINIOptions();
	void SetupDirectory();
	void Process();
	void CreateDir(int nFileType, char *szFilename);
	bool OpenRFFile(geVFile **theFp, int nFileType, char *szFilename, int nHow, bool ToDOS);

	void CopyVFile(geVFile * FmFile,geVFile *ToFile);
	int CopyToDos(int nFileType, char *szFilename, int Critical = 1);

	void CopyMain();
	void CopyRFIni();
	void CopyBuiltIn();
	void CopyMenu();
	void CopyCharacter();
	void Scan3DTforINI(BOOL needsattributefile, BOOL needshudfile, CString startlevel);
	void CopyEffect();
	void CopyPawn();
	void CopyMessage();
	void CopyInventory();
	void CopyEnvironment();
	void CopyPlayerSetup();
	void CopyWeapon();
	void CopyScript();
	void CopyConvScript();
	void CopyLevels();
	void CopyHud();
	void CopySplash();
	void CreateVFS();
	void GetDir(CString &Path, geVFile *VFS);
	void GetFiles(CString &Path, geVFile *VFS);
	bool MakeDirectory(geVFile *FS, const char *Path);
	void CopyFile(const char *srcPath, const char *destPath);
	void CopyOneFile(geVFile *FSSrc, geVFile *FSDest, const char *src, const char *dest);
	bool CheckCopy(char *filename);
	BOOL RemoveEmptyDirectory(CString &sPath);
	BOOL AContains(CArray<CString, CString> &A, CString S);
	void ASetAtFirstFreeIndex(CArray<CString, CString> &A, CString S);

	TCHAR m_currentdir[512];
	geVFile *VFS;

	CString	m_splash1;
	CString	m_splash2;
	CString	m_splashaudio1;
	CString	m_splashaudio2;
	CString	playeravatar;
	CString m_startlevel;
	bool character;
	CArray<CString, CString> Streaming;
	int Sindex;
	bool m_splashini;

	int m_copiedfiles;
	int m_missingfiles;
	int m_criticalfiles;

	bool m_cancelbuild;

	CArray<CString, CString> Levels;
	CArray<CString, CString> Menus;
	CArray<CString, CString> HUDs;
	CArray<CString, CString> Environments;
	CArray<CString, CString> PlayerSetups;
	CArray<CString, CString> Weapons;
	CArray<CString, CString> Scripts;
	CArray<CString, CString> ConvScripts;

	CString	m_LevelDir;
	CString	m_BitmapDir;
	CString	m_ActorDir;
	CString	m_AudioDir;
	CString	m_AudioStreamDir;
	CString	m_VideoDir;
	CString	m_MIDIDir;

// Dialog Data
	//{{AFX_DATA(CGameBuilderDlg)
	enum { IDD = IDD_GAMEBUILDER_DIALOG };
	CIconListBox	m_messagelist;
	CTextProgressCtrl	m_status;
	CString	m_packfile;
	CString	m_copytext;
	CString	m_menuini;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGameBuilderDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CGameBuilderDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnProcOk();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GAMEBUILDERDLG_H__4FECD8A7_D9F3_11D5_B4D1_0060674A702D__INCLUDED_)
