
// MikasSolitaireDoc.cpp : implementation of the CMikasSolitaireDoc class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "MikasSolitaire.h"
#endif

#include "MikasSolitaireDoc.h"
#include <utility>

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMikasSolitaireDoc

IMPLEMENT_DYNCREATE(CMikasSolitaireDoc, CDocument)

BEGIN_MESSAGE_MAP(CMikasSolitaireDoc, CDocument)
END_MESSAGE_MAP()


// CMikasSolitaireDoc construction/destruction

CMikasSolitaireDoc::CMikasSolitaireDoc() noexcept
{
	// TODO: add one-time construction code here

}

CMikasSolitaireDoc::~CMikasSolitaireDoc()
{
}

BOOL CMikasSolitaireDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	m_game.Initialize();
	UpdateAllViews(nullptr);
	return TRUE;
}

void CMikasSolitaireDoc::Serialize(CArchive& ar)
{
	m_game.Serialize(ar);
}

BOOL CMikasSolitaireDoc::OnOpenDocument(LPCTSTR path)
{
	try
	{
		CFile file(path, CFile::modeRead | CFile::shareDenyWrite);
		CArchive archive(&file, CArchive::load);
		CGame loaded;
		loaded.Serialize(archive);
		archive.Close();
		file.Close();
		m_game = std::move(loaded);
		SetModifiedFlag(FALSE);
		UpdateAllViews(nullptr);
		return TRUE;
	}
	catch (CException* error)
	{
		ReportSaveLoadException(path, error, FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
		error->Delete();
		return FALSE;
	}
}

BOOL CMikasSolitaireDoc::OnSaveDocument(LPCTSTR path)
{
	wchar_t fullPath[32768];
	DWORD length = GetFullPathNameW(path, _countof(fullPath), fullPath, nullptr);
	if (length == 0 || length >= _countof(fullPath))
	{
		AfxMessageBox(IDS_SAVE_PATH_FAILED, MB_ICONERROR);
		return FALSE;
	}
	CString directory(fullPath);
	int separator = directory.ReverseFind(L'\\');
	if (separator < 0)
	{
		AfxMessageBox(IDS_SAVE_DIRECTORY_FAILED, MB_ICONERROR);
		return FALSE;
	}
	// Keep the separator for drive roots such as C:\.
	directory = directory.Left(separator + 1);
	wchar_t temporary[MAX_PATH] = {};
	if (!GetTempFileNameW(directory, L"mks", 0, temporary))
	{
		CFileException error;
		error.m_lOsError = GetLastError();
		error.m_cause = CFileException::OsErrorToException(error.m_lOsError);
		error.m_strFileName = directory;
		ReportSaveLoadException(path, &error, TRUE, AFX_IDP_FAILED_TO_SAVE_DOC);
		return FALSE;
	}
	try
	{
		CFile file(temporary, CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive);
		CArchive archive(&file, CArchive::store);
		Serialize(archive);
		archive.Close();
		file.Close();
		if (!MoveFileExW(temporary, fullPath, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
			CFileException::ThrowOsError(GetLastError(), fullPath);
		SetModifiedFlag(FALSE);
		return TRUE;
	}
	catch (CException* error)
	{
		ReportSaveLoadException(path, error, TRUE, AFX_IDP_FAILED_TO_SAVE_DOC);
		error->Delete();
		if (!DeleteFileW(temporary))
		{
			CString message;
			message.Format(IDS_TEMP_REMOVE_FAILED, temporary);
			AfxMessageBox(message, MB_ICONWARNING);
		}
		return FALSE;
	}
}

BOOL CMikasSolitaireDoc::DoSave(LPCTSTR path, BOOL replace)
{
	CString name(path);
	if (name.IsEmpty())
	{
		name = GetPathName();
		if (replace && name.IsEmpty())
		{
			name = GetTitle();
			int invalid = name.FindOneOf(L":/\\");
			if (invalid >= 0) name = name.Left(invalid);
			name += L".mksol";
		}
		if (!AfxGetApp()->DoPromptFileName(name,
			replace ? AFX_IDS_SAVEFILE : AFX_IDS_SAVEFILECOPY,
			OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, FALSE, GetDocTemplate()))
			return FALSE;
	}
	CWaitCursor wait;
	if (!OnSaveDocument(name)) return FALSE;
	if (replace)
	{
		SetPathName(name);
		OnDocumentEvent(onAfterSaveDocument);
	}
	return TRUE;
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CMikasSolitaireDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CMikasSolitaireDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data.
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CMikasSolitaireDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = nullptr;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != nullptr)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CMikasSolitaireDoc diagnostics

#ifdef _DEBUG
void CMikasSolitaireDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMikasSolitaireDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CMikasSolitaireDoc commands
