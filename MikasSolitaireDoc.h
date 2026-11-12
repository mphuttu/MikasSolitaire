
// MikasSolitaireDoc.h : interface of the CMikasSolitaireDoc class
//


#pragma once
#include "Game.h"

class CMikasSolitaireDoc : public CDocument
{
protected: // create from serialization only
	CMikasSolitaireDoc() noexcept;
	DECLARE_DYNCREATE(CMikasSolitaireDoc)

// Attributes
public:
	CGame& Game() { return m_game; }
	const CGame& Game() const { return m_game; }
	BOOL ConfirmSaveChanges() { return SaveModified(); }

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR path);
	virtual BOOL OnSaveDocument(LPCTSTR path);
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CMikasSolitaireDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CGame m_game;
	virtual BOOL DoSave(LPCTSTR path, BOOL replace = TRUE);

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
};
