
// MikasSolitaire.h : main header file for the MikasSolitaire application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CMikasSolitaireApp:
// See MikasSolitaire.cpp for the implementation of this class
//

class CMikasSolitaireApp : public CWinAppEx
{
public:
	CMikasSolitaireApp() noexcept;


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	BOOL  m_bHiColorIcons;
	bool IsFinnish() const { return m_language == 1; }

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	afx_msg void OnHelpContents();
	afx_msg void OnLanguage(UINT id);
	afx_msg void OnUpdateLanguage(CCmdUI* cmdUI);
	DECLARE_MESSAGE_MAP()

private:
	int m_language = 0;
	int m_requestedLanguage = 0;
};

extern CMikasSolitaireApp theApp;
