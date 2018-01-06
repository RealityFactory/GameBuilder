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

const int kActorFile = 0x0000;		// ACTOR files
const int kAudioFile = 0x0001;		// AUDIO files
const int kVideoFile = 0x0002;		// VIDEO files
const int kMIDIFile =  0x0003;		// MIDI files
const int kLevelFile = 0x0004;		// LEVEL files
const int kAudioStreamFile = 0x0005;// AUDIO STREAM files
const int kBitmapFile = 0x0006;		// BITMAP files
const int kInstallFile = 0x0007;	// Install files (main directory)
const int kBaseFile = 0x0008;
const int kScriptFile = 0x0009;

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
	DDX_Control(pDX, IDC_PROGRESS, m_status);
	DDX_Control(pDX, IDC_LEVELS, m_levels);
	DDX_Text(pDX, IDC_VFS, m_packfile);
	DDX_Text(pDX, IDC_MENU, m_menuini);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CGameBuilderDlg, CDialog)
	//{{AFX_MSG_MAP(CGameBuilderDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_ADDLEVEL, OnAddlevel)
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
	//m_menuini = _T("");
	m_splash1 = _T("");
	m_splash2 = _T("");
	m_splashaudio1 = _T("");
	m_splashaudio2 = _T("");
	playeravatar = _T("");
	character = false;
	_chdir("..");
	_getcwd(m_currentdir, 512);
	SetupDirectory();
	GetVFS();
	m_levels.ResetContent();
	Streaming.SetSize(10,-1);
	Sindex = 0;
	m_status.SetShowText(FALSE);
	m_status.SetRange(0,12);
	m_status.SetStep(1);
	m_status.SetPos(0);
	m_status.SetBgColour(RGB(196,196,196));
	
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


BOOL CGameBuilderDlg::EmptyDirectory(CString &sPath)
{
	CFileFind finder;
	
	CString  sWildCard = sPath + "\\*.*";
	
	BOOL bFound;
	BOOL bWorking = finder.FindFile(sWildCard);
	
	bFound = bWorking;
	
	while (bWorking) 
	{
		bWorking = finder.FindNextFile();
		
		if (finder.IsDots()) continue;
		
		if (finder.IsDirectory()) 
		{
			CString s = finder.GetFilePath();
			EmptyDirectory(s);
			RemoveDirectory(finder.GetFilePath());
			continue; 
		}
		_unlink( finder.GetFilePath() );
		
	}
	
	return bFound;
	
}


void CGameBuilderDlg::SetupDirectory()
{
	CString sPath = m_currentdir;
	CString sMain, sFolder;
	sPath += "\\GameBuilder";
	EmptyDirectory(sPath);
	_rmdir(sPath);
	CreateDirectory(sPath,NULL);
	sMain = sPath+"\\install";
	CreateDirectory(sMain,NULL);
	sMain = sPath+"\\scripts";
	CreateDirectory(sMain,NULL);
	sMain = sPath+"\\media";
	CreateDirectory(sMain,NULL);
	sFolder = sMain+"\\actors";
	CreateDirectory(sFolder,NULL);
	sFolder = sMain+"\\levels";
	CreateDirectory(sFolder,NULL);
	sFolder = sMain+"\\bitmaps";
	CreateDirectory(sFolder,NULL);
	sFolder = sMain+"\\bitmaps\\inventory";
	CreateDirectory(sFolder,NULL);
	sFolder = sMain+"\\bitmaps\\menu";
	CreateDirectory(sFolder,NULL);
	sFolder = sMain+"\\bitmaps\\fonts";
	CreateDirectory(sFolder,NULL);
	sFolder = sMain+"\\bitmaps\\fx";
	CreateDirectory(sFolder,NULL);
	sFolder = sMain+"\\bitmaps\\explode";
	CreateDirectory(sFolder,NULL);
	sFolder = sMain+"\\audio";
	CreateDirectory(sFolder,NULL);
	sFolder = sMain+"\\audio\\menu";
	CreateDirectory(sFolder,NULL);
	sFolder = sMain+"\\midi";
	CreateDirectory(sFolder,NULL);
	sFolder = sMain+"\\video";
	CreateDirectory(sFolder,NULL);
}

void CGameBuilderDlg::GetVFS()
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
			"RealityFactory INI Editor", MB_ICONSTOP | MB_OK);
		CDialog::OnOK();
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

