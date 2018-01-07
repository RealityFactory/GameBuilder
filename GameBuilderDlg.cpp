// GameBuilderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GameBuilder.h"
#include "GameBuilderDlg.h"
#include "MFileDlg.h"
#include "NEncryptKey.h"
#include "basetype.h"
#include "vfile.h"
#include <direct.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <afxtempl.h>
#include "TextProgressCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


const int kActorFile		= 0x0000;		///< ACTOR files
const int kAudioFile		= 0x0001;		///< AUDIO files
const int kVideoFile		= 0x0002;		///< VIDEO files
const int kMIDIFile			= 0x0003;		///< MIDI files
const int kLevelFile		= 0x0004;		///< LEVEL files
const int kAudioStreamFile	= 0x0005;		///< AUDIO STREAM files
const int kBitmapFile		= 0x0006;		///< BITMAP files
const int kSavegameFile		= 0x0007;		///< SAVE GAME files
const int kTempFile			= 0x0008;		///< Temporary files
const int kInstallFile		= 0x0009;		///< Install files (main directory)
const int kScriptFile		= 0x000a;		///< SCRIPT files
const int kBaseFile			= 0x000b;
const int kMenuJpgFile		= 0x000c;		///< Jpeg or jpg files in menu sub-dir
const int kRawFile			= 0x0fff;		///< RAW filename, don't modify


/////////////////////////////////////////////////////////////////////////////
// CGameBuilderDlg dialog

CGameBuilderDlg::CGameBuilderDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGameBuilderDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGameBuilderDlg)
	m_packfile = _T("");
	m_copytext = _T("");
	m_menuini = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGameBuilderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGameBuilderDlg)
	DDX_Control(pDX, IDC_LEVELS, m_messagelist);
	DDX_Control(pDX, IDC_PROGRESS, m_status);
	DDX_Text(pDX, IDC_VFS, m_packfile);
	DDX_Text(pDX, IDC_MENU, m_menuini);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CGameBuilderDlg, CDialog)
	//{{AFX_MSG_MAP(CGameBuilderDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, OnProcOk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGameBuilderDlg message handlers

BOOL CGameBuilderDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_splash1 = _T("");
	m_splash2 = _T("");
	m_splashaudio1 = _T("");
	m_splashaudio2 = _T("");
	playeravatar = _T("");
	character = false;

	m_LevelDir			= _T("media\\levels");
	m_BitmapDir			= _T("media\\bitmaps");
	m_ActorDir			= _T("media\\actors");
	m_AudioDir			= _T("media\\audio");
	m_AudioStreamDir	= _T("media\\audio");
	m_VideoDir			= _T("media\\video");
	m_MIDIDir			= _T("media\\midi");

	m_splashini = false;
	m_copiedfiles = 0;
	m_missingfiles = 0;
	m_criticalfiles = 0;

	m_cancelbuild = false;

	_chdir("..");
	_getcwd(m_currentdir, 512);


	m_messagelist.SetTextPosition(CIconListBox::ITEM_LEFT);
	m_messagelist.SetIconPosition(CIconListBox::ITEM_LEFT);
	Streaming.SetSize(10,-1);
	Sindex = 0;

	Levels.SetSize(10, -1);
	Menus.SetSize(5, -1);
	HUDs.SetSize(10, -1);
	Environments.SetSize(10, -1);
	PlayerSetups.SetSize(10, -1);
	Weapons.SetSize(10, -1);
	Scripts.SetSize(10, -1);
	ConvScripts.SetSize(10, -1);

	m_status.SetWindowText("");
	m_status.SetRange(0, 20);
	m_status.SetStep(1);
	m_status.SetPos(0);
	m_status.SetBarColour(RGB(90,128,196));

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CGameBuilderDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CGameBuilderDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

// deletes all files and subdirs and their files within a directoy
BOOL CGameBuilderDlg::EmptyDirectory(CString &sPath)
{
	CFileFind finder;

	CString  sWildCard = sPath + "\\*.*";

	BOOL bFound;
	BOOL bWorking = finder.FindFile(sWildCard);

	bFound = bWorking;

	while(bWorking)
	{
		bWorking = finder.FindNextFile();

		if(finder.IsDots()) continue;

		if(finder.IsDirectory())
		{
			CString s = finder.GetFilePath();
			EmptyDirectory(s);
			RemoveDirectory(finder.GetFilePath());
			continue;
		}

		_unlink(finder.GetFilePath());
	}

	return bFound;
}

BOOL CGameBuilderDlg::AContains(CArray<CString, CString> &A, CString S)
{
	int i;
	S.MakeLower();
	for(i=0; i<A.GetSize(); ++i)
	{
		if(A.GetAt(i) == S)
			return TRUE;
	}

	return FALSE;
}

void CGameBuilderDlg::ASetAtFirstFreeIndex(CArray<CString, CString> &A, CString S)
{
	int i;
	S.MakeLower();
	for(i=0; i<A.GetSize() && !(A.GetAt(i)).IsEmpty(); ++i)
	{
	}
	A.SetAtGrow(i, S);
}


void CGameBuilderDlg::SetupDirectory()
{
	CString sPath = m_currentdir;
	CString sMain, sFolder;

	sPath += "\\GameBuilder";
	EmptyDirectory(sPath);
	_rmdir(sPath);
	CreateDirectory(sPath, NULL);
	sPath += "\\";

	sMain = sPath + "install";
	CreateDirectory(sMain, NULL);

	sMain = sPath + "scripts";
	CreateDirectory(sMain, NULL);

	int pos;
	for(pos=0; (pos = m_ActorDir.Find("\\", pos)) != -1; ++pos)
	{
		sFolder = sPath + m_ActorDir.Left(pos);
		CreateDirectory(sFolder, NULL);
	}
	sFolder = sPath + m_ActorDir;
	CreateDirectory(sFolder, NULL);

	for(pos=0; (pos = m_LevelDir.Find("\\", pos)) != -1; ++pos)
	{
		sFolder = sPath + m_LevelDir.Left(pos);
		CreateDirectory(sFolder, NULL);
	}
	sFolder = sPath + m_LevelDir;
	CreateDirectory(sFolder, NULL);

	for(pos=0; (pos = m_BitmapDir.Find("\\", pos)) != -1; ++pos)
	{
		sFolder = sPath + m_BitmapDir.Left(pos);
		CreateDirectory(sFolder, NULL);
	}
	sFolder = sPath + m_BitmapDir;
	CreateDirectory(sFolder, NULL);

	sFolder = sPath + m_BitmapDir + "\\inventory";
	CreateDirectory(sFolder, NULL);

	sFolder = sPath + m_BitmapDir + "\\menu";
	CreateDirectory(sFolder, NULL);

	sFolder = sPath + m_BitmapDir + "\\fonts";
	CreateDirectory(sFolder, NULL);

	sFolder = sPath + m_BitmapDir + "\\fx";
	CreateDirectory(sFolder, NULL);

	sFolder = sPath + m_BitmapDir + "\\explode";
	CreateDirectory(sFolder, NULL);

	for(pos=0; (pos = m_AudioStreamDir.Find("\\", pos)) != -1; ++pos)
	{
		sFolder = sPath + m_AudioStreamDir.Left(pos);
		CreateDirectory(sFolder, NULL);
	}
	sFolder = sPath + m_AudioStreamDir;
	CreateDirectory(sFolder, NULL);

	for(pos=0; (pos = m_AudioDir.Find("\\", pos)) != -1; ++pos)
	{
		sFolder = sPath + m_AudioDir.Left(pos);
		CreateDirectory(sFolder, NULL);
	}
	sFolder = sPath + m_AudioDir;
	CreateDirectory(sFolder, NULL);

	sFolder = sPath + m_AudioDir + "\\menu";
	CreateDirectory(sFolder,NULL);

	for(pos=0; (pos = m_MIDIDir.Find("\\", pos)) != -1; ++pos)
	{
		sFolder = sPath + m_MIDIDir.Left(pos);
		CreateDirectory(sFolder, NULL);
	}
	sFolder = sPath + m_MIDIDir;
	CreateDirectory(sFolder,NULL);

	for(pos=0; (pos = m_VideoDir.Find("\\", pos)) != -1; ++pos)
	{
		sFolder = sPath + m_VideoDir.Left(pos);
		CreateDirectory(sFolder, NULL);
	}
	sFolder = sPath + m_VideoDir;
	CreateDirectory(sFolder,NULL);
}

void CGameBuilderDlg::GetRFINIOptions()
{
	FILE *fdInput = NULL;
	char szInputString[1024] = {""};
	char szOutputString[1024] = {""};
	char *szAtom = NULL;
	bool m_usefirst = false;
	bool m_usesecond = false;
	bool m_usefirstcut = false;
	bool m_usesecondcut = false;
	CString	Splash1 = "";
	CString	SplashAudio1 = "";
	CString	Cut1 = "";
	CString	Splash2 = "";
	CString	SplashAudio2 = "";
	CString	Cut2 = "";

	if((fdInput = fopen("RealityFactory.ini", "rt")) == NULL)
	{
		MessageBox("ERROR - RealityFactory.ini VANISHED! SERIOUS FATAL ERROR - EXITING!",
			"RealityFactory GameBuilder", MB_ICONSTOP | MB_OK);
		CDialog::OnOK();
		m_messagelist.Destroy();
		return;
	}

	while(fgets(szInputString, 512, fdInput) != NULL)
	{
		if(szInputString[0] == ';' || strlen(szInputString) <= 5)
			continue;
		strcpy(szOutputString, szInputString);
		szAtom = strtok(szOutputString, "=");
		if(szAtom == NULL)
			continue;
		if(!stricmp(szAtom, "LevelDirectory"))
		{
			szAtom = strtok(NULL, " \n");
			m_LevelDir = _T(szAtom);
			continue;
		}
		if(!stricmp(szAtom, "BitmapDirectory"))
		{
			szAtom = strtok(NULL, " \n");
			m_BitmapDir = _T(szAtom);
			continue;
		}
		if(!stricmp(szAtom, "ActorDirectory"))
		{
			szAtom = strtok(NULL, " \n");
			m_ActorDir = _T(szAtom);
			continue;
		}
		if(!stricmp(szAtom, "AudioDirectory"))
		{
			szAtom = strtok(NULL, " \n");
			m_AudioDir = _T(szAtom);
			continue;
		}
		if(!stricmp(szAtom, "AudioStreamDirectory"))
		{
			szAtom = strtok(NULL, " \n");
			m_AudioStreamDir = _T(szAtom);
			continue;
		}
		if(!stricmp(szAtom, "VideoDirectory"))
		{
			szAtom = strtok(NULL, " \n");
			m_VideoDir = _T(szAtom);
			continue;
		}
		if(!stricmp(szAtom, "MIDIDirectory"))
		{
			szAtom = strtok(NULL, " \n");
			m_MIDIDir = _T(szAtom);
			continue;
		}

		if(!stricmp(szAtom, "PackFile"))
		{
			szAtom = strtok(NULL, " \n");
			m_packfile = _T(szAtom);
			continue;
		}
		if(!stricmp(szAtom, "Menu"))
		{
			szAtom = strtok(NULL, " \n");
			m_menuini = _T(szAtom);
			continue;
		}
		if(!stricmp(szAtom, "PlayerAvatar"))
		{
			szAtom = strtok(NULL, " \n");
			playeravatar = _T(szAtom);
			continue;
		}
		if(!stricmp(szAtom, "StartLevel"))
		{
			szAtom = strtok(NULL, " \n");
			m_startlevel = _T(szAtom);
			continue;
		}
		if(!stricmp(szAtom, "UseCharSelect"))
		{
			szAtom = strtok(NULL, " \n");
			if(!stricmp(szAtom, "true"))
				character = true;
			continue;
		}
		if(!stricmp(szAtom, "UseFirst"))
		{
			szAtom = strtok(NULL, " \n");
			if(!stricmp(szAtom, "true"))
				m_usefirst = true;
			continue;
		}
		if(!stricmp(szAtom, "UseCutScene"))
		{
			szAtom = strtok(NULL, " \n");
			if(!stricmp(szAtom, "true"))
				m_usefirstcut = true;
			continue;
		}
		if(!stricmp(szAtom, "UseSecond"))
		{
			szAtom = strtok(NULL, " \n");
			if(!stricmp(szAtom, "true"))
				m_usesecond = true;
			continue;
		}
		if(!stricmp(szAtom, "UseCutScene1"))
		{
			szAtom = strtok(NULL, " \n");
			if(!stricmp(szAtom, "true"))
				m_usesecondcut = true;
			continue;
		}
		if(!stricmp(szAtom, "SplashScreen"))
		{
			szAtom = strtok(NULL, " \n");
			Splash1 = _T(szAtom);
			continue;
		}
		if(!stricmp(szAtom, "SplashAudio"))
		{
			szAtom = strtok(NULL, " \n");
			SplashAudio1 = _T(szAtom);
			continue;
		}
		if(!stricmp(szAtom, "CutScene"))
		{
			szAtom = strtok(NULL, " \n");
			Cut1 = _T(szAtom);
			continue;
		}
		if(!stricmp(szAtom, "SplashScreen1"))
		{
			szAtom = strtok(NULL, " \n");
			Splash2 = _T(szAtom);
			continue;
		}
		if(!stricmp(szAtom, "SplashAudio1"))
		{
			szAtom = strtok(NULL, " \n");
			SplashAudio2 = _T(szAtom);
			continue;
		}
		if(!stricmp(szAtom, "CutScene1"))
		{
			szAtom = strtok(NULL, " \n");
			Cut2 = _T(szAtom);
			continue;
		}
	}
	fclose(fdInput);

	if(m_usefirst)
	{
		if(m_usefirstcut)
		{
			m_splash1 = Cut1;
		}
		else
		{
			m_splash1 = Splash1;
			m_splashaudio1 = SplashAudio1;
		}
	}
	if(m_usesecond)
	{
		if(m_usesecondcut)
		{
			m_splash2 = Cut2;
		}
		else
		{
			m_splash2 = Splash2;
			m_splashaudio2 = SplashAudio2;
		}
	}
	UpdateData(FALSE);
}


