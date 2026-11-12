#pragma once
#include "Game.h"
#include "D2DRenderer.h"

class CMikasSolitaireView : public CView
{
protected:
	CMikasSolitaireView() noexcept;
	DECLARE_DYNCREATE(CMikasSolitaireView)

public:
	CMikasSolitaireDoc* GetDocument() const;
	virtual void OnDraw(CDC* pDC);
	virtual void OnUpdate(CView* sender, LPARAM hint, CObject* hintObject);
	virtual ~CMikasSolitaireView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	afx_msg int OnCreate(LPCREATESTRUCT createStruct);
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnLButtonDown(UINT flags, CPoint point);
	afx_msg void OnLButtonUp(UINT flags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT flags, CPoint point);
	afx_msg void OnMouseMove(UINT flags, CPoint point);
	afx_msg void OnCaptureChanged(CWnd* window);
	afx_msg BOOL OnSetCursor(CWnd* window, UINT hitTest, UINT message);
	afx_msg void OnTimer(UINT_PTR id);
	afx_msg LRESULT OnCheckPosition(WPARAM, LPARAM);
	afx_msg void OnRButtonUp(UINT flags, CPoint point);
	afx_msg void OnOpenGame();
	afx_msg void OnRestartGame();
	afx_msg void OnUndo();
	afx_msg void OnUpdateUndo(CCmdUI* cmdUI);
	afx_msg void OnCardBack(UINT id);
	afx_msg void OnUpdateCardBack(CCmdUI* cmdUI);
	afx_msg void OnUpdateUnavailable(CCmdUI* cmdUI);
	afx_msg void OnUpdateSave(CCmdUI* cmdUI);
	DECLARE_MESSAGE_MAP()

private:
	struct Layout
	{
		int width, height, gap, left, top, row, step;
		CRect Slot(int column, int y) const;
	};
	Layout GetLayout() const;
	CGame::Location HitTest(CPoint point, const Layout& layout) const;
	void DrawCard(const CRect& rect, const CCard& card);
	void ReportRenderError(HRESULT error);
	void AfterMove();
	void CheckPosition();
	void StopAnimation();
	void CancelDrag();

	CD2DRenderer m_renderer;
	CGame::Location m_selected;
	int m_cardBack = 0;
	bool m_renderErrorShown = false;
	bool m_movedOnLastButtonDown = false;
	bool m_trackingDrag = false;
	bool m_dragging = false;
	bool m_stuckNotified = false;
	bool m_animating = false;
	CPoint m_dragStart, m_dragPoint, m_dragOffset;
	ULONGLONG m_animationStart = 0;
};

#ifndef _DEBUG
inline CMikasSolitaireDoc* CMikasSolitaireView::GetDocument() const
	{ return reinterpret_cast<CMikasSolitaireDoc*>(m_pDocument); }
#endif