void CGameBuilderDlg::OnAddlevel() 
{
	MFileDlg dlgFile(TRUE);
	TCHAR m_dir[512];
	strcpy(m_dir, m_currentdir);
	strcat(m_dir, "\\media\\levels");
	dlgFile.SetInitalDir(m_dir);
	
	CString title, strFilter, strDefault;
	VERIFY(title.LoadString(AFX_IDS_OPENFILE));
	
	// append the "*.*"  files filter
	strFilter = _T("Genesis Level Files (*.3dt)");
	strFilter += (TCHAR)'\0';   // next string please
	strFilter += _T("*.3dt");
	strFilter += (TCHAR)'\0';   // last string
	dlgFile.m_ofn.nMaxCustFilter++;
	
	dlgFile.m_ofn.lpstrFilter = strFilter;
	dlgFile.m_ofn.lpstrTitle = title;
	dlgFile.m_ofn.hwndOwner = AfxGetMainWnd()->GetSafeHwnd();
	chdir(m_currentdir);

	if (dlgFile.DoModal() == IDOK)
	{
		POSITION pos = dlgFile.GetStartPosition();
		while (pos != NULL)
		{
			CString strPath = dlgFile.GetNextPathName(pos);
			CString FileName = strPath.Mid(strPath.ReverseFind('\\')+1);
			if(m_levels.GetCount()>0)
			{
				if(m_levels.FindString(-1,FileName)!=LB_ERR)
					continue;
			}
			m_levels.AddString(FileName);
		}
		UpdateData(FALSE);
	}

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
			strcat(szTemp, "media\\actors\\");
			strcat(szTemp, destDir);
			break;
		case kAudioFile:
			strcat(szTemp, "media\\audio\\");
			strcat(szTemp, destDir);
			break;
		case kVideoFile:
			strcat(szTemp, "media\\video\\");
			strcat(szTemp, destDir);
			break;
		case kMIDIFile:
			strcat(szTemp, "media\\midi\\");
			strcat(szTemp, destDir);
			break;
		case kLevelFile:
			strcat(szTemp, "media\\levels\\");
			strcat(szTemp, destDir);
			break;
		case kAudioStreamFile:
			strcat(szTemp, "media\\audio\\");
			strcat(szTemp, destDir);
			break;
		case kBitmapFile:
			strcat(szTemp, "media\\bitmaps\\");
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
		}
		CreateDirectory(szTemp,NULL);
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
		strcat(szTemp, "media\\actors\\");
		strcat(szTemp, szFilename);
		break;
    case kAudioFile:
		strcat(szTemp, "media\\audio\\");
		strcat(szTemp, szFilename);
		break;
    case kVideoFile:
		strcat(szTemp, "media\\video\\");
		strcat(szTemp, szFilename);
		break;
	case kMIDIFile:
		strcat(szTemp, "media\\midi\\");
		strcat(szTemp, szFilename);
		break;
	case kLevelFile:
		strcat(szTemp, "media\\levels\\");
		strcat(szTemp, szFilename);
		break;
	case kAudioStreamFile:
		strcat(szTemp, "media\\audio\\");
		strcat(szTemp, szFilename);
		break;
	case kBitmapFile:
		strcat(szTemp, "media\\bitmaps\\");
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
	if(m_levels.GetCount()>0)
	{
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
		m_status.SetShowText(TRUE);
		m_status.StepIt();
		CopyMain();

		str = "Processing Setup File";
        m_status.SetWindowText(str);
		m_status.StepIt();
		CopyRFIni();

		str = "Processing Required Resources";
        m_status.SetWindowText(str);
		m_status.StepIt();
		CopyBuiltIn();

		str = "Processing Menu Resources";
        m_status.SetWindowText(str);
		m_status.StepIt();
		CopyMenu();

		if(character)
		{
			str = "Processing Character Selection";
			m_status.SetWindowText(str);
			m_status.StepIt();
			CopyCharacter();
		}
		else
			m_status.StepIt();

		str = "Processing Effects";
        m_status.SetWindowText(str);
		m_status.StepIt();
		CopyEffect();

		str = "Processing Inventory";
        m_status.SetWindowText(str);
		m_status.StepIt();
		CopyInventory();

		str = "Processing Player Data";
        m_status.SetWindowText(str);
		m_status.StepIt();
		CopyPlayerSetup();

		str = "Processing Weapons";
        m_status.SetWindowText(str);
		m_status.StepIt();
		CopyWeapon();

		str = "Processing Game Levels";
        m_status.SetWindowText(str);
		m_status.StepIt();
		CopyLevels();

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

		MessageBox("Finished Creating Game", "Game Builder", MB_ICONSTOP | MB_OK);
	}
}