void CGameBuilderDlg::CreateDir(int nFileType, char *szFilename)
{
	char szTemp[256];
	CString srcDir = szFilename;
	CString destDir = "";

	chdir(m_currentdir);
	int length = srcDir.Find('\\');
	while(length>0)
	{
		destDir = destDir + srcDir.Left(length);
		destDir = destDir + "\\";
		strcpy(szTemp, m_currentdir);
		strcat(szTemp, "\\GameBuilder\\");

		switch(nFileType)
		{
		case kActorFile:
			strcat(szTemp, m_ActorDir);
			strcat(szTemp, "\\");
			strcat(szTemp, destDir);
			break;
		case kAudioFile:
			strcat(szTemp, m_AudioDir);
			strcat(szTemp, "\\");
			strcat(szTemp, destDir);
			break;
		case kVideoFile:
			strcat(szTemp, m_VideoDir);
			strcat(szTemp, "\\");
			strcat(szTemp, destDir);
			break;
		case kMIDIFile:
			strcat(szTemp, m_MIDIDir);
			strcat(szTemp, "\\");
			strcat(szTemp, destDir);
			break;
		case kLevelFile:
			strcat(szTemp, m_LevelDir);
			strcat(szTemp, "\\");
			strcat(szTemp, destDir);
			break;
		case kAudioStreamFile:
			strcat(szTemp, m_AudioStreamDir);
			strcat(szTemp, "\\");
			strcat(szTemp, destDir);
			break;
		case kBitmapFile:
			strcat(szTemp, m_BitmapDir);
			strcat(szTemp, "\\");
			strcat(szTemp, destDir);
			break;
		case kInstallFile:
			strcat(szTemp, "install\\");
			strcat(szTemp, destDir);
			break;
		case kScriptFile:
			strcat(szTemp, "scripts\\");
			strcat(szTemp, destDir);
			break;
		case kMenuJpgFile:
			strcat(szTemp, m_BitmapDir);
			strcat(szTemp, "\\menu\\");
			strcat(szTemp, destDir);
			break;
		}
		CreateDirectory(szTemp, NULL);
		srcDir = srcDir.Mid(length+1);
		length = srcDir.Find('\\');
	}
}

bool CGameBuilderDlg::OpenRFFile(geVFile **theFp, int nFileType,
								 char *szFilename, int nHow, bool ToDOS)
{
	char szTemp[256];

	*theFp = NULL;
	chdir(m_currentdir);
	szTemp[0]='\0';
	if(ToDOS)
	{
		strcpy(szTemp, m_currentdir);
		strcat(szTemp, "\\GameBuilder\\");
	}

	switch(nFileType)
	{
    case kActorFile:
		strcat(szTemp, m_ActorDir);
		strcat(szTemp, "\\");
		strcat(szTemp, szFilename);
		break;
    case kAudioFile:
		strcat(szTemp, m_AudioDir);
		strcat(szTemp, "\\");
		strcat(szTemp, szFilename);
		break;
    case kVideoFile:
		strcat(szTemp, m_VideoDir);
		strcat(szTemp, "\\");
		strcat(szTemp, szFilename);
		break;
	case kMIDIFile:
		strcat(szTemp, m_MIDIDir);
		strcat(szTemp, "\\");
		strcat(szTemp, szFilename);
		break;
	case kLevelFile:
		strcat(szTemp, m_LevelDir);
		strcat(szTemp, "\\");
		strcat(szTemp, szFilename);
		break;
	case kAudioStreamFile:
		strcat(szTemp, m_AudioStreamDir);
		strcat(szTemp, "\\");
		strcat(szTemp, szFilename);
		break;
	case kBitmapFile:
		strcat(szTemp, m_BitmapDir);
		strcat(szTemp, "\\");
		strcat(szTemp, szFilename);
		break;
	case kInstallFile:
		strcat(szTemp, "install\\");
		strcat(szTemp, szFilename);
		break;
	case kScriptFile:
		strcat(szTemp, "scripts\\");
		strcat(szTemp, szFilename);
		break;
	case kMenuJpgFile:
		strcat(szTemp, m_BitmapDir);
		strcat(szTemp, "\\menu");
		strcat(szTemp, szFilename);
		break;
	case kBaseFile:
		strcat(szTemp, szFilename);
		break;
	}

	//	Ok, open the file up.

	*theFp = geVFile_OpenNewSystem(NULL, GE_VFILE_TYPE_DOS, szTemp, NULL, nHow);
	if(!(*theFp) && VFS && !ToDOS)
		*theFp = geVFile_Open(VFS, szTemp, nHow);

	//	If it worked, return true, otherwise return false.

	if(*theFp != NULL)
		return true;
	else
		return false;
}

void CGameBuilderDlg::Process()
{
	{
		GetRFINIOptions();
		SetupDirectory();

		CWaitCursor c;
		CFile sourceFile;
		BYTE buffer[256];
		DWORD dwRead;
		CFileException ex;
		TCHAR m_dir[512];

		strcpy(m_dir, m_currentdir);
		strcat(m_dir, "\\");
		strcat(m_dir, m_packfile);
		sourceFile.Open(m_dir, CFile::modeRead | CFile::shareDenyWrite, &ex);
		dwRead = sourceFile.Read(buffer, 4);
		sourceFile.Close();

		if(strnicmp((char *)buffer, "CF00", 4)==0)
		{
			CEncryptKey EKey;
			if(EKey.DoModal())
			{
				TCHAR encrypt[512];
				CString EnKey = EKey.m_encryptkey+"        ";
				EnKey = EnKey.Left(8);
				strcpy(encrypt, EnKey);
				VFS = geVFile_OpenNewSystem(NULL, GE_VFILE_TYPE_VIRTUAL, m_dir, encrypt, GE_VFILE_OPEN_READONLY | GE_VFILE_OPEN_DIRECTORY);
			}
			else
			{
				return;
			}
		}
		else
			VFS = geVFile_OpenNewSystem(NULL, GE_VFILE_TYPE_VIRTUAL, m_dir, NULL, GE_VFILE_OPEN_READONLY | GE_VFILE_OPEN_DIRECTORY);

		if(!VFS)
		{
			MessageBox("Unable to Get VFS for "+m_packfile, "RealityFactory VFS", MB_ICONSTOP | MB_OK);
			return;
		}

		CString str;
        str = "Processing Main Directory";
        m_status.SetWindowText(str);
		m_status.StepIt();
		CopyMain();
		if(m_cancelbuild) return;

		str = "Processing Setup File";
        m_status.SetWindowText(str);
		m_status.StepIt();
		CopyRFIni();
		if(m_cancelbuild) return;

		str = "Processing Required Resources";
        m_status.SetWindowText(str);
		m_status.StepIt();
		CopyBuiltIn();
		if(m_cancelbuild) return;

		str = "Processing Menu Resources";
        m_status.SetWindowText(str);
		m_status.StepIt();
		CopyMenu();
		if(m_cancelbuild) return;

		if(character)
		{
			str = "Processing Character Selection";
			m_status.SetWindowText(str);
			m_status.StepIt();
			CopyCharacter();
			if(m_cancelbuild) return;
		}
		else
			m_status.StepIt();

		str = "Processing Game Levels";
		m_status.SetWindowText(str);
		m_status.StepIt();
		CopyLevels();
		if(m_cancelbuild) return;

		str = "Processing HUD Resources";
        m_status.SetWindowText(str);
		m_status.StepIt();
		CopyHud();
		if(m_cancelbuild) return;

		str = "Processing Effects";
        m_status.SetWindowText(str);
		m_status.StepIt();
		CopyEffect();
		if(m_cancelbuild) return;

		str = "Processing Script files";
        m_status.SetWindowText(str);
		m_status.StepIt();
		CopyScript();
		if(m_cancelbuild) return;

		str = "Processing Conversation Script files";
        m_status.SetWindowText(str);
		m_status.StepIt();
		CopyConvScript();
		if(m_cancelbuild) return;

		str = "Processing Pawn.ini";
        m_status.SetWindowText(str);
		m_status.StepIt();
		CopyPawn();
		if(m_cancelbuild) return;

		str = "Processing Message.ini";
        m_status.SetWindowText(str);
		m_status.StepIt();
		CopyMessage();
		if(m_cancelbuild) return;

		str = "Processing Splash.ini";
        m_status.SetWindowText(str);
		m_status.StepIt();
		CopySplash();
		if(m_cancelbuild) return;

		str = "Processing Environment.ini";
        m_status.SetWindowText(str);
		m_status.StepIt();
		CopyEnvironment();
		if(m_cancelbuild) return;

		str = "Processing Inventory.ini";
        m_status.SetWindowText(str);
		m_status.StepIt();
		CopyInventory();
		if(m_cancelbuild) return;

		str = "Processing Player Data";
        m_status.SetWindowText(str);
		m_status.StepIt();
		CopyPlayerSetup();
		if(m_cancelbuild) return;

		str = "Processing Weapons";
        m_status.SetWindowText(str);
		m_status.StepIt();
		CopyWeapon();
		if(m_cancelbuild) return;

		str = "Creating VFS File";
        m_status.SetWindowText(str);
		m_status.StepIt();

		geVFile_Close(VFS);

		CreateVFS();
		m_status.StepIt();

		geVFile_CloseAPI();

		CString sPath = m_currentdir;
		sPath += "\\GameBuilder";
		RemoveEmptyDirectory(sPath);
		m_status.StepIt();

		str.Format("Copied files: %d    Missing files: %d (critical: %d)", m_copiedfiles, m_missingfiles, m_criticalfiles);
		m_messagelist.Add(str, AfxGetApp()->LoadStandardIcon(IDI_ASTERISK));
		MessageBox("Finished Creating Game  ", "Game Builder", MB_ICONINFORMATION | MB_OK);
	}
}

void CGameBuilderDlg::OnProcOk()
{
	// TODO: Add your control notification handler code here
	Process();
	if(m_cancelbuild)
	{
		CString sPath = m_currentdir;
		sPath += "\\GameBuilder";
		EmptyDirectory(sPath);
		_rmdir(sPath);
	}

	CDialog::OnOK();
	m_messagelist.Destroy();
}

void CGameBuilderDlg::CopyVFile(geVFile * FmFile,geVFile *ToFile)
{
	char CopyBuf[16384];
	int CopyBufLen = 16384;
	long Size;
	geVFile_Properties Props;

	if(!geVFile_GetProperties(FmFile, &Props))
		return;

	if(!geVFile_Size(FmFile, &Size))
		return;

	while(Size)
	{
		int CurLen = min(Size, CopyBufLen);

		if(!geVFile_Read(FmFile, CopyBuf, CurLen))
			return;

		if(!geVFile_Write(ToFile, CopyBuf, CurLen))
			return;

		Size -= CurLen;
	}
	return;
}

