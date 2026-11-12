
// MikasSolitaire.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "MikasSolitaire.h"
#include "MainFrm.h"

#include "MikasSolitaireDoc.h"
#include "MikasSolitaireView.h"
#include <htmlhelp.h>
#pragma comment(lib, "htmlhelp.lib")

namespace
{
	constexpr int FinnishMenuVersion = 1;
}

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMikasSolitaireApp

BEGIN_MESSAGE_MAP(CMikasSolitaireApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CMikasSolitaireApp::OnAppAbout)
	ON_COMMAND(ID_GAME_HELP, &CMikasSolitaireApp::OnHelpContents)
	ON_COMMAND_RANGE(ID_LANGUAGE_ENGLISH, ID_LANGUAGE_FINNISH, &CMikasSolitaireApp::OnLanguage)
	ON_UPDATE_COMMAND_UI_RANGE(ID_LANGUAGE_ENGLISH, ID_LANGUAGE_FINNISH, &CMikasSolitaireApp::OnUpdateLanguage)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
END_MESSAGE_MAP()


// CMikasSolitaireApp construction

CMikasSolitaireApp::CMikasSolitaireApp() noexcept
{
	m_bHiColorIcons = TRUE;


	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// If the application is built using Common Language Runtime support (/clr):
	//     1) This additional setting is needed for Restart Manager support to work properly.
	//     2) In your project, you must add a reference to System.Windows.Forms in order to build.
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("MikasSolitaire.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CMikasSolitaireApp object

CMikasSolitaireApp theApp;


// CMikasSolitaireApp initialization

BOOL CMikasSolitaireApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	m_language = GetProfileInt(L"Game", L"Language", 0) == 1 ? 1 : 0;
	m_requestedLanguage = m_language;
	LANGID language = m_language ? MAKELANGID(LANG_FINNISH, SUBLANG_DEFAULT) :
		MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
	if (SetThreadUILanguage(language) != language)
	{
		AfxMessageBox(L"Unable to set the application language. / Ohjelman kielt\u00e4 ei voitu asettaa.", MB_ICONERROR);
		return FALSE;
	}

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);

	// AfxInitRichEdit2() is required to use RichEdit control
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)


	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CMikasSolitaireDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CMikasSolitaireView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);


	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);


	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	if (GetProfileInt(L"Game", L"MenuLanguage", -1) != m_language ||
		(m_language == 1 && GetProfileInt(L"Game", L"FinnishMenuVersion", 0) != FinnishMenuVersion))
	{
		CMainFrame* frame = DYNAMIC_DOWNCAST(CMainFrame, m_pMainWnd);
		if (!frame || !frame->ResetLanguageMenu())
		{
			AfxMessageBox(IDS_MENU_RESET_FAILED, MB_ICONERROR);
			return FALSE;
		}
	}

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();
	return TRUE;
}

int CMikasSolitaireApp::ExitInstance()
{
	::HtmlHelpW(nullptr, nullptr, HH_CLOSE_ALL, 0);
	//TODO: handle additional resources you may have added
	AfxOleTerm(FALSE);

	return CWinAppEx::ExitInstance();
}

void CMikasSolitaireApp::OnHelpContents()
{
	wchar_t path[32768];
	DWORD length = GetModuleFileNameW(nullptr, path, _countof(path));
	if (length == 0 || length >= _countof(path))
	{
		AfxMessageBox(IDS_HELP_DIRECTORY_FAILED, MB_ICONERROR);
		return;
	}
	CString helpPath(path);
	int separator = helpPath.ReverseFind(L'\\');
	if (separator < 0)
	{
		AfxMessageBox(IDS_HELP_DIRECTORY_FAILED, MB_ICONERROR);
		return;
	}
	helpPath = helpPath.Left(separator + 1) + (IsFinnish() ? L"MikasSolitaire-fi.chm" : L"MikasSolitaire.chm");
	DWORD attributes = GetFileAttributesW(helpPath);
	if (attributes == INVALID_FILE_ATTRIBUTES || (attributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		CString message;
		message.Format(IDS_HELP_MISSING, helpPath.GetString());
		AfxMessageBox(message, MB_ICONERROR);
		return;
	}
	if (!::HtmlHelpW(m_pMainWnd ? m_pMainWnd->GetSafeHwnd() : nullptr, helpPath, HH_DISPLAY_TOC, 0))
		AfxMessageBox(IDS_HELP_OPEN_FAILED, MB_ICONERROR);
}

void CMikasSolitaireApp::OnLanguage(UINT id)
{
	int language = id == ID_LANGUAGE_FINNISH ? 1 : 0;
	if (language == m_requestedLanguage) return;
	if (language != m_language &&
		AfxMessageBox(IDS_LANGUAGE_CONFIRM, MB_YESNO | MB_ICONQUESTION) != IDYES) return;
	if (!WriteProfileInt(L"Game", L"Language", language))
	{
		AfxMessageBox(IDS_LANGUAGE_SAVE_FAILED, MB_ICONERROR);
		return;
	}
	m_requestedLanguage = language;
	if (language != m_language) AfxMessageBox(IDS_LANGUAGE_RESTART, MB_ICONINFORMATION);
}

void CMikasSolitaireApp::OnUpdateLanguage(CCmdUI* cmdUI)
{
	cmdUI->SetRadio(cmdUI->m_nID == ID_LANGUAGE_ENGLISH + m_requestedLanguage);
}

// CMikasSolitaireApp message handlers


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg() noexcept;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() noexcept : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// App command to run the dialog
void CMikasSolitaireApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CMikasSolitaireApp customization load/save methods

void CMikasSolitaireApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
}

void CMikasSolitaireApp::LoadCustomState()
{
}

void CMikasSolitaireApp::SaveCustomState()
{
	BOOL languageSaved = WriteProfileInt(L"Game", L"MenuLanguage", m_language);
	BOOL versionSaved = WriteProfileInt(L"Game", L"FinnishMenuVersion", FinnishMenuVersion);
	if (!languageSaved || !versionSaved)
		AfxMessageBox(IDS_LANGUAGE_SAVE_FAILED, MB_ICONWARNING);
}

// CMikasSolitaireApp message handlers