void CGameBuilderDlg::OnProcOk() 
{
	// TODO: Add your control notification handler code here
	Process();
	CDialog::OnOK();
}

void CGameBuilderDlg::CopyVFile(geVFile * FmFile,geVFile *ToFile)
{
	char CopyBuf[16384];
	int CopyBufLen = 16384;
	long Size;
	geVFile_Properties Props;
	
	if ( ! geVFile_GetProperties(FmFile,&Props) )
		return;
	
	if ( ! geVFile_Size(FmFile,&Size) )
		return;
	
	while( Size )
	{
		int CurLen = min(Size,CopyBufLen);
		
		if ( ! geVFile_Read(FmFile,CopyBuf,CurLen) )
			return;
		
		if ( ! geVFile_Write(ToFile,CopyBuf,CurLen) )
			return;
		
		Size -= CurLen;
	}
	return;
}

void CGameBuilderDlg::CopyToDos(int nFileType, char *szFilename)
{
	geVFile *MainFS, *SecondFS;
	CString File = szFilename;
	char tFile[256];

	File.Replace("\\\\", "\\");
	strcpy(tFile, File);

	if(OpenRFFile(&MainFS, nFileType, tFile, GE_VFILE_OPEN_READONLY, false))
	{
		CreateDir(nFileType, tFile);
		if(OpenRFFile(&SecondFS, nFileType, tFile, GE_VFILE_OPEN_CREATE, true))
		{
			CopyVFile(MainFS, SecondFS);
			geVFile_Close(SecondFS);
			if(nFileType==kAudioStreamFile)
			{
				Streaming.SetAtGrow(Sindex, tFile);
				Sindex+=1;
			}
		}
		geVFile_Close(MainFS);
	}
}

void CGameBuilderDlg::CopyMain()
{
	TCHAR filename[512];

	CopyToDos(kBaseFile, "RealityFactory.exe");
	CopyToDos(kBaseFile, "RealityFactory.ini");
	CopyToDos(kBaseFile, "keyboard.ini");
	CopyToDos(kBaseFile, "setup.ini");
	CopyToDos(kInstallFile, "camera.ini");
	if(character)
		CopyToDos(kInstallFile, "character.ini");
	CopyToDos(kInstallFile, "effect.ini");
	CopyToDos(kInstallFile, "explosion.ini");
	CopyToDos(kInstallFile, "inventory.ini");
	CopyToDos(kInstallFile, "playersetup.ini");
	CopyToDos(kInstallFile, "weapon.ini");
	CopyToDos(kInstallFile, "armour.ini");
	CopyToDos(kInstallFile, "control.ini");
	CopyToDos(kInstallFile, "conversation.txt");
	CopyToDos(kInstallFile, "material.ini");
	if(m_menuini=="")
		m_menuini = "menu.ini";
	strcpy(filename, m_menuini);
	CopyToDos(kInstallFile, filename);
	CopyToDos(kBaseFile, "d3ddrv.dll");
	CopyToDos(kBaseFile, "decrypt.dll");
	CopyToDos(kBaseFile, "glidedrv.dll");
	CopyToDos(kBaseFile, "ogldrv.dll");
	CopyToDos(kBaseFile, "softdrv.dll");
	CopyToDos(kBaseFile, "softdrv2.dll");
}

void CGameBuilderDlg::CopyRFIni()
{
	TCHAR filename[512];

	if(m_splash1!="")
	{
		m_splash1.MakeLower();
		strcpy(filename, m_splash1);
		if(m_splash1.Find(".avi")!=-1 || m_splash1.Find(".gif")!=-1)
			CopyToDos(kVideoFile, filename);
		else
			CopyToDos(kBitmapFile, filename);
	}
	else
		CopyToDos(kBitmapFile, "rflogo.bmp");

	if(m_splashaudio1!="")
	{
		m_splashaudio1.MakeLower();
		strcpy(filename, m_splashaudio1);
		CopyToDos(kAudioFile, filename);
	}
	else
		CopyToDos(kAudioFile, "startup.wav");

	if(m_splash2!="")
	{
		m_splash2.MakeLower();
		strcpy(filename, m_splash2);
		if(m_splash2.Find(".avi")!=-1 || m_splash2.Find(".gif")!=-1)
			CopyToDos(kVideoFile, filename);
		else
			CopyToDos(kBitmapFile, filename);
	}

	if(m_splashaudio2!="")
	{
		strcpy(filename, m_splashaudio2);
		CopyToDos(kAudioFile, filename);
	}

	if(playeravatar!="")
	{
		strcpy(filename, playeravatar);
		CopyToDos(kActorFile, filename);
	}
}