int CGameBuilderDlg::CopyToDos(int nFileType, char *szFilename, int Critical/* = 1*/)
{
	geVFile *MainFS, *SecondFS;
	CString File = szFilename;
	char tFile[256];
	int rval;

	File.Replace("\\\\", "\\");
	strcpy(tFile, File);

	if(OpenRFFile(&MainFS, nFileType, tFile, GE_VFILE_OPEN_READONLY, false))
	{
		CreateDir(nFileType, tFile);
		if(OpenRFFile(&SecondFS, nFileType, tFile, GE_VFILE_OPEN_CREATE, true))
		{
			CopyVFile(MainFS, SecondFS);
			geVFile_Close(SecondFS);
			if(nFileType == kAudioStreamFile)
			{
				int i;
				File.MakeLower();
				i = File.ReverseFind(_T('\\'));
				if(i != -1)
				{
					File = File.Right(File.GetLength()-i-1);
				}
				Streaming.SetAtGrow(Sindex, File);
				++Sindex;
			}
		}
		geVFile_Close(MainFS);
	}
	else
	{
		++m_missingfiles;

		if(Critical)
		{
			++m_criticalfiles;
			strcpy(tFile, "Missing critical file: ");
			strcat(tFile, File);
			m_messagelist.Add(tFile, AfxGetApp()->LoadStandardIcon(IDI_HAND));
			m_messagelist.UpdateWindow();
			strcat(tFile, "\nContinue build?");
			rval = MessageBox(tFile, "Warning", MB_ICONQUESTION | MB_OKCANCEL);
		}
		else
		{
			strcpy(tFile, "Missing file: ");
			strcat(tFile, File);
			m_messagelist.Add(tFile, AfxGetApp()->LoadStandardIcon(IDI_EXCLAMATION));
			m_messagelist.UpdateWindow();
		}

		if(rval == IDCANCEL)
			m_cancelbuild = true;
	}

	return rval;
}

void CGameBuilderDlg::CopyMain()
{
	TCHAR filename[512];

	if(CopyToDos(kBaseFile, "RealityFactory.exe")	== IDCANCEL) return;
	if(CopyToDos(kBaseFile, "VideoSetup.exe")		== IDCANCEL) return;

	if(CopyToDos(kBaseFile, "RealityFactory.ini")	== IDCANCEL) return;
	if(CopyToDos(kBaseFile, "d3d24.ini", 0)			== IDCANCEL) return;
	if(CopyToDos(kBaseFile, "keyboard.ini", 0)		== IDCANCEL) return;
	if(CopyToDos(kBaseFile, "setup.ini", 0)			== IDCANCEL) return;

	if(CopyToDos(kBaseFile, "D3D7xDrv.dll")			== IDCANCEL) return;
	if(CopyToDos(kBaseFile, "D3DDrv.dll")			== IDCANCEL) return;
	if(CopyToDos(kBaseFile, "OglDrv.dll")			== IDCANCEL) return;
	if(CopyToDos(kBaseFile, "decrypt.dll")			== IDCANCEL) return;
	if(CopyToDos(kBaseFile, "FreeImage.dll")		== IDCANCEL) return;
	if(CopyToDos(kBaseFile, "vorbisfile.dll")		== IDCANCEL) return;
	if(CopyToDos(kBaseFile, "vorbis.dll")			== IDCANCEL) return;
	if(CopyToDos(kBaseFile, "ogg.dll")				== IDCANCEL) return;

	if(CopyToDos(kInstallFile, "armour.ini")		== IDCANCEL) return;
	if(CopyToDos(kInstallFile, "camera.ini")		== IDCANCEL) return;
	if(CopyToDos(kInstallFile, "control.ini")		== IDCANCEL) return;
	if(CopyToDos(kInstallFile, "effect.ini")		== IDCANCEL) return;
	if(CopyToDos(kInstallFile, "explosion.ini")		== IDCANCEL) return;
	if(CopyToDos(kInstallFile, "inventory.ini")		== IDCANCEL) return;
	if(CopyToDos(kInstallFile, "material.ini")		== IDCANCEL) return;
	if(CopyToDos(kInstallFile, "message.ini")		== IDCANCEL) return;
	if(CopyToDos(kInstallFile, "pawn.ini")			== IDCANCEL) return;

	if(m_menuini == "")
		m_menuini = "menu.ini";
	strcpy(filename, m_menuini);
	if(CopyToDos(kInstallFile, filename)			== IDCANCEL) return;
    Menus.SetAtGrow(0, m_menuini);

	if(character)
	{
		if(CopyToDos(kInstallFile, "character.ini")		== IDCANCEL) return;
	}
	else
	{
		if(CopyToDos(kInstallFile, "environment.ini")	== IDCANCEL) return;
		Environments.SetAtGrow(0, "environment.ini");
		if(CopyToDos(kInstallFile, "playersetup.ini")	== IDCANCEL) return;
		PlayerSetups.SetAtGrow(0, "playersetup.ini");
		if(CopyToDos(kInstallFile, "weapon.ini")		== IDCANCEL) return;
		Weapons.SetAtGrow(0, "weapon.ini");
	}
}

void CGameBuilderDlg::CopyRFIni()
{
	TCHAR filename[512];

	if(m_splash1!="")
	{
		m_splash1.MakeLower();
		strcpy(filename, m_splash1);
		if(m_splash1.Find(".avi")!=-1 || m_splash1.Find(".gif")!=-1)
		{
			if(CopyToDos(kVideoFile, filename) == IDCANCEL) return;
		}
		else
		{
			if(CopyToDos(kBitmapFile, filename) == IDCANCEL) return;
		}
	}
	//else
	//	CopyToDos(kBitmapFile, "rflogo.bmp");

	if(m_splashaudio1!="")
	{
		m_splashaudio1.MakeLower();
		strcpy(filename, m_splashaudio1);
		if(CopyToDos(kAudioFile, filename) == IDCANCEL) return;
	}
	//else
	//	CopyToDos(kAudioFile, "startup.wav");

	if(m_splash2!="")
	{
		m_splash2.MakeLower();
		strcpy(filename, m_splash2);
		if(m_splash2.Find(".avi")!=-1 || m_splash2.Find(".gif")!=-1)
		{
			if(CopyToDos(kVideoFile, filename) == IDCANCEL) return;
		}
		else
		{
			if(CopyToDos(kBitmapFile, filename) == IDCANCEL) return;
		}
	}

	if(m_splashaudio2!="")
	{
		strcpy(filename, m_splashaudio2);
		if(CopyToDos(kAudioFile, filename) == IDCANCEL) return;
	}

	if(playeravatar!="" && !character)
	{
		strcpy(filename, playeravatar);
		if(CopyToDos(kActorFile, filename) == IDCANCEL) return;
	}
}

void CGameBuilderDlg::CopyBuiltIn()
{
	if(CopyToDos(kBitmapFile, "corona.bmp")		== IDCANCEL) return;
	if(CopyToDos(kBitmapFile, "a_corona.bmp")	== IDCANCEL) return;
	if(CopyToDos(kBitmapFile, "bolt.bmp")		== IDCANCEL) return;
	if(CopyToDos(kBitmapFile, "water.bmp")		== IDCANCEL) return;
	if(CopyToDos(kBitmapFile, "a_water.bmp")	== IDCANCEL) return;
	if(CopyToDos(kBitmapFile, "flame03.bmp")	== IDCANCEL) return;
	if(CopyToDos(kBitmapFile, "a_flame.bmp")	== IDCANCEL) return;
	if(CopyToDos(kBitmapFile, "g_bubble.bmp")	== IDCANCEL) return;
	if(CopyToDos(kBitmapFile, "a_bubble.bmp")	== IDCANCEL) return;
	if(CopyToDos(kBitmapFile, "rain.bmp")		== IDCANCEL) return;
	if(CopyToDos(kBitmapFile, "a_rain.bmp")		== IDCANCEL) return;
}


char *FirstToken(char *string1, char *string2)
{
	return strtok(string1, string2);
}

char *NextToken()
{
	char *temp;
	temp = strtok(NULL," \n");
	return temp;
}

void CGameBuilderDlg::CopyMenu()
{
	geVFile *SecondFS;
	TCHAR filename[512];
	BOOL defaultconversationtxt = TRUE;
	BOOL defaultmessagetxt = TRUE;

	int imenu;
	for(imenu=0; imenu<Menus.GetSize() && !(Menus.GetAt(imenu)).IsEmpty(); ++imenu)
	{
		strcpy(filename, Menus.GetAt(imenu));
		OpenRFFile(&SecondFS, kInstallFile, filename, GE_VFILE_OPEN_READONLY, true);
		char szInputLine[256];
		char menuline[256];
		char *szAtom;

		m_status.SetWindowText("Processing Menu: " + Menus.GetAt(imenu));

		while(geVFile_GetS(SecondFS, szInputLine, 256)==GE_TRUE)
		{
			if(szInputLine[0] == ';')
				continue;				// Comment line
			if(strlen(szInputLine) <= 5)
				continue;				// Skip blank lines

			// All config commands are "thing=value"
			szAtom = FirstToken(szInputLine,"=");
			if(!stricmp(szAtom,"background"))
			{
				NextToken();
				strcpy(menuline,"menu\\");
				strcat(menuline,NextToken());
				if(CopyToDos(kBitmapFile, menuline) == IDCANCEL) return;
			}
			else if(!stricmp(szAtom,"images"))
			{
				NextToken();
				strcpy(menuline,"menu\\");
				strcat(menuline,NextToken());
				if(CopyToDos(kBitmapFile, menuline) == IDCANCEL) return;
				strcpy(menuline,"menu\\");
				strcat(menuline,NextToken());
				if(CopyToDos(kBitmapFile, menuline) == IDCANCEL) return;
			}
			else if(!stricmp(szAtom,"titles"))
			{
				NextToken();
				strcpy(menuline,"menu\\");
				strcat(menuline,NextToken());
				if(CopyToDos(kBitmapFile, menuline) == IDCANCEL) return;
				strcpy(menuline,"menu\\");
				strcat(menuline,NextToken());
				if(CopyToDos(kBitmapFile, menuline) == IDCANCEL) return;
			}
			else if(!stricmp(szAtom,"animation"))
			{
				NextToken();
				strcpy(menuline,"menu\\");
				strcat(menuline,NextToken());
				if(CopyToDos(kVideoFile, menuline) == IDCANCEL) return;
			}
			else if(!stricmp(szAtom,"font"))
			{
				NextToken();
				char fontline[132];
				strcpy(fontline,NextToken());
				strcpy(menuline,"fonts\\");
				strcat(menuline,fontline);
				strcat(menuline,".bmp");
				if(CopyToDos(kBitmapFile, menuline) == IDCANCEL) return;
				strcpy(menuline,"fonts\\a_");
				strcat(menuline,fontline);
				strcat(menuline,".bmp");
				if(CopyToDos(kBitmapFile, menuline) == IDCANCEL) return;
				strcpy(menuline,"fonts\\");
				strcat(menuline,fontline);
				strcat(menuline,".dat");
				if(CopyToDos(kBitmapFile, menuline) == IDCANCEL) return;
			}
			else if(!stricmp(szAtom,"cursor"))
			{
				strcpy(menuline,"menu\\");
				strcat(menuline,NextToken());
				if(CopyToDos(kBitmapFile, menuline) == IDCANCEL) return;
				strcpy(menuline,"menu\\");
				strcat(menuline,NextToken());
				if(CopyToDos(kBitmapFile, menuline) == IDCANCEL) return;
			}
			else if(!stricmp(szAtom,"crosshair"))
			{
				strcpy(menuline,"menu\\");
				strcat(menuline,NextToken());
				if(CopyToDos(kBitmapFile, menuline) == IDCANCEL) return;
				strcpy(menuline,"menu\\");
				strcat(menuline,NextToken());
				if(CopyToDos(kBitmapFile, menuline) == IDCANCEL) return;
			}
			else if(!stricmp(szAtom,"loadscreen"))
			{
				strcpy(menuline,"menu\\");
				strcat(menuline,NextToken());
				if(CopyToDos(kBitmapFile, menuline) == IDCANCEL) return;
			}
			else if(!stricmp(szAtom,"mouseclick"))
			{
				char file[256] = "menu\\";
				strcat(file, NextToken());
				if(CopyToDos(kAudioFile, file) == IDCANCEL) return;
			}
			else if(!stricmp(szAtom,"keyclick"))
			{
				char file[256] = "menu\\";
				strcat(file, NextToken());
				if(CopyToDos(kAudioFile, file) == IDCANCEL) return;
			}
			else if(!stricmp(szAtom,"slideclick"))
			{
				char file[256] = "menu\\";
				strcat(file, NextToken());
				if(CopyToDos(kAudioFile, file) == IDCANCEL) return;
			}
			else if(!stricmp(szAtom,"music"))
			{
				char *musicname = NextToken();
				int len = strlen(musicname)-4;
				if(!stricmp((musicname+len),".mid"))
				{
					if(CopyToDos(kMIDIFile, musicname) == IDCANCEL) return;
				}
				else
				{
					if(CopyToDos(kAudioStreamFile, musicname) == IDCANCEL) return;
				}
			}
			else if(!stricmp(szAtom, "savegameimage"))
			{
				NextToken();NextToken();
				NextToken();NextToken();
				NextToken();NextToken();
				szAtom = NextToken();
				if(szAtom)
				{
					strcpy(menuline, szAtom);
					if(CopyToDos(kBitmapFile, menuline) == IDCANCEL) return;
				}
			}
			else if(imenu == 0  && !stricmp(szAtom, "menuinis"))
			{
				int i;
				for(i=0; i<5; i++)
				{
					strcpy(menuline, NextToken());
					if(stricmp(menuline, "default") && stricmp(menuline, "standard"))
					{
						if(!AContains(Menus, menuline))
						{
							ASetAtFirstFreeIndex(Menus, menuline);
							if(CopyToDos(kInstallFile, menuline) == IDCANCEL) return;
						}
					}
				}
			}
			else if(defaultconversationtxt && !stricmp(szAtom, "convtxts"))
			{
				int i;
				BOOL copieddefault = FALSE;
				for(i=0; i<5; i++)
				{
					strcpy(menuline, NextToken());
					if(!stricmp(menuline, "default") || !stricmp(menuline, "standard"))
					{
						if(!copieddefault)
						{
							if(CopyToDos(kInstallFile, "conversation.txt") == IDCANCEL) return;
							copieddefault = TRUE;
						}
					}
					else
					{
						if(CopyToDos(kInstallFile, menuline) == IDCANCEL) return;
					}
				}
				defaultconversationtxt = FALSE;
			}
			else if(defaultmessagetxt && !stricmp(szAtom, "messagetxts"))
			{
				int i;
				BOOL copieddefault = FALSE;
				for(i=0; i<5; i++)
				{
					strcpy(menuline, NextToken());
					if(!stricmp(menuline, "default") || !stricmp(menuline, "standard"))
					{
						if(!copieddefault)
						{
							if(CopyToDos(kInstallFile, "message.txt") == IDCANCEL) return;
							copieddefault = TRUE;
						}
					}
					else
					{
						if(CopyToDos(kInstallFile, menuline) == IDCANCEL) return;
					}
				}
				defaultmessagetxt = FALSE;
			}
		}

		geVFile_Close(SecondFS);
	}

	if(defaultconversationtxt)
	{
		if(CopyToDos(kInstallFile, "conversation.txt") == IDCANCEL) return;
	}
	if(defaultmessagetxt)
	{
		if(CopyToDos(kInstallFile, "message.txt") == IDCANCEL) return;
	}
}

void CGameBuilderDlg::CopyCharacter()
{
	CString readinfo;
	CString valuename, value;
	CString charname;
	CString startlevel;
	CString str;
	geVFile *MainFS;
	char szInputLine[256];
	char file[256];
	BOOL needsattributefile = FALSE;
	BOOL needshudfile = FALSE;
	BOOL needsweaponfile = FALSE;
	BOOL needsplayersetupfile = FALSE;
	BOOL needsenvironmentfile = FALSE;
	startlevel.Empty();

	OpenRFFile(&MainFS, kInstallFile, "character.ini", GE_VFILE_OPEN_READONLY, false);
	while(geVFile_GetS(MainFS, szInputLine, 256) == GE_TRUE)
	{
		if(strlen(szInputLine) <= 1)
			continue;
		readinfo = szInputLine;
		readinfo.TrimRight();

		if(readinfo != "")
		{
			if(readinfo[0] == '[' && readinfo[readinfo.GetLength()-1] == ']') //if a section heading
			{
				// finish previous character
				if((needsattributefile || needshudfile) && !startlevel.IsEmpty())
				{
					Scan3DTforINI(needsattributefile, needshudfile, startlevel);
				}
				if(needsweaponfile)
				{
					if(!AContains(Weapons, "weapon.ini"))
					{
						ASetAtFirstFreeIndex(Weapons, "weapon.ini");
						if(CopyToDos(kInstallFile, "weapon.ini") == IDCANCEL) return;
					}
				}
				if(needsplayersetupfile)
				{
					if(!AContains(PlayerSetups, "playersetup.ini"))
					{
						ASetAtFirstFreeIndex(PlayerSetups, "playersetup.ini");
						if(CopyToDos(kInstallFile, "playersetup.ini") == IDCANCEL) return;
					}
				}
				if(needsenvironmentfile)
				{
					if(!AContains(Environments, "environment.ini"))
					{
						ASetAtFirstFreeIndex(Environments, "environment.ini");
						if(CopyToDos(kInstallFile, "environment.ini") == IDCANCEL) return;
					}
				}

				readinfo.TrimLeft('[');
				readinfo.TrimRight(']');
				m_status.SetWindowText("Processing Character Selection :" + readinfo);

				needsattributefile = TRUE;
				needshudfile = TRUE;
				needsweaponfile = TRUE;
				needsplayersetupfile = TRUE;
				needsenvironmentfile = TRUE;
				continue;
			}

			if(readinfo[0] != ';')
			{
				valuename = readinfo.Left(readinfo.Find("="));
				value = readinfo.Right(readinfo.GetLength()-valuename.GetLength()-1);
				valuename.TrimLeft();
				valuename.TrimRight();
				value.TrimLeft();
				value.TrimRight();
				strcpy(file, value);
				if(valuename=="image")
				{
					if(CopyToDos(kBitmapFile, file) == IDCANCEL) return;
				}
				else if(valuename=="shadowbitmap")
				{
					if(!value.IsEmpty())
						if(CopyToDos(kBitmapFile, file) == IDCANCEL) return;
				}
				else if(valuename=="shadowalphamap")
				{
					if(!value.IsEmpty())
						if(CopyToDos(kBitmapFile, file) == IDCANCEL) return;
				}
				else if(valuename=="actorname")
				{
					if(!value.IsEmpty())
						if(CopyToDos(kActorFile, file) == IDCANCEL) return;
				}
				else if(valuename=="weaponfile")
				{
					if(!value.IsEmpty())
					{
						value.MakeLower();
						if(!AContains(Weapons, value))
						{
							ASetAtFirstFreeIndex(Weapons, value);
							if(CopyToDos(kInstallFile, file) == IDCANCEL) return;
						}
						needsweaponfile = FALSE;
					}
				}
				else if(valuename=="playersetupfile")
				{
					if(!value.IsEmpty())
					{
						value.MakeLower();
						if(!AContains(PlayerSetups, value))
						{
							ASetAtFirstFreeIndex(PlayerSetups, value);
							if(CopyToDos(kInstallFile, file) == IDCANCEL) return;
						}
						needsplayersetupfile = FALSE;
					}
				}
				else if(valuename=="environmentfile")
				{
					if(!value.IsEmpty())
					{
						value.MakeLower();
						if(!AContains(Environments, value))
						{
							ASetAtFirstFreeIndex(Environments, value);
							if(CopyToDos(kInstallFile, file) == IDCANCEL) return;
						}
						needsenvironmentfile = FALSE;
					}
				}
				else if(valuename=="hudfile")
				{
					if(!value.IsEmpty())
					{
						value.MakeLower();
						if(!AContains(HUDs, value))
						{
							ASetAtFirstFreeIndex(HUDs, value);
							if(CopyToDos(kInstallFile, file) == IDCANCEL) return;
						}
						needshudfile = FALSE;
					}
				}
				else if(valuename=="attributefile")
				{
					if(!value.IsEmpty())
					{
						if(CopyToDos(kInstallFile, file) == IDCANCEL) return;
						needsattributefile = FALSE;
					}
				}
				else if(valuename=="startlevel")
				{
					if(value.IsEmpty())
					{
						startlevel = m_startlevel;
					}
					else
					{
						startlevel = value;
					}
					startlevel.MakeLower();

					if(!AContains(Levels, startlevel))
					{
						ASetAtFirstFreeIndex(Levels, startlevel);
						strcpy(file, startlevel);
						if(CopyToDos(kLevelFile, file) == IDCANCEL) return;
					}
				}
			}
		}
	}
	geVFile_Close(MainFS);

	if((needsattributefile || needshudfile) && !startlevel.IsEmpty())
	{
		Scan3DTforINI(needsattributefile, needshudfile, startlevel);
	}
	if(needsweaponfile)
	{
		if(!AContains(Weapons, "weapon.ini"))
		{
			ASetAtFirstFreeIndex(Weapons, "weapon.ini");
			if(CopyToDos(kInstallFile, "weapon.ini") == IDCANCEL) return;
		}
	}
	if(needsplayersetupfile)
	{
		if(!AContains(PlayerSetups, "playersetup.ini"))
		{
			ASetAtFirstFreeIndex(PlayerSetups, "playersetup.ini");
			if(CopyToDos(kInstallFile, "playersetup.ini") == IDCANCEL) return;
		}
	}
	if(needsenvironmentfile)
	{
		if(!AContains(Environments, "environment.ini"))
		{
			ASetAtFirstFreeIndex(Environments, "environment.ini");
			if(CopyToDos(kInstallFile, "environment.ini") == IDCANCEL) return;
		}
	}
}

void CGameBuilderDlg::Scan3DTforINI(BOOL needsattributefile, BOOL needshudfile, CString startlevel)
{
	geVFile *Level;
	char szInputLine[132];
	char file[256];
	CString rinfo;

	startlevel.Replace(".bsp", ".3dt");
	strcpy(file, startlevel);
	OpenRFFile(&Level, kLevelFile, file, GE_VFILE_OPEN_READONLY, false);

	while((geVFile_GetS(Level, szInputLine, 132)) == GE_TRUE && (needsattributefile || needshudfile))
	{
		if(strlen(szInputLine) <= 1)
			continue;
		rinfo = szInputLine;
		rinfo.TrimRight();
		if(rinfo != "")
		{
			if(needsattributefile)
			{
				if(rinfo.Find("Key AttributeInfoFile Value ") != -1)
				{
					rinfo = rinfo.Mid(rinfo.Find('"')+1);
					rinfo.TrimRight('"');
					if(!rinfo.IsEmpty())
					{
						strcpy(file, rinfo);
						if(CopyToDos(kInstallFile, file) == IDCANCEL) return;
					}
					needsattributefile = FALSE;
					continue;
				}
			}
			if(needshudfile)
			{
				if(rinfo.Find("Key HUDInfoFile Value ") != -1)
				{
					rinfo = rinfo.Mid(rinfo.Find('"')+1);
					rinfo.TrimRight('"');
					if(!rinfo.IsEmpty())
					{
						strcpy(file, rinfo);
						if(!AContains(HUDs, rinfo))
						{
							ASetAtFirstFreeIndex(HUDs, rinfo);
							if(CopyToDos(kInstallFile, file) == IDCANCEL) return;
						}
					}
					needshudfile = FALSE;
					continue;
				}
			}
		}
	}
	geVFile_Close(Level);
}