void CGameBuilderDlg::CopyBuiltIn()
{

	CopyToDos(kBitmapFile, "a_lvlsmoke.bmp");
	CopyToDos(kBitmapFile, "lvlsmoke.bmp");
	CopyToDos(kBitmapFile, "corona.bmp");
	CopyToDos(kBitmapFile, "corona_a.bmp");
	CopyToDos(kBitmapFile, "bolt.bmp");
	CopyToDos(kBitmapFile, "water.bmp");
	CopyToDos(kBitmapFile, "a_water.bmp");
	CopyToDos(kBitmapFile, "flame03.bmp");
	CopyToDos(kBitmapFile, "a_flame.bmp");
	CopyToDos(kBitmapFile, "g_bubble.bmp");
	CopyToDos(kBitmapFile, "a_bubbl.bmp");
	CopyToDos(kBitmapFile, "rain.bmp");
	CopyToDos(kBitmapFile, "a_rain.bmp");
	CopyToDos(kAudioFile, "loopbzzt.wav");
	CopyToDos(kAudioFile, "onebzzt.wav");
}


char *FirstToken(char *string1, char *string2)
{
	return strtok(string1,string2);
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

	strcpy(filename, m_menuini);
	OpenRFFile(&SecondFS, kInstallFile, filename, GE_VFILE_OPEN_READONLY, true);
	char szInputLine[132];
	char *szAtom;
	char menuline[132];
	while(geVFile_GetS(SecondFS, szInputLine, 132)==GE_TRUE)
	{
		if(szInputLine[0] == ';') 
			continue;				// Comment line
		if(strlen(szInputLine) <= 5)
			continue;				// Skip blank lines
		// All config commands are "thing=value"
		szAtom = FirstToken(szInputLine," =");
		if(!stricmp(szAtom,"background"))
		{
			NextToken();
			strcpy(menuline,"menu\\");
			strcat(menuline,NextToken());
			CopyToDos(kBitmapFile, menuline);
		}
		else if(!stricmp(szAtom,"images"))
		{
			NextToken();
			strcpy(menuline,"menu\\");
			strcat(menuline,NextToken());
			CopyToDos(kBitmapFile, menuline);
			strcpy(menuline,"menu\\");
			strcat(menuline,NextToken());
			CopyToDos(kBitmapFile, menuline);
		}
		else if(!stricmp(szAtom,"titles"))
		{
			NextToken();
			strcpy(menuline,"menu\\");
			strcat(menuline,NextToken());
			CopyToDos(kBitmapFile, menuline);
			strcpy(menuline,"menu\\");
			strcat(menuline,NextToken());
			CopyToDos(kBitmapFile, menuline);
		}
		else if(!stricmp(szAtom,"animation"))
		{
			NextToken();
			strcpy(menuline,"menu\\");
			strcat(menuline,NextToken());
			CopyToDos(kVideoFile, menuline);
		}
		else if(!stricmp(szAtom,"font"))
		{
			NextToken();
			char fontline[132];
			strcpy(fontline,NextToken());
			strcpy(menuline,"fonts\\");
			strcat(menuline,fontline);
			strcat(menuline,".bmp");
			CopyToDos(kBitmapFile, menuline);
			strcpy(menuline,"fonts\\a_");
			strcat(menuline,fontline);
			strcat(menuline,".bmp");
			CopyToDos(kBitmapFile, menuline);
			strcpy(menuline,"fonts\\");
			strcat(menuline,fontline);
			strcat(menuline,".dat");
			CopyToDos(kBitmapFile, menuline);
		}
		else if(!stricmp(szAtom,"cursor"))
		{
			strcpy(menuline,"menu\\");
			strcat(menuline,NextToken());
			CopyToDos(kBitmapFile, menuline);
			strcpy(menuline,"menu\\");
			strcat(menuline,NextToken());
			CopyToDos(kBitmapFile, menuline);
		}
		else if(!stricmp(szAtom,"crosshair"))
		{
			strcpy(menuline,"menu\\");
			strcat(menuline,NextToken());
			CopyToDos(kBitmapFile, menuline);
			strcpy(menuline,"menu\\");
			strcat(menuline,NextToken());
			CopyToDos(kBitmapFile, menuline);
		}
		else if(!stricmp(szAtom,"mouseclick"))
		{
			char file[256] = "menu\\";
			strcat(file, NextToken());
			CopyToDos(kAudioFile, file);
		}
		else if(!stricmp(szAtom,"keyclick"))
		{
			char file[256] = "menu\\";
			strcat(file, NextToken());
			CopyToDos(kAudioFile, file);
		}
		else if(!stricmp(szAtom,"slideclick"))
		{
			char file[256] = "menu\\";
			strcat(file, NextToken());
			CopyToDos(kAudioFile, file);
		}
		else if(!stricmp(szAtom,"music"))
		{
			char *musicname = NextToken();
			int len = strlen(musicname)-4;
			if(!stricmp((musicname+len),".mid"))
				CopyToDos(kMIDIFile, musicname);
			else
				CopyToDos(kAudioStreamFile, musicname);
		}
		else if(!stricmp(szAtom,"loadscreen"))
		{
			NextToken();
			strcpy(menuline,"menu\\");
			strcat(menuline,NextToken());
			CopyToDos(kBitmapFile, menuline);
		}
	}
	geVFile_Close(SecondFS);
}