void CGameBuilderDlg::CopyConvScript()
{
	CString str;
	CString str2;
	char file[256];
	char str1[256];
	char bsp[256];
	char szInputLine[256];
	geVFile *MainFS;
	int i, j;
	BOOL incomment;
	int iScript;

	for(iScript=0; iScript<ConvScripts.GetSize() && !(ConvScripts.GetAt(iScript)).IsEmpty(); ++iScript)
	{
		strcpy(file, ConvScripts.GetAt(iScript));
		m_status.SetWindowText("Processing Conversation Script: " + ConvScripts.GetAt(iScript));
		incomment = FALSE;

		OpenRFFile(&MainFS, kScriptFile, file, GE_VFILE_OPEN_READONLY, false);
		while(geVFile_GetS(MainFS, szInputLine, 256) == GE_TRUE)
		{
			if(strlen(szInputLine) <= 1)
				continue;

			str = szInputLine;
			str.TrimLeft();
			str.TrimRight();
			str.MakeLower();

			if(incomment)
			{
				i = str.Find("*/");
				if(i != -1)
				{
					incomment = FALSE;
					str = str.Right(str.GetLength()-i-2);
					str.TrimLeft();
				}
				else
				{
					continue;
				}
			}
			for(i=str.Find("/*"); i!=-1; i=str.Find("/*", i))
			{
				str2 = str.Left(i);
				j = str.Find("*/", i+2);
				if(j != -1)
				{
					incomment = FALSE;
					str = str.Right(str.GetLength()-j-2);
					str.TrimLeft();
					str = str2 + str;
				}
				else
				{
					incomment = TRUE;
					str = str2;
					str.TrimRight();
					break;
				}
			}
			// trim (trailing) comments
			i = str.Find("//");
			if(i != -1)
			{
				str = str.Left(i);
				str.TrimRight();
			}
			if(str.GetLength() < 5)
				continue;


			for(i=str.Find(".wav"); i>=0 && i<str.GetLength(); i=str.Find(".wav", i+3))
			{
				j=i-1;
				while(j>=0 && (str.GetAt(j) != '"') && (str.GetAt(j) != '['))
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1, i-j+3));
					if(CopyToDos(kAudioFile, bsp) == IDCANCEL) return;
				}
			}
			for(i=str.Find(".mp3"); i>=0 && i<str.GetLength(); i=str.Find(".mp3", i+3))
			{
				j=i-1;
				while(j>=0 && (str.GetAt(j) != '"') && (str.GetAt(j) != '['))
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1, i-j+3));
					if(CopyToDos(kAudioFile, bsp) == IDCANCEL) return;
				}
			}
			for(i=str.Find(".ogg"); i>=0 && i<str.GetLength(); i=str.Find(".ogg", i+3))
			{
				j=i-1;
				while(j>=0 && (str.GetAt(j) != '"') && (str.GetAt(j) != '['))
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1,i-j+3));
					if(CopyToDos(kAudioFile, bsp) == IDCANCEL) return;
				}
			}
			for(i=str.Find(".mid"); i>=0 && i<str.GetLength(); i=str.Find(".mid", i+3))
			{
				j=i-1;
				while(j>=0 && (str.GetAt(j) != '"') && (str.GetAt(j) != '['))
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1,i-j+3));
					if(CopyToDos(kMIDIFile, bsp) == IDCANCEL) return;
				}
			}
			for(i=str.Find(".bmp"); i>=0 && i<str.GetLength(); i=str.Find(".bmp", i+3))
			{
				j=i-1;
				while(j>=0 && (str.GetAt(j) != '"') && (str.GetAt(j) != '['))
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(str1, "conversation\\");
					strcat(str1, str.Mid(j+1,i-j+3));
					strcpy(bsp, str1);
					if(CopyToDos(kBitmapFile, bsp) == IDCANCEL) return;
				}
			}
			for(i=str.Find(".jpg"); i>=0 && i<str.GetLength(); i=str.Find(".jpg", i+3))
			{
				j=i-1;
				while(j>=0 && (str.GetAt(j) != '"') && (str.GetAt(j) != '['))
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(str1, "conversation\\");
					strcat(str1, str.Mid(j+1,i-j+3));
					strcpy(bsp, str1);
					if(CopyToDos(kBitmapFile, bsp) == IDCANCEL) return;
				}
			}
			for(i=str.Find(".tga"); i>=0 && i<str.GetLength(); i=str.Find(".tga", i+3))
			{
				j=i-1;
				while(j>=0 && (str.GetAt(j) != '"') && (str.GetAt(j) != '['))
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(str1, "conversation\\");
					strcat(str1, str.Mid(j+1,i-j+3));
					strcpy(bsp, str1);
					if(CopyToDos(kBitmapFile, bsp) == IDCANCEL) return;
				}
			}
			for(i=str.Find(".png"); i>=0 && i<str.GetLength(); i=str.Find(".png", i+3))
			{
				j=i-1;
				while(j>=0 && (str.GetAt(j) != '"') && (str.GetAt(j) != '['))
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(str1, "conversation\\");
					strcat(str1, str.Mid(j+1,i-j+3));
					strcpy(bsp, str1);
					if(CopyToDos(kBitmapFile, bsp) == IDCANCEL) return;
				}
			}
		}
		geVFile_Close(MainFS);
	}
}

void CGameBuilderDlg::CopyScript()
{
	CString str;
	CString str2;
	char file[256];
	char str1[256];
	char bsp[256];
	char szInputLine[256];
	geVFile *MainFS;
	int i, j;
	BOOL incomment;
	int iScript;

	for(iScript=0; iScript<Scripts.GetSize() && !(Scripts.GetAt(iScript)).IsEmpty(); ++iScript)
	{
		strcpy(file, Scripts.GetAt(iScript));
		m_status.SetWindowText("Processing Scripts: " + Scripts.GetAt(iScript));
		incomment = FALSE;

		OpenRFFile(&MainFS, kScriptFile, file, GE_VFILE_OPEN_READONLY, false);
		while(geVFile_GetS(MainFS, szInputLine, 256) == GE_TRUE)
		{
			if(strlen(szInputLine) <= 1)
				continue;

			str = szInputLine;
			str.TrimLeft();
			str.TrimRight();
			str.MakeLower();

			if(incomment)
			{
				i = str.Find("*/");
				if(i != -1)
				{
					incomment = FALSE;
					str = str.Right(str.GetLength()-i-2);
					str.TrimLeft();
				}
				else
				{
					continue;
				}
			}
			for(i=str.Find("/*"); i!=-1; i=str.Find("/*", i))
			{
				str2 = str.Left(i);
				j = str.Find("*/", i+2);
				if(j != -1)
				{
					incomment = FALSE;
					str = str.Right(str.GetLength()-j-2);
					str.TrimLeft();
					str = str2 + str;
				}
				else
				{
					incomment = TRUE;
					str = str2;
					str.TrimRight();
					break;
				}
			}
			// trim (trailing) comments
			i = str.Find("//");
			if(i != -1)
			{
				str = str.Left(i);
				str.TrimRight();
			}
			if(str.GetLength() < 5)
				continue;

			for(i=str.Find(".wav"); i>=0 && i<str.GetLength(); i=str.Find(".wav", i+3))
			{
				j=i-1;
				while(j>=0 && (str.GetAt(j) != '"') && (str.GetAt(j) != '['))
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1, i-j+3));
					if(CopyToDos(kAudioFile, bsp) == IDCANCEL) return;
				}
			}
			for(i=str.Find(".mp3"); i>=0 && i<str.GetLength(); i=str.Find(".mp3", i+3))
			{
				j=i-1;
				while(j>=0 && (str.GetAt(j) != '"') && (str.GetAt(j) != '['))
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1, i-j+3));
					if(CopyToDos(kAudioFile, bsp) == IDCANCEL) return;
				}
			}
			for(i=str.Find(".ogg"); i>=0 && i<str.GetLength(); i=str.Find(".ogg", i+3))
			{
				j=i-1;
				while(j>=0 && (str.GetAt(j) != '"') && (str.GetAt(j) != '['))
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1,i-j+3));
					if(CopyToDos(kAudioFile, bsp) == IDCANCEL) return;
				}
			}
			for(i=str.Find(".mid"); i>=0 && i<str.GetLength(); i=str.Find(".mid", i+3))
			{
				j=i-1;
				while(j>=0 && (str.GetAt(j) != '"') && (str.GetAt(j) != '['))
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1,i-j+3));
					if(CopyToDos(kMIDIFile, bsp) == IDCANCEL) return;
				}
			}
			for(i=str.Find(".bmp"); i>=0 && i<str.GetLength(); i=str.Find(".bmp", i+3))
			{
				j=i-1;
				while(j>=0 && (str.GetAt(j) != '"') && (str.GetAt(j) != '['))
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(str1, "terrain\\");
					strcat(str1, str.Mid(j+1,i-j+3));
					strcpy(bsp, str1);
					if(CopyToDos(kBitmapFile, bsp) == IDCANCEL) return;
				}
			}
			for(i=str.Find(".jpg"); i>=0 && i<str.GetLength(); i=str.Find(".jpg", i+3))
			{
				j=i-1;
				while(j>=0 && (str.GetAt(j) != '"') && (str.GetAt(j) != '['))
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(str1, "terrain\\");
					strcat(str1, str.Mid(j+1,i-j+3));
					strcpy(bsp, str1);
					if(CopyToDos(kBitmapFile, bsp) == IDCANCEL) return;
				}
			}
			for(i=str.Find(".tga"); i>=0 && i<str.GetLength(); i=str.Find(".tga", i+3))
			{
				j=i-1;
				while(j>=0 && (str.GetAt(j) != '"') && (str.GetAt(j) != '['))
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(str1, "terrain\\");
					strcat(str1, str.Mid(j+1,i-j+3));
					strcpy(bsp, str1);
					if(CopyToDos(kBitmapFile, bsp) == IDCANCEL) return;
				}
			}
			for(i=str.Find(".png"); i>=0 && i<str.GetLength(); i=str.Find(".png", i+3))
			{
				j=i-1;
				while(j>=0 && (str.GetAt(j) != '"') && (str.GetAt(j) != '['))
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(str1, "terrain\\");
					strcat(str1, str.Mid(j+1,i-j+3));
					strcpy(bsp, str1);
					if(CopyToDos(kBitmapFile, bsp) == IDCANCEL) return;
				}
			}
		}

		geVFile_Close(MainFS);
	}
}

void CGameBuilderDlg::CopyEffect()
{
	CString readinfo;
	CString valuename, value;
	CString str;
	geVFile *MainFS, *SecondFS;;
	char szInputLine[256];
	char file[256];

	OpenRFFile(&MainFS, kInstallFile, "effect.ini", GE_VFILE_OPEN_READONLY, false);
	while(geVFile_GetS(MainFS, szInputLine, 256) == GE_TRUE)
	{
		if(strlen(szInputLine) <= 1)
			continue;
		readinfo = szInputLine;
		readinfo.TrimRight();

		if(readinfo != "")
		{
			if(readinfo[0] == '[' && readinfo[readinfo.GetLength()-1] == ']') //if a section heading
			{
				readinfo.TrimLeft('[');
				readinfo.TrimRight(']');
				m_status.SetWindowText("Processing Effects: " + readinfo);
				continue;
			}

			if(readinfo[0] != ';')
			{
				valuename = readinfo.Left(readinfo.Find("="));
				value = readinfo.Right(readinfo.GetLength()-valuename.GetLength()-1);
				valuename.TrimLeft();
				valuename.TrimRight();
				value.TrimLeft();
				value.TrimRight();
				strcpy(file, value);

				if(valuename == "bitmapname" || valuename == "alphamapname")
				{
					if(CopyToDos(kBitmapFile, file) == IDCANCEL) return;
				}
				else if(valuename == "name")
				{
					if(CopyToDos(kAudioFile, file) == IDCANCEL) return;
				}
				else if(valuename == "basebitmapname" || valuename == "basealphamapname")
				{
					char base[256];
					int i = 0;
					while(1)
					{
						sprintf(base, "%s%d%s", file, i, ".bmp");
						if(!OpenRFFile(&SecondFS, kBitmapFile, base, GE_VFILE_OPEN_READONLY, false))
							break;
						geVFile_Close(SecondFS);
						if(CopyToDos(kBitmapFile, base) == IDCANCEL) return;
						++i;
					}
				}
				else if(valuename == "basename")
				{
					char base[256];
					int i = 0;
					while(1)
					{
						sprintf(base, "%s%d%s", file, i, ".act");
						if(!OpenRFFile(&SecondFS, kActorFile, base, GE_VFILE_OPEN_READONLY, false))
							break;
						geVFile_Close(SecondFS);
						if(CopyToDos(kActorFile, base) == IDCANCEL) return;
						++i;
					}
				}
			}

		}
	}
	geVFile_Close(MainFS);
}

void CGameBuilderDlg::CopyPawn()
{
	CString readinfo;
	CString valuename, value;
	CString str;
	geVFile *MainFS;
	char szInputLine[256];
	char file[256];

	OpenRFFile(&MainFS, kInstallFile, "pawn.ini", GE_VFILE_OPEN_READONLY, false);
	while(geVFile_GetS(MainFS, szInputLine, 256) == GE_TRUE)
	{
		if(strlen(szInputLine) <= 1)
			continue;
		readinfo = szInputLine;
		readinfo.TrimRight();

		if(readinfo != "")
		{
			if(readinfo[0] == '[' && readinfo[readinfo.GetLength()-1] == ']') // if a section heading
			{
				readinfo.TrimLeft('[');
				readinfo.TrimRight(']');
				m_status.SetWindowText("Processing Pawn.ini: " + readinfo);
				continue;
			}

			if(readinfo[0] != ';')
			{
				valuename = readinfo.Left(readinfo.Find("="));
				value = readinfo.Right(readinfo.GetLength()-valuename.GetLength()-1);
				valuename.TrimLeft();
				valuename.TrimRight();
				value.TrimLeft();
				value.TrimRight();
				strcpy(file, value);
				if(valuename == "actorname")
				{
					if(CopyToDos(kActorFile, file) == IDCANCEL) return;
				}
				else if(valuename == "shadowbitmap" || valuename == "shadowalphamap")
				{
					if(!value.IsEmpty())
						if(CopyToDos(kBitmapFile, file) == IDCANCEL) return;
				}
				else if(valuename == "background" || valuename == "backgroundalpha"
					||	valuename == "replybackground" || valuename == "replybackgroundalpha"
					||	valuename == "replymenubar" || valuename == "replymenubaralpha"
					||	valuename == "icon")
				{
					strcpy(file, "conversation\\");
					strcat(file, value);
					if(CopyToDos(kBitmapFile, file) == IDCANCEL) return;
				}
				else if(valuename == "giffile0" || valuename == "giffile1"
					||	valuename == "giffile2" || valuename == "giffile3"
					||	valuename == "giffile4" || valuename == "giffile5"
					||	valuename == "giffile6" || valuename == "giffile7"
					||	valuename == "giffile8")
				{
					if(!value.IsEmpty())
						if(CopyToDos(kVideoFile, file) == IDCANCEL) return;
				}
			}

		}
	}
	geVFile_Close(MainFS);
}

void CGameBuilderDlg::CopyEnvironment()
{
	CString readinfo;
	CString valuename, value;
	geVFile *MainFS;
	char szInputLine[256];
	char filename[256];
	char file[256];

	int i;

	for(i=0; i<Environments.GetSize() && !(Environments.GetAt(i)).IsEmpty(); ++i)
	{
		strcpy(filename, Environments.GetAt(i));
		m_status.SetWindowText("Processing Environment.ini: " + Environments.GetAt(i));

		OpenRFFile(&MainFS, kInstallFile, filename, GE_VFILE_OPEN_READONLY, false);
		while(geVFile_GetS(MainFS, szInputLine, 256) == GE_TRUE)
		{
			if(strlen(szInputLine) <= 1)
				continue;
			readinfo = szInputLine;
			readinfo.TrimRight();

			if(readinfo != "")
			{
				if(readinfo[0] == '[' && readinfo[readinfo.GetLength()-1] == ']') //if a section heading
					continue;

				if(readinfo[0] != ';')
				{
					value = readinfo;
					value.TrimLeft();
					value.TrimRight();
					strcpy(file, value);
					if(CopyToDos(kAudioFile, file) == IDCANCEL) return;
				}
			}
		}
		geVFile_Close(MainFS);
	}
}

void CGameBuilderDlg::CopySplash()
{
	if(!m_splashini)
		return;

	CString readinfo;
	CString valuename, value;
	geVFile *MainFS;
	char szInputLine[256];
	char file[256];

	OpenRFFile(&MainFS, kInstallFile, "splash.ini", GE_VFILE_OPEN_READONLY, false);
	while(geVFile_GetS(MainFS, szInputLine, 256) == GE_TRUE)
	{
		if(strlen(szInputLine) <= 1)
			continue;
		readinfo = szInputLine;
		readinfo.TrimRight();

		if(readinfo != "")
		{
			if(readinfo[0] == '[' && readinfo[readinfo.GetLength()-1] == ']') //if a section heading
				continue;

			if(readinfo[0] != ';')
			{
				value = readinfo;
				value.TrimLeft();
				value.TrimRight();
				strcpy(file, value);
				if(CopyToDos(kBitmapFile, file) == IDCANCEL) return;
			}
		}
	}
	geVFile_Close(MainFS);
}

void CGameBuilderDlg::CopyMessage()
{
	CString readinfo;
	CString valuename, value;
	CString str;
	geVFile *MainFS;
	char szInputLine[256];
	char file[256];

	OpenRFFile(&MainFS, kInstallFile, "message.ini", GE_VFILE_OPEN_READONLY, false);
	while(geVFile_GetS(MainFS, szInputLine, 256) == GE_TRUE)
	{
		str = "message.ini: " + CString(szInputLine);
        m_status.SetWindowText(str);

		if(strlen(szInputLine) <= 1)
			continue;
		readinfo = szInputLine;
		readinfo.TrimRight();

		if(readinfo != "")
		{
			if (readinfo[0] == '[' && readinfo[readinfo.GetLength()-1] == ']') //if a section heading
				continue;

			if(readinfo[0] != ';')
			{
				valuename = readinfo.Left(readinfo.Find("="));
				value = readinfo.Right(readinfo.GetLength()-valuename.GetLength()-1);
				valuename.TrimLeft();
				valuename.TrimRight();
				value.TrimLeft();
				value.TrimRight();
				if(valuename == "graphic" || valuename == "graphicalpha")
				{
					if(value.Find(".") != -1) // single image
					{
						strcpy(file, value);
						if(CopyToDos(kBitmapFile, file) == IDCANCEL) return;
					}
					else // animated
					{
						geVFile *SecondFS;
						int i = 0;
						while(1)
						{
							sprintf(file, "%s%d%s", value, i, ".bmp");
							if(!OpenRFFile(&SecondFS, kBitmapFile, file, GE_VFILE_OPEN_READONLY, false))
								break;
							geVFile_Close(SecondFS);
							if(CopyToDos(kBitmapFile, file) == IDCANCEL) return;
							++i;
						}
					}
				}
			}
		}
	}
	geVFile_Close(MainFS);
}

void CGameBuilderDlg::CopyInventory()
{
	CString readinfo;
	CString valuename, value;
	CString str;
	geVFile *MainFS;
	char szInputLine[256];
	char file[256];

	OpenRFFile(&MainFS, kInstallFile, "inventory.ini", GE_VFILE_OPEN_READONLY, false);
	while(geVFile_GetS(MainFS, szInputLine, 256) == GE_TRUE)
	{
		if(strlen(szInputLine) <= 1)
			continue;
		readinfo = szInputLine;
		readinfo.TrimRight();

		if(readinfo != "")
		{
			if(readinfo[0] == '[' && readinfo[readinfo.GetLength()-1] == ']') //if a section heading
				continue;

			if(readinfo[0] != ';')
			{
				valuename = readinfo.Left(readinfo.Find("="));
				value = readinfo.Right(readinfo.GetLength()-valuename.GetLength()-1);
				valuename.TrimLeft();
				valuename.TrimRight();
				value.TrimLeft();
				value.TrimRight();
				strcpy(file, value);
				if(valuename == "background" || valuename == "backgroundalpha"
					|| valuename == "highlight" || valuename == "highlightalpha"
					|| valuename == "arrowr" || valuename == "arrowralpha"
					|| valuename == "arrowl" || valuename == "arrowlalpha"
					|| valuename == "arrowrhighlight" || valuename == "arrowlhighlight"
					|| valuename == "image" || valuename == "imagealpha"
					|| valuename == "graphic" || valuename == "graphicalpha")
				{
					strcpy(file, "inventory\\");
					strcat(file, value);
					if(CopyToDos(kBitmapFile, file) == IDCANCEL) return;
				}
				else if(valuename == "keysound")
				{
					if(CopyToDos(kAudioFile, file) == IDCANCEL) return;
				}
			}
		}
	}
	geVFile_Close(MainFS);
}

void CGameBuilderDlg::CopyPlayerSetup()
{
	CString readinfo;
	CString valuename, value, keyname;
	geVFile *MainFS;
	char szInputLine[256];
	char filename[256];
	char file[256];

	int ii;

	for(ii=0; ii<PlayerSetups.GetSize() && !(PlayerSetups.GetAt(ii)).IsEmpty(); ++ii)
	{
		strcpy(filename, PlayerSetups.GetAt(ii));
		m_status.SetWindowText("Processing Player Data: " + PlayerSetups.GetAt(ii));

		OpenRFFile(&MainFS, kInstallFile, filename, GE_VFILE_OPEN_READONLY, false);
		while(geVFile_GetS(MainFS, szInputLine, 256) == GE_TRUE)
		{
			if(strlen(szInputLine) <= 1)
				continue;
			readinfo = szInputLine;
			readinfo.TrimRight();

			if (readinfo != "")
			{
				if(readinfo[0] == '[' && readinfo[readinfo.GetLength()-1] == ']') //if a section heading
				{
					keyname = readinfo;
					keyname.TrimLeft('[');
					keyname.TrimRight(']');
				}
				else
				{
					if(readinfo[0] != ';')
					{
						if(keyname=="Sounds")
						{
							valuename = readinfo.Left(readinfo.Find("="));
							value = readinfo.Right(readinfo.GetLength()-valuename.GetLength()-1);
							valuename.TrimLeft();
							valuename.TrimRight();
							value.TrimLeft();
							value.TrimRight();
							strcpy(file, value);

							if(valuename=="die" || valuename=="injury" || valuename=="land")
							{
								char strip[256], *temp;
								int i = 0;
								strcpy(strip,value);
								temp = strtok(strip," \n");
								while(temp)
								{
									if(CopyToDos(kAudioFile, temp) == IDCANCEL) return;
									++i;
									if(i == 5)
										break;
									temp = strtok(NULL," \n");
								}
							}
						}
					}
				}
			}
		}
		geVFile_Close(MainFS);
	}
}

void CGameBuilderDlg::CopyWeapon()
{
	CString readinfo;
	CString valuename, value;
	geVFile *MainFS;
	char szInputLine[256];
	char filename[256];
	char file[256];

	int ii;

	for(ii=0; ii<Weapons.GetSize() && !(Weapons.GetAt(ii)).IsEmpty(); ++ii)
	{
		strcpy(filename, Weapons.GetAt(ii));
		m_status.SetWindowText("Processing Weapons: " + Weapons.GetAt(ii));

		OpenRFFile(&MainFS, kInstallFile, filename, GE_VFILE_OPEN_READONLY, false);
		while(geVFile_GetS(MainFS, szInputLine, 256) == GE_TRUE)
		{
			if(strlen(szInputLine) <= 1)
				continue;
			readinfo = szInputLine;
			readinfo.TrimRight();

			if (readinfo != "")
			{
				if (readinfo[0] == '[' && readinfo[readinfo.GetLength()-1] == ']') //if a section heading
					continue;

				if(readinfo[0] != ';')
				{
					valuename = readinfo.Left(readinfo.Find("="));
					value = readinfo.Right(readinfo.GetLength()-valuename.GetLength()-1);
					valuename.TrimLeft();
					valuename.TrimRight();
					value.TrimLeft();
					value.TrimRight();
					strcpy(file, value);
					if(valuename=="actor" || valuename=="viewactor" || valuename=="playeractor" || valuename=="dropactor")
					{
						if(CopyToDos(kActorFile, file) == IDCANCEL) return;
					}
					else if(valuename=="movesound" || valuename=="impactsound" || valuename=="bouncesound"
						|| valuename=="reloadsound" || valuename=="attacksound"
						|| valuename=="emptysound" || valuename=="hitsound")
					{
						if(CopyToDos(kAudioFile, file) == IDCANCEL) return;
					}
					else if(valuename=="crosshair" || valuename=="crosshairalpha"
						|| valuename=="zoomoverlay" || valuename=="zoomoverlayalpha")
					{
						if(CopyToDos(kBitmapFile, file) == IDCANCEL) return;
					}
				}
			}
		}
		geVFile_Close(MainFS);
	}
}