void CGameBuilderDlg::CopyCharacter()
{
	CString readinfo;
	CString valuename, value;
	geVFile *MainFS;
	char szInputLine[132];
	char file[256];

	OpenRFFile(&MainFS, kInstallFile, "character.ini", GE_VFILE_OPEN_READONLY, false);
	while(geVFile_GetS(MainFS, szInputLine, 132)==GE_TRUE)
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
				if(valuename=="image")
					CopyToDos(kBitmapFile, file);
			}
			
		}
	}
	geVFile_Close(MainFS);
}

void CGameBuilderDlg::CopyEffect()
{
	CString readinfo;
	CString valuename, value;
	geVFile *MainFS, *SecondFS;;
	char szInputLine[132];
	char file[256];

	OpenRFFile(&MainFS, kInstallFile, "effect.ini", GE_VFILE_OPEN_READONLY, false);
	while(geVFile_GetS(MainFS, szInputLine, 132)==GE_TRUE)
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
				if(valuename=="bitmapname" || valuename=="alphamapname")
					CopyToDos(kBitmapFile, file);
				if(valuename=="name")
					CopyToDos(kAudioFile, file);
				if(valuename=="basebitmapname" || valuename=="basealphamapname")
				{
					char base[132];
					int i = 0;
					while(1)
					{
						sprintf(base, "%s%d%s", file, i, ".bmp" );
						if(!OpenRFFile(&SecondFS, kBitmapFile, base, GE_VFILE_OPEN_READONLY, false))
							break;
						geVFile_Close(SecondFS);
						CopyToDos(kBitmapFile, base);
						i+=1;
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
	geVFile *MainFS;
	char szInputLine[132];
	char file[256];

	OpenRFFile(&MainFS, kInstallFile, "inventory.ini", GE_VFILE_OPEN_READONLY, false);
	while(geVFile_GetS(MainFS, szInputLine, 132)==GE_TRUE)
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
				if(valuename=="background" || valuename=="highlight"
					 || valuename=="highlightalpha" || valuename=="image"
					  || valuename=="imagealpha")
				{
					strcpy(file, "Inventory\\");
					strcat(file, value);
					CopyToDos(kBitmapFile, file);
				}
				if(valuename=="keysound")
					CopyToDos(kAudioFile, file);
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
	char szInputLine[132];
	char file[256];

	OpenRFFile(&MainFS, kInstallFile, "playersetup.ini", GE_VFILE_OPEN_READONLY, false);
	while(geVFile_GetS(MainFS, szInputLine, 132)==GE_TRUE)
	{
		if(strlen(szInputLine) <= 1)
			continue;
		readinfo = szInputLine;
		readinfo.TrimRight();

		if (readinfo != "")
		{
			if (readinfo[0] == '[' && readinfo[readinfo.GetLength()-1] == ']') //if a section heading
			{
				keyname = readinfo;
				keyname.TrimLeft('[');
				keyname.TrimRight(']');
			}
			else
			{
				if(readinfo[0] != ';')
				{
					valuename = readinfo.Left(readinfo.Find("="));
					value = readinfo.Right(readinfo.GetLength()-valuename.GetLength()-1);
					valuename.TrimLeft();
					valuename.TrimRight();
					value.TrimLeft();
					value.TrimRight();
					strcpy(file, value);
					if(keyname=="Sounds")
					{
						if(valuename=="die" || valuename=="injury" || valuename=="land")
						{
							char strip[256], *temp;
							int i = 0;
							strcpy(strip,value);
							temp = strtok(strip," \n");
							while(temp)
							{
								CopyToDos(kAudioFile, temp);
								i+=1;
								if(i==5)
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

void CGameBuilderDlg::CopyWeapon()
{
	CString readinfo;
	CString valuename, value;
	geVFile *MainFS;
	char szInputLine[132];
	char file[256];

	OpenRFFile(&MainFS, kInstallFile, "weapon.ini", GE_VFILE_OPEN_READONLY, false);
	while(geVFile_GetS(MainFS, szInputLine, 132)==GE_TRUE)
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
				if(valuename=="actor" || valuename=="viewactor" || valuename=="playeractor")
					CopyToDos(kActorFile, file);
				if(valuename=="movesound" || valuename=="impactsound"
					|| valuename=="bouncesound" || valuename=="attacksound"
					|| valuename=="emptysound" || valuename=="hitsound")
					CopyToDos(kAudioFile, file);
				if(valuename=="crosshair" || valuename=="crosshairalpha")
					CopyToDos(kBitmapFile, file);
			}
			
		}
	}
	geVFile_Close(MainFS);
}

void CGameBuilderDlg::CopyLevels()
{
	CString Level; 
	CString str;
	char file[256];
	char bsp[256];
	char szInputLine[132];
	geVFile *MainFS, *SecondFS;
	int i, j, ii;

	for(ii=0;ii<m_levels.GetCount();ii++)
	{
		m_levels.GetText(ii, file);
		Level = file;
		Level.MakeLower();
		Level.Replace("3dt", "bsp");
		strcpy(bsp, Level);
		CopyToDos(kLevelFile, bsp);
		OpenRFFile(&MainFS, kLevelFile, file, GE_VFILE_OPEN_READONLY, false);
		while(geVFile_GetS(MainFS, szInputLine, 132)==GE_TRUE)
		{
			if(strlen(szInputLine) <= 1)
				continue;
			str = szInputLine;
			str.TrimRight();
			str.MakeLower();
			i = str.Find(".wav");
			if(i>=0 && i<str.GetLength())
			{
				j=i-1;
				while(str.GetAt(j)!='"' && j>=0)
				{
					j-=1;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1,i-j+3));
					int type = kAudioFile;
					i = str.Find("szstreamingaudio");
					if(i>=0 && i<str.GetLength())
						type = kAudioStreamFile;
					i = str.Find("szstreamfile");
					if(i>=0 && i<str.GetLength())
						type = kAudioStreamFile;
					CopyToDos(type, bsp);
				}
			}
			i = str.Find(".mp3");
			if(i>=0 && i<str.GetLength())
			{
				j=i-1;
				while(str.GetAt(j)!='"' && j>=0)
				{
					j-=1;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1,i-j+3));
					CopyToDos(kAudioFile, bsp);
				}
			}
			i = str.Find(".bmp");
			if(i>=0 && i<str.GetLength())
			{
				j=i-1;
				while(str.GetAt(j)!='"' && j>=0)
				{
					j-=1;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1,i-j+3));
					CopyToDos(kBitmapFile, bsp);
				}
			}
			i = str.Find(".mid");
			if(i>=0 && i<str.GetLength())
			{
				j=i-1;
				while(str.GetAt(j)!='"' && j>=0)
				{
					j-=1;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1,i-j+3));
					CopyToDos(kMIDIFile, bsp);
				}
			}
			i = str.Find(".avi");
			if(i>=0 && i<str.GetLength())
			{
				j=i-1;
				while(str.GetAt(j)!='"' && j>=0)
				{
					j-=1;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1,i-j+3));
					CopyToDos(kVideoFile, bsp);
				}
			}
			i = str.Find(".gif");
			if(i>=0 && i<str.GetLength())
			{
				j=i-1;
				while(str.GetAt(j)!='"' && j>=0)
				{
					j-=1;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1,i-j+3));
					CopyToDos(kVideoFile, bsp);
				}
			}
			i = str.Find(".act");
			if(i>=0 && i<str.GetLength())
			{
				j=i-1;
				while(str.GetAt(j)!='"' && j>=0)
				{
					j-=1;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1,i-j+3));
					CopyToDos(kActorFile, bsp);
				}
			}
			i = str.Find("key attributeinfofile value");
			if(i>=0 && i<str.GetLength())
			{
				i = str.Find(".");
				j=i-1;
				while(str.GetAt(j)!='"' && j>=0)
				{
					j-=1;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1,i-j+3));
					CopyToDos(kInstallFile, bsp);
				}
			}
			i = str.Find("key hudinfofile value");
			if(i>=0 && i<str.GetLength())
			{
				i = str.Find(".");
				j=i-1;
				while(str.GetAt(j)!='"' && j>=0)
				{
					j-=1;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1,i-j+3));
					CopyToDos(kInstallFile, bsp);
					CopyHud(bsp);

				}
			}
			i = str.Find("key alphanamebase value");
			if(i>=0 && i<str.GetLength())
			{
				j=str.GetLength()-2;
				while(str.GetAt(j)!='"' && j>=0)
				{
					j-=1;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1,str.GetLength()-j-2));

					char base[132];
					int ij = 0;
					while(1)
					{
						sprintf(base, "%s%d%s", bsp, ij, ".bmp" );
						if(!OpenRFFile(&SecondFS, kBitmapFile, base, GE_VFILE_OPEN_READONLY, false))
							break;
						geVFile_Close(SecondFS);
						CopyToDos(kBitmapFile, base);
						ij+=1;
					}
				}
			}
			i = str.Find("key bmpnamebase value");
			if(i>=0 && i<str.GetLength())
			{
				j=str.GetLength()-2;
				while(str.GetAt(j)!='"' && j>=0)
				{
					j-=1;
				}
				if(j>=0)
				{
					strcpy(bsp, str.Mid(j+1,str.GetLength()-j-2));

					char base[132];
					int ij = 0;
					while(1)
					{
						sprintf(base, "%s%d%s", bsp, ij, ".bmp" );
						if(!OpenRFFile(&SecondFS, kBitmapFile, base, GE_VFILE_OPEN_READONLY, false))
							break;
						geVFile_Close(SecondFS);
						CopyToDos(kBitmapFile, base);
						ij+=1;
					}
				}
			}
		}
		geVFile_Close(MainFS);
	}
}

void CGameBuilderDlg::CopyHud(char *filename)
{
	CString readinfo;
	CString valuename, value;
	geVFile *MainFS;
	char szInputLine[132];
	char file[256];

	OpenRFFile(&MainFS, kInstallFile, filename, GE_VFILE_OPEN_READONLY, false);
	while(geVFile_GetS(MainFS, szInputLine, 132)==GE_TRUE)
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
				if(valuename=="frame" || valuename=="framealpha"
					 || valuename=="indicatoralpha" || valuename=="indicator"
					  || valuename=="npcindicator" || valuename=="npcindicatoralpha")
					CopyToDos(kBitmapFile, file);
			}
			
		}
	}
	geVFile_Close(MainFS);
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
		MessageBox("Unable to Get VFS for "+m_packfile, "RealityFactory VFS", MB_ICONSTOP | MB_OK);
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
	if(Path=="")
		searchString+= "*.*";
	else
	{
		searchString += Path;
		searchString+= "\\*.*";
	}

	if ((fhandle=_findfirst( searchString, &c_file ))!=-1) 
	{	
		if ((c_file.attrib & _A_SUBDIR)==_A_SUBDIR) 
		{
			if ((strcmp(c_file.name, ".")!=0) && (strcmp(c_file.name, "..")!=0)) 
			{
				newPath = Path;
				newPath+= "\\";
				newPath+= c_file.name;
				MakeDirectory(VFS, newPath);
				GetFiles(newPath, VFS);
				GetDir(newPath, VFS);
			}
		}
		while(_findnext( fhandle, &c_file ) == 0 ) 
		{	
			if ((c_file.attrib & _A_SUBDIR)==_A_SUBDIR) 
			{
				if ((strcmp(c_file.name, ".")!=0) && (strcmp(c_file.name, "..")!=0)) 
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
	if(Path=="")
		searchString+= "*.*";
	else
	{
		searchString += Path;
		searchString+= "\\*.*";
	}

	if ((fhandle=_findfirst( searchString, &c_file ))!=-1) 
	{	
		if ((c_file.attrib & _A_SUBDIR)==0) 
		{
			if ((strcmp(c_file.name, ".")!=0) && (strcmp(c_file.name, "..")!=0)) 
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
					if (srcPath.GetAt(srcPath.GetLength()-1) == _T('\\'))
						srcPath = srcPath.Left(srcPath.GetLength()-1);
					FSSrc = geVFile_OpenNewSystem(NULL, GE_VFILE_TYPE_DOS, srcPath, NULL, GE_VFILE_OPEN_READONLY | GE_VFILE_OPEN_DIRECTORY);
					CopyOneFile(FSSrc, VFS, c_file.name, newPath);

					srcPath += "\\";
					srcPath+= c_file.name;
					remove(srcPath);
				}
			}
		}
		while(_findnext( fhandle, &c_file ) == 0 ) 
		{		
			if ((c_file.attrib & _A_SUBDIR)==0) 
			{
				if ((strcmp(c_file.name, ".")!=0) && (strcmp(c_file.name, "..")!=0)) 
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
						if (srcPath.GetAt(srcPath.GetLength()-1) == _T('\\'))
							srcPath = srcPath.Left(srcPath.GetLength()-1);
						FSSrc = geVFile_OpenNewSystem(NULL, GE_VFILE_TYPE_DOS, srcPath, NULL, GE_VFILE_OPEN_READONLY | GE_VFILE_OPEN_DIRECTORY);
						CopyOneFile(FSSrc, VFS, c_file.name, newPath);
						
						srcPath += "\\";
						srcPath+= c_file.name;
						remove(srcPath);
					}
				}
			}
		}
	}
	_findclose(fhandle);
}