void CGameBuilderDlg::CopyLevels()
{
	CString Level;
	CString str;
	char file[256];
	char bsp[256];
	char szInputLine[256];
	geVFile *MainFS, *SecondFS;
	int i, j, ii;

	for(ii=0; ii<Levels.GetSize() && !(Levels.GetAt(ii)).IsEmpty(); ++ii)
	{
		Level = Levels.GetAt(ii);
		m_status.SetWindowText("Processing Game Levels: " + Level);
		Level.Replace(".bsp", ".3dt");
		strcpy(file, Level);

		OpenRFFile(&MainFS, kLevelFile, file, GE_VFILE_OPEN_READONLY, false);
		while(geVFile_GetS(MainFS, szInputLine, 256) == GE_TRUE)
		{
			if(strlen(szInputLine) <= 1)
				continue;
			str = szInputLine;
			str.TrimRight();
			str.MakeLower();

			i = str.Find(".bsp");
			if(i>=0 && i<str.GetLength())
			{
				j=i-1;
				while(j>=0 && str.GetAt(j) != '"')
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1, i-j+3));
					if(!AContains(Levels, bsp))
					{
						ASetAtFirstFreeIndex(Levels, bsp);
						if(CopyToDos(kLevelFile, bsp) == IDCANCEL) return;
					}
				}
				continue;
			}
			i = str.Find(".wav");
			if(i>=0 && i<str.GetLength())
			{
				j=i-1;
				while(j>=0 && str.GetAt(j) != '"')
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1, i-j+3));
					int type = kAudioFile;
					i = str.Find("szstreamingaudio");
					if(i>=0 && i<str.GetLength())
						type = kAudioStreamFile;
					i = str.Find("szstreamfile");
					if(i>=0 && i<str.GetLength())
						type = kAudioStreamFile;
					if(CopyToDos(type, bsp) == IDCANCEL) return;
				}
				continue;
			}
			i = str.Find(".mp3");
			if(i>=0 && i<str.GetLength())
			{
				j=i-1;
				while(j>=0 && str.GetAt(j) != '"')
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1, i-j+3));
					if(CopyToDos(kAudioFile, bsp) == IDCANCEL) return;
				}
				continue;
			}
			i = str.Find(".ogg");
			if(i>=0 && i<str.GetLength())
			{
				j=i-1;
				while(j>=0 && str.GetAt(j) != '"')
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1, i-j+3));
					if(CopyToDos(kAudioFile, bsp) == IDCANCEL) return;
				}
				continue;
			}
			i = str.Find(".bmp");
			if(i>=0 && i<str.GetLength())
			{
				j=i-1;
				while(j>=0 && str.GetAt(j) != '"')
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1, i-j+3));
					if(CopyToDos(kBitmapFile, bsp) == IDCANCEL) return;
				}
				continue;
			}
			i = str.Find(".tga");
			if(i>=0 && i<str.GetLength())
			{
				j=i-1;
				while(j>=0 && str.GetAt(j) != '"')
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1, i-j+3));
					if(CopyToDos(kBitmapFile, bsp) == IDCANCEL) return;
				}
				continue;
			}
			i = str.Find(".jpg");
			if(i>=0 && i<str.GetLength())
			{
				j=i-1;
				while(j>=0 && str.GetAt(j) != '"')
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1, i-j+3));
					if(CopyToDos(kBitmapFile, bsp) == IDCANCEL) return;
				}
				continue;
			}
			i = str.Find(".png");
			if(i>=0 && i<str.GetLength())
			{
				j=i-1;
				while(j>=0 && str.GetAt(j) != '"')
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1, i-j+3));
					if(CopyToDos(kBitmapFile, bsp) == IDCANCEL) return;
				}
				continue;
			}
			i = str.Find(".mid");
			if(i>=0 && i<str.GetLength())
			{
				j=i-1;
				while(j>=0 && str.GetAt(j) != '"')
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1, i-j+3));
					if(CopyToDos(kMIDIFile, bsp) == IDCANCEL) return;
				}
				continue;
			}
			i = str.Find(".avi");
			if(i>=0 && i<str.GetLength())
			{
				j=i-1;
				while(j>=0 && str.GetAt(j) != '"')
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1, i-j+3));
					if(CopyToDos(kVideoFile, bsp) == IDCANCEL) return;
				}
				continue;
			}
			i = str.Find(".gif");
			if(i>=0 && i<str.GetLength())
			{
				j=i-1;
				while(j>=0 && str.GetAt(j) != '"')
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1, i-j+3));
					if(CopyToDos(kVideoFile, bsp) == IDCANCEL) return;
				}
				continue;
			}
			i = str.Find(".act");
			if(i>=0 && i<str.GetLength())
			{
				j=i-1;
				while(j>=0 && str.GetAt(j) != '"')
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1, i-j+3));
					if(CopyToDos(kActorFile, bsp) == IDCANCEL) return;
				}
				continue;
			}
			i = str.Find("skydome");
			if(i>=0 && i<str.GetLength())
			{
	            if(CopyToDos(kBitmapFile, "terrain\\clouds_256.bmp")	== IDCANCEL) return;
				if(CopyToDos(kBitmapFile, "terrain\\a_clouds4.bmp")		== IDCANCEL) return;

				if(CopyToDos(kBitmapFile, "terrain\\moonfull.bmp")		== IDCANCEL) return;
				if(CopyToDos(kBitmapFile, "terrain\\a_moonfull.bmp")	== IDCANCEL) return;
				if(CopyToDos(kBitmapFile, "terrain\\a_moonq1.bmp")		== IDCANCEL) return;
				if(CopyToDos(kBitmapFile, "terrain\\a_moonnew.bmp")		== IDCANCEL) return;
				if(CopyToDos(kBitmapFile, "terrain\\a_moonq3.bmp")		== IDCANCEL) return;

				if(CopyToDos(kBitmapFile, "terrain\\star1.bmp")			== IDCANCEL) return;
				if(CopyToDos(kBitmapFile, "terrain\\star2.bmp")			== IDCANCEL) return;
				if(CopyToDos(kBitmapFile, "terrain\\white64.bmp")		== IDCANCEL) return;
				if(CopyToDos(kBitmapFile, "terrain\\white128.bmp")		== IDCANCEL) return;
				if(CopyToDos(kBitmapFile, "terrain\\white256.bmp")		== IDCANCEL) return;
				if(CopyToDos(kBitmapFile, "terrain\\a_star2.bmp")		== IDCANCEL) return;
				if(CopyToDos(kBitmapFile, "terrain\\a_starfield64.bmp") == IDCANCEL) return;
				if(CopyToDos(kBitmapFile, "terrain\\a_starfield128.bmp")== IDCANCEL) return;

				if(CopyToDos(kBitmapFile, "terrain\\flare5.bmp")		== IDCANCEL) return;
				if(CopyToDos(kBitmapFile, "terrain\\flare6.bmp")		== IDCANCEL) return;
				if(CopyToDos(kBitmapFile, "terrain\\a_flare2.bmp")		== IDCANCEL) return;
				if(CopyToDos(kBitmapFile, "terrain\\a_flare3.bmp")		== IDCANCEL) return;
				if(CopyToDos(kBitmapFile, "terrain\\a_flare4.bmp")		== IDCANCEL) return;
				if(CopyToDos(kBitmapFile, "terrain\\a_flare5.bmp")		== IDCANCEL) return;
				if(CopyToDos(kBitmapFile, "terrain\\a_flare6.bmp")		== IDCANCEL) return;

				if(CopyToDos(kBitmapFile, "terrain\\a_sunflare.bmp")	== IDCANCEL) return;
				if(CopyToDos(kBitmapFile, "terrain\\flat.bmp")			== IDCANCEL) return;
				if(CopyToDos(kBitmapFile, "terrain\\desert256.bmp")		== IDCANCEL) return;
				continue;
			}
			i = str.Find("key attributeinfofile value");
			if(i>=0 && i<str.GetLength())
			{
				i = str.Find(".");
				j=i-1;
				while(j>=0 && str.GetAt(j) != '"')
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1, i-j+3));
					if(CopyToDos(kInstallFile, bsp) == IDCANCEL) return;
				}
				continue;
			}
			i = str.Find("key hudinfofile value");
			if(i>=0 && i<str.GetLength())
			{
				i = str.Find(".");
				j=i-1;
				while(j>=0 && str.GetAt(j)!='"')
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1, i-j+3));
					if(!AContains(HUDs, bsp))
					{
						ASetAtFirstFreeIndex(HUDs, bsp);
						if(CopyToDos(kInstallFile, bsp) == IDCANCEL) return;
					}
				}
				continue;
			}
			i = str.Find("key alphanamebase value");
			if(i>=0 && i<str.GetLength())
			{
				j=str.GetLength()-3;
				while(j>=0 && str.GetAt(j)!='"')
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1, str.GetLength()-j-2));

					char base[256];
					int ij = 0;
					while(1)
					{
						sprintf(base, "%s%d%s", bsp, ij, ".bmp");
						if(!OpenRFFile(&SecondFS, kBitmapFile, base, GE_VFILE_OPEN_READONLY, false))
							break;
						geVFile_Close(SecondFS);
						if(CopyToDos(kBitmapFile, base) == IDCANCEL) return;
						++ij;
					}
				}
				continue;
			}
			i = str.Find("key bmpnamebase value");
			if(i>=0 && i<str.GetLength())
			{
				j=str.GetLength()-3;
				while(j>=0 && str.GetAt(j)!='"')
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1, str.GetLength()-j-2));

					char base[256];
					int ij = 0;
					while(1)
					{
						sprintf(base, "%s%d%s", bsp, ij, ".bmp");
						if(!OpenRFFile(&SecondFS, kBitmapFile, base, GE_VFILE_OPEN_READONLY, false))
							break;
						geVFile_Close(SecondFS);
						if(CopyToDos(kBitmapFile, base) == IDCANCEL) return;
						++ij;
					}
				}
				continue;
			}
			// alphamap - overlay
			i = str.Find("key alphamap value");
			if(i>=0 && i<str.GetLength())
			{
				if(str.Find(".") == -1) // no file extension
				{
					j=str.GetLength()-3;
					while(j>=0 && str.GetAt(j)!='"')
					{
						--j;
					}
					if(j>=0)
					{
						strcpy(bsp, str.Mid(j+1, str.GetLength()-j-2));

						char base[256];
						int ij = 0;
						while(1)
						{
							sprintf(base, "%s%d%s", bsp, ij, ".bmp");
							if(!OpenRFFile(&SecondFS, kBitmapFile, base, GE_VFILE_OPEN_READONLY, false))
								break;
							geVFile_Close(SecondFS);
							if(CopyToDos(kBitmapFile, base) == IDCANCEL) return;
							++ij;
						}
					}
				}
				continue;
			}
			// bitmap - overlay
			i = str.Find("key bitmap value");
			if(i>=0 && i<str.GetLength())
			{
				if(str.Find(".") == -1) // no file extension
				{
					j=str.GetLength()-3;
					while(j>=0 && str.GetAt(j)!='"')
					{
						--j;
					}
					if(j>=0)
					{
						strcpy(bsp, str.Mid(j+1, str.GetLength()-j-2));

						char base[256];
						int ij = 0;
						while(1)
						{
							sprintf(base, "%s%d%s", bsp, ij, ".bmp");
							if(!OpenRFFile(&SecondFS, kBitmapFile, base, GE_VFILE_OPEN_READONLY, false))
								break;
							geVFile_Close(SecondFS);
							if(CopyToDos(kBitmapFile, base) == IDCANCEL) return;
							++ij;
						}
					}
				}
				continue;
			}
			// alphaname - walldecal
			i = str.Find("key alphaname value");
			if(i>=0 && i<str.GetLength())
			{
				if(str.Find(".") == -1) // no file extension
				{
					j=str.GetLength()-3;
					while(j>=0 && str.GetAt(j)!='"')
					{
						--j;
					}
					if(j>=0)
					{
						strcpy(bsp, str.Mid(j+1, str.GetLength()-j-2));

						char base[256];
						int ij = 0;
						while(1)
						{
							sprintf(base, "%s%d%s", bsp, ij, ".bmp");
							if(!OpenRFFile(&SecondFS, kBitmapFile, base, GE_VFILE_OPEN_READONLY, false))
								break;
							geVFile_Close(SecondFS);
							if(CopyToDos(kBitmapFile, base) == IDCANCEL) return;
							++ij;
						}
					}
				}
				continue;
			}
			// bmpname - walldecal
			i = str.Find("key bmpname value");
			if(i>=0 && i<str.GetLength())
			{
				if(str.Find(".") == -1) // no file extension
				{
					j=str.GetLength()-3;
					while(j>=0 && str.GetAt(j)!='"')
					{
						--j;
					}
					if(j>=0)
					{
						strcpy(bsp, str.Mid(j+1, str.GetLength()-j-2));

						char base[256];
						int ij = 0;
						while(1)
						{
							sprintf(base, "%s%d%s", bsp, ij, ".bmp");
							if(!OpenRFFile(&SecondFS, kBitmapFile, base, GE_VFILE_OPEN_READONLY, false))
								break;
							geVFile_Close(SecondFS);
							if(CopyToDos(kBitmapFile, base) == IDCANCEL) return;
							++ij;
						}
					}
				}
				continue;
			}
			i = str.Find("key basename value");
			if(i>=0 && i<str.GetLength())
			{
				j=str.GetLength()-3;
				while(j>=0 && str.GetAt(j) != '"')
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1, str.GetLength()-j-2));
					char base[256];
					int ij = 0;
					while(1)
					{
						sprintf(base, "%s%d%s", bsp, ij, ".act");
						if(!OpenRFFile(&SecondFS, kActorFile, base, GE_VFILE_OPEN_READONLY, false))
							break;
						geVFile_Close(SecondFS);
						if(CopyToDos(kActorFile, base) == IDCANCEL) return;
						++ij;
					}
				}
				continue;
			}
			if(!m_splashini)
			{
				// is there a changelevel that uses splash.ini?
				i = str.Find("key szsplashfile value");
				if(i>=0 && i<str.GetLength())
				{
					if(str.Find(".") == -1) // no file extension
					{
						j=str.GetLength()-3;
						while(j>=0 && str.GetAt(j) != '"')
						{
							--j;
						}
						if(j>=0)
						{
							if(CopyToDos(kInstallFile, "splash.ini") == IDCANCEL) return;
							m_splashini = true;
						}
					}
					continue;
				}
			}
			i = str.Find("key scriptname value"); // levelcontroller, pawn, skydome
			if(i>=0 && i<str.GetLength())
			{
				i = str.Find(".");
				j=i-1;
				while(j>=0 && str.GetAt(j) != '"')
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1, str.GetLength()-j-2));
					if(!AContains(Scripts, bsp))
					{
						ASetAtFirstFreeIndex(Scripts, bsp);
						if(CopyToDos(kScriptFile, bsp) == IDCANCEL) return;
					}
				}
				continue;
			}
			i = str.Find("key convscriptname value"); // pawn conversation script
			if(i>=0 && i<str.GetLength())
			{
				i = str.Find(".");
				j=i-1;
				while(j>=0 && str.GetAt(j) != '"')
				{
					--j;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1, str.GetLength()-j-2));
					if(!AContains(ConvScripts, bsp))
					{
						ASetAtFirstFreeIndex(ConvScripts, bsp);
						if(CopyToDos(kScriptFile, bsp) == IDCANCEL) return;
					}
				}
				continue;
			}
		}
		geVFile_Close(MainFS);
	}
}

void CGameBuilderDlg::CopyHud()
{
	CString readinfo;
	CString valuename, value;
	geVFile *MainFS;
	char szInputLine[256];
	char filename[256];
	char file[256];
	int iHUD;

	for(iHUD=0; iHUD<HUDs.GetSize() && !(HUDs.GetAt(iHUD)).IsEmpty(); ++iHUD)
	{
		strcpy(filename, HUDs.GetAt(iHUD));
		m_status.SetWindowText("Processing HUDs: " + HUDs.GetAt(iHUD));


		OpenRFFile(&MainFS, kInstallFile, filename, GE_VFILE_OPEN_READONLY, false);
		while(geVFile_GetS(MainFS, szInputLine, 256) == GE_TRUE)
		{
			if(strlen(szInputLine) <= 1)
				continue;
			readinfo = szInputLine;
			readinfo.TrimRight();

			if(readinfo != "")
			{
				if(readinfo[0] == '[' && readinfo[readinfo.GetLength()-1] == ']') //if a section heading
					continue;

				if(readinfo[0] != ';')
				{
					valuename = readinfo.Left(readinfo.Find("="));
					value = readinfo.Right(readinfo.GetLength()-valuename.GetLength()-1);
					valuename.TrimLeft();
					valuename.TrimRight();
					value.TrimLeft();
					value.TrimRight();
					strcpy(file, value);
					if(	valuename == "frame" || valuename == "framealpha" ||
						valuename == "indicator" || valuename == "indicatoralpha" ||
						valuename == "npcindicator" || valuename == "npcindicatoralpha" ||
						valuename == "bitmap" || valuename == "bitmapalpha")
					{
						if(CopyToDos(kBitmapFile, file) == IDCANCEL) return;
					}
					else if(valuename == "giffile")
					{
						if(CopyToDos(kVideoFile, file) == IDCANCEL) return;
					}
				}
			}
		}
		geVFile_Close(MainFS);
	}
}

void CGameBuilderDlg::CreateVFS()
{
	CString destPath = m_currentdir;
	destPath = destPath + "\\GameBuilder\\";
	destPath = destPath + m_packfile;
	CString sPath = "";
	CString srcPath = m_currentdir;
	srcPath += "\\$$tmp$$.vfs";
	chdir(m_currentdir);

	VFS = geVFile_OpenNewSystem(NULL, GE_VFILE_TYPE_VIRTUAL, "$$tmp$$.vfs", NULL, GE_VFILE_OPEN_CREATE | GE_VFILE_OPEN_DIRECTORY);
	if(!VFS)
	{
		MessageBox("Unable to Get VFS for " + m_packfile, "RealityFactory VFS", MB_ICONSTOP | MB_OK);
		remove(srcPath);
	}
	else
	{
		CWaitCursor c;
		GetFiles(sPath, VFS);
		GetDir(sPath, VFS);
		geVFile_Close(VFS);
		chdir(m_currentdir);
		CopyFile(srcPath, destPath);
		chdir(m_currentdir);
		remove(srcPath);
	}
}

void CGameBuilderDlg::GetDir(CString &Path, geVFile *VFS)
{
	struct _finddata_t  c_file;
	long fhandle;
	CString newPath;
	CString searchString = m_currentdir;
	searchString += "\\GameBuilder\\";

	if(Path == "")
		searchString += "*.*";
	else
	{
		searchString += Path;
		searchString += "\\*.*";
	}

	if((fhandle=_findfirst(searchString, &c_file)) != -1)
	{
		if((c_file.attrib & _A_SUBDIR) == _A_SUBDIR)
		{
			if((strcmp(c_file.name, ".") != 0) && (strcmp(c_file.name, "..") !=0 ))
			{
				newPath = Path;
				newPath+= "\\";
				newPath+= c_file.name;
				MakeDirectory(VFS, newPath);
				GetFiles(newPath, VFS);
				GetDir(newPath, VFS);
			}
		}
		while(_findnext(fhandle, &c_file) == 0)
		{
			if((c_file.attrib & _A_SUBDIR) == _A_SUBDIR)
			{
				if((strcmp(c_file.name, ".") != 0) && (strcmp(c_file.name, "..") != 0))
				{
					newPath = Path;
					newPath+= "\\";
					newPath+= c_file.name;
					MakeDirectory(VFS, newPath);
					GetFiles(newPath, VFS);
					GetDir(newPath, VFS);
				}
			}
		}
	}
	_findclose(fhandle);
}

void CGameBuilderDlg::GetFiles(CString &Path, geVFile *VFS)
{
	struct _finddata_t  c_file;
	long fhandle;
	CString newPath;
	geVFile *FSSrc;
	CString searchString = m_currentdir;
	searchString += "\\GameBuilder\\";

	if(Path == "")
		searchString += "*.*";
	else
	{
		searchString += Path;
		searchString += "\\*.*";
	}

	if((fhandle=_findfirst( searchString, &c_file ))!=-1)
	{
		if((c_file.attrib & _A_SUBDIR)==0)
		{
			if((strcmp(c_file.name, ".") != 0) && (strcmp(c_file.name, "..") != 0))
			{
				if(CheckCopy(c_file.name))
				{
					newPath = Path;
					newPath+= "\\";
					newPath+= c_file.name;
					newPath = newPath.Mid(1);
					CString srcPath = m_currentdir;
					srcPath += "\\GameBuilder\\";
					srcPath += Path;

					if(srcPath.GetAt(srcPath.GetLength()-1) == _T('\\'))
						srcPath = srcPath.Left(srcPath.GetLength()-1);

					FSSrc = geVFile_OpenNewSystem(NULL, GE_VFILE_TYPE_DOS, srcPath, NULL, GE_VFILE_OPEN_READONLY | GE_VFILE_OPEN_DIRECTORY);
					CopyOneFile(FSSrc, VFS, c_file.name, newPath);
					srcPath += "\\";
					srcPath+= c_file.name;
					remove(srcPath);
				}
				++m_copiedfiles;
			}
		}
		while(_findnext(fhandle, &c_file) == 0)
		{
			if((c_file.attrib & _A_SUBDIR) == 0)
			{
				if((strcmp(c_file.name, ".") != 0) && (strcmp(c_file.name, "..") != 0))
				{
					if(CheckCopy(c_file.name))
					{
						newPath = Path;
						newPath+= "\\";
						newPath+= c_file.name;
						newPath =newPath.Mid(1);
						CString srcPath = m_currentdir;
						srcPath += "\\GameBuilder\\";
						srcPath += Path;

						if(srcPath.GetAt(srcPath.GetLength()-1) == _T('\\'))
							srcPath = srcPath.Left(srcPath.GetLength()-1);

						FSSrc = geVFile_OpenNewSystem(NULL, GE_VFILE_TYPE_DOS, srcPath, NULL, GE_VFILE_OPEN_READONLY | GE_VFILE_OPEN_DIRECTORY);
						CopyOneFile(FSSrc, VFS, c_file.name, newPath);

						srcPath += "\\";
						srcPath += c_file.name;
						remove(srcPath);
					}
					++m_copiedfiles;
				}
			}
		}
	}
	_findclose(fhandle);
}

bool CGameBuilderDlg::MakeDirectory(geVFile *FS, const char *Path)
{
	char	Buff[_MAX_PATH];
	char	*p;
	geVFile *NewDirectory;
	bool	Res;

	if(*Path == '\0')
		return false;

	p = Buff;
	while(*Path && *Path != '\\')
		*p++ = *Path++;

	*p = '\0';

	NewDirectory = geVFile_Open(FS, Buff, GE_VFILE_OPEN_DIRECTORY);
	if(!NewDirectory)
	{
		NewDirectory = geVFile_Open(FS, Buff, GE_VFILE_OPEN_CREATE | GE_VFILE_OPEN_DIRECTORY);
		if(!NewDirectory)
			return false;
	}

	if(*Path == '\\')
		Path++;

	Res = MakeDirectory(NewDirectory, Path);
	geVFile_Close(NewDirectory);
	return Res;
}

void CGameBuilderDlg::CopyFile(const char *srcPath, const char *destPath)
{
	CFile sourceFile;
	CFile destFile;
	BYTE buffer[16384];
	DWORD dwRead;

	CWaitCursor c;
	CFileException ex;
	sourceFile.Open(srcPath, CFile::modeRead | CFile::shareDenyWrite, &ex);
	destFile.Open(destPath, CFile::modeWrite | CFile::shareExclusive | CFile::modeCreate, &ex);
	do
	{
		dwRead = sourceFile.Read(buffer, 16384);
		destFile.Write(buffer, dwRead);
	}
	while(dwRead > 0);

	destFile.Close();
	sourceFile.Close();
}

void CGameBuilderDlg::CopyOneFile(geVFile *FSSrc, geVFile *FSDest, const char *src, const char *dest)
{
	geVFile *SrcFile;
	geVFile *DestFile;

	SrcFile = geVFile_Open(FSSrc, src, GE_VFILE_OPEN_READONLY);
	if(!SrcFile)
		return;
	DestFile = geVFile_Open(FSDest, dest, GE_VFILE_OPEN_CREATE);
	if(!DestFile)
		return;

	CopyVFile(SrcFile, DestFile);

	geVFile_Close(DestFile);
	geVFile_Close(SrcFile);

	return;
}

// ceck to see if we can put filename into a vfs pack
bool CGameBuilderDlg::CheckCopy(char *filename)
{
	CString Name = filename;
	Name.MakeLower();
	if(Name == "realityfactory.exe")
		return false;
	if(Name == "realityfactory.ini")
		return false;
	if(Name == "keyboard.ini")
		return false;
	if(Name == "setup.ini")
		return false;
	if(Name == "d3d24.ini")
		return false;
	if(Name == "uninstal.exe")
		return false;
	if(Name == "videosetup.exe")
		return false;
	int i = Name.Find(".dll");
	if(i>=0 && i<Name.GetLength())
		return false;
	i = Name.Find(".log");
	if(i>=0 && i<Name.GetLength())
		return false;
	i = Name.Find(".mid");
	if(i>=0 && i<Name.GetLength())
		return false;
	i = Name.Find(".s");
	if(i>=0 && i<Name.GetLength())
		return false;
	i = Name.Find(".c");
	if(i>=0 && i<Name.GetLength())
		return false;
	i = Name.Find(".jpg");
	if(i>=0 && i<Name.GetLength())
		return false;
	i = Name.Find(".png");
	if(i>=0 && i<Name.GetLength())
		return false;
	i = Name.Find(".mp3");
	if(i>=0 && i<Name.GetLength())
		return false;
	i = Name.Find(".avi");
	if(i>=0 && i<Name.GetLength())
		return false;
	i = Name.Find(".gif");
	if(i>=0 && i<Name.GetLength())
		return false;
	i = Name.Find(".wav");
	if(i>=0 && i<Name.GetLength())
	{
		if(Sindex>0)
		{
			CString Stream;
			for(int j=0;j<Sindex;j++)
			{
				Stream = Streaming.GetAt(j);
				Stream.MakeLower();
				if(Stream==Name)
					return false;
			}
		}
	}

	return true;
}

BOOL CGameBuilderDlg::RemoveEmptyDirectory(CString &sPath)
{
	CFileFind finder;

	CString  sWildCard = sPath + "\\*.*";

	BOOL bFound;
	BOOL bWorking = finder.FindFile(sWildCard);

	bFound = bWorking;

	while(bWorking)
	{
		bWorking = finder.FindNextFile();

		if(finder.IsDots()) continue;

		if(finder.IsDirectory())
		{
			CString s = finder.GetFilePath();
			RemoveEmptyDirectory(s);
			RemoveDirectory(finder.GetFilePath());
			continue;
		}
	}

	return bFound;
}

void CGameBuilderDlg::OnCancel()
{
	CDialog::OnCancel();
	m_messagelist.Destroy();
}