bool CGameBuilderDlg::MakeDirectory(geVFile *FS, const char *Path)
{
	char		Buff[_MAX_PATH];
	char *		p;
	geVFile *		NewDirectory;
	bool	Res;

	if	(*Path == '\0')
		return false;

	p = Buff;
	while	(*Path && *Path != '\\')
		*p++ = *Path++;

	*p = '\0';

	NewDirectory = geVFile_Open(FS, Buff, GE_VFILE_OPEN_DIRECTORY);
	if	(!NewDirectory)
	{
		NewDirectory = geVFile_Open(FS, Buff, GE_VFILE_OPEN_CREATE | GE_VFILE_OPEN_DIRECTORY);
		if	(!NewDirectory)
			return false;
	}

	if	(*Path == '\\')
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
	while (dwRead > 0);
	
	destFile.Close();
	sourceFile.Close();
}

void CGameBuilderDlg::CopyOneFile(geVFile *FSSrc, geVFile *FSDest, const char *src, const char *dest)
{
	geVFile *		SrcFile;
	geVFile *     DestFile;

	SrcFile = geVFile_Open(FSSrc, src, GE_VFILE_OPEN_READONLY);
	if	(!SrcFile)
		return;
	DestFile = geVFile_Open(FSDest, dest, GE_VFILE_OPEN_CREATE);
	if	(!DestFile)
		return;

	CopyVFile(SrcFile, DestFile);

	geVFile_Close(DestFile);
	geVFile_Close(SrcFile);

	return;
}

bool CGameBuilderDlg::CheckCopy(char *filename)
{
	CString Name = filename;
	Name.MakeLower();
	if(Name=="realityfactory.exe")
		return false;
	if(Name=="realityfactory.ini")
		return false;
	if(Name=="keyboard.ini")
		return false;
	if(Name=="setup.ini")
		return false;
	int i = Name.Find(".dll");
	if(i>=0 && i<Name.GetLength())
		return false;
	i = Name.Find(".mid");
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
			for(int j=0;j<Sindex;j++)
			{
				if(Streaming.GetAt(j)==Name)
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
	
	while (bWorking) 
	{
		bWorking = finder.FindNextFile();
		
		if (finder.IsDots()) continue;
		
		if (finder.IsDirectory()) 
		{
			CString s = finder.GetFilePath();
			RemoveEmptyDirectory(s);
			RemoveDirectory(finder.GetFilePath());
			continue; 
		}
		
	}
	
	return bFound;
	
}
