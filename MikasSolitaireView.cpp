#include "pch.h"
#include "framework.h"
#include "MikasSolitaire.h"
#include "MikasSolitaireDoc.h"
#include "MikasSolitaireView.h"
#include <algorithm>

namespace
{
	constexpr UINT_PTR WinTimer = 1;
	constexpr UINT CheckPositionMessage = WM_APP + 1;
}

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMikasSolitaireView, CView)

BEGIN_MESSAGE_MAP(CMikasSolitaireView, CView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	ON_WM_CAPTURECHANGED()
	ON_WM_SETCURSOR()
	ON_WM_TIMER()
	ON_MESSAGE(CheckPositionMessage, &CMikasSolitaireView::OnCheckPosition)
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_FILE_OPEN, &CMikasSolitaireView::OnOpenGame)
	ON_COMMAND(ID_GAME_RESTART, &CMikasSolitaireView::OnRestartGame)
	ON_COMMAND(ID_EDIT_UNDO, &CMikasSolitaireView::OnUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, &CMikasSolitaireView::OnUpdateUndo)
	ON_COMMAND_RANGE(ID_CARD_BACK_1, ID_CARD_BACK_6, &CMikasSolitaireView::OnCardBack)
	ON_UPDATE_COMMAND_UI_RANGE(ID_CARD_BACK_1, ID_CARD_BACK_6, &CMikasSolitaireView::OnUpdateCardBack)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, &CMikasSolitaireView::OnUpdateSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT, &CMikasSolitaireView::OnUpdateUnavailable)
END_MESSAGE_MAP()

CMikasSolitaireView::CMikasSolitaireView() noexcept {}
CMikasSolitaireView::~CMikasSolitaireView() {}

int CMikasSolitaireView::OnCreate(LPCREATESTRUCT createStruct)
{
	if (CView::OnCreate(createStruct) == -1) return -1;
	CString error;
	if (!m_renderer.Initialize(error))
	{
		AfxMessageBox(error, MB_ICONERROR);
		return -1;
	}
	m_cardBack = theApp.GetProfileInt(L"Game", L"CardBack", 0);
	if (m_cardBack < 0 || m_cardBack >= 6) m_cardBack = 0;
	return 0;
}

CRect CMikasSolitaireView::Layout::Slot(int column, int y) const
{
	int x = left + column * (width + gap);
	return CRect(x, y, x + width, y + height);
}

CMikasSolitaireView::Layout CMikasSolitaireView::GetLayout() const
{
	CRect client;
	GetClientRect(&client);
	const CGame& m_game = GetDocument()->Game();
	size_t longest = 1;
	for (int i = 0; i < 7; ++i)
		longest = (std::max)(longest, m_game.Tableau(i).size());
	// Fit every exposed card in the client area, even in long columns.
	double scale = (std::min)(client.Width() / 840.0,
		client.Height() / (336.0 + 26.0 * (longest - 1)));
	scale = (std::max)(scale, 0.05);
	Layout layout;
	layout.width = (std::max)(1, static_cast<int>(100 * scale));
	layout.height = (std::max)(1, static_cast<int>(138 * scale));
	layout.gap = (std::max)(1, static_cast<int>(20 * scale));
	layout.step = (std::max)(1, static_cast<int>(26 * scale));
	layout.left = (std::max)(0, (client.Width() - 7 * layout.width - 6 * layout.gap) / 2);
	layout.top = (std::max)(1, static_cast<int>(25 * scale));
	layout.row = layout.top + layout.height + (std::max)(1, static_cast<int>(35 * scale));
	return layout;
}

void CMikasSolitaireView::DrawCard(const CRect& rect, const CCard& card)
{
	m_renderer.DrawBitmap(card.IsFaceUp() ? m_renderer.Face(card) : m_renderer.Back(m_cardBack), rect);
}

void CMikasSolitaireView::OnDraw(CDC* dc)
{
	CRect client;
	GetClientRect(&client);
	if (client.IsRectEmpty()) return;
	const CGame& m_game = GetDocument()->Game();
	Layout layout = GetLayout();
	HRESULT hr = m_renderer.Prepare(GetSafeHwnd(), client.Width(), client.Height(), layout.width, layout.height);
	if (FAILED(hr))
	{
		dc->FillSolidRect(client, RGB(0, 100, 54));
		ReportRenderError(hr);
		return;
	}
	m_renderErrorShown = false;
	m_renderer.BeginDraw();
	auto slot = [&](const CRect& rect)
	{
		m_renderer.DrawOutline(rect, D2D1::ColorF(160.0f / 255, 195.0f / 255, 160.0f / 255));
	};
	CRect stock = layout.Slot(0, layout.top);
	slot(stock);
	if (!m_game.Stock().empty()) m_renderer.DrawBitmap(m_renderer.Back(m_cardBack), stock);
	CRect waste = layout.Slot(1, layout.top);
	slot(waste);
	if (!m_game.Waste().empty() && !(m_dragging && m_selected.pile == CGame::Pile::Waste))
		DrawCard(waste, m_game.Waste().back());
	for (int i = 0; i < 4; ++i)
	{
		CRect rect = layout.Slot(i + 3, layout.top);
		slot(rect);
		if (!m_game.Foundation(i).empty() && !(m_dragging && m_selected.pile == CGame::Pile::Foundation &&
			m_selected.column == i)) DrawCard(rect, m_game.Foundation(i).back());
	}
	for (int i = 0; i < 7; ++i)
	{
		const auto& pile = m_game.Tableau(i);
		CRect rect = layout.Slot(i, layout.row);
		slot(rect);
		for (size_t j = 0; j < pile.size(); ++j)
		{
			if (m_dragging && m_selected.pile == CGame::Pile::Tableau &&
				m_selected.column == i && static_cast<int>(j) >= m_selected.card) break;
			rect = layout.Slot(i, layout.row + static_cast<int>(j) * layout.step);
			DrawCard(rect, pile[j]);
		}
	}
	if (m_dragging)
	{
		CRect rect(m_dragPoint.x - m_dragOffset.x, m_dragPoint.y - m_dragOffset.y,
			m_dragPoint.x - m_dragOffset.x + layout.width, m_dragPoint.y - m_dragOffset.y + layout.height);
		if (m_selected.pile == CGame::Pile::Waste) DrawCard(rect, m_game.Waste().back());
		else if (m_selected.pile == CGame::Pile::Foundation)
			DrawCard(rect, m_game.Foundation(m_selected.column).back());
		else if (m_selected.pile == CGame::Pile::Tableau)
		{
			const auto& pile = m_game.Tableau(m_selected.column);
			for (size_t i = static_cast<size_t>(m_selected.card); i < pile.size(); ++i)
			{
				DrawCard(rect, pile[i]);
				rect.OffsetRect(0, layout.step);
			}
		}
	}
	CRect selected;
	if (m_selected.pile == CGame::Pile::Waste) selected = waste;
	else if (m_selected.pile == CGame::Pile::Foundation)
		selected = layout.Slot(m_selected.column + 3, layout.top);
	else if (m_selected.pile == CGame::Pile::Tableau)
		selected = layout.Slot(m_selected.column, layout.row + m_selected.card * layout.step);
	if (m_selected.pile != CGame::Pile::None && !m_dragging)
	{
		m_renderer.DrawOutline(selected, D2D1::ColorF(1.0f, 215.0f / 255, 0.0f), 2.0f);
	}
	if (m_animating)
	{
		const int frame = static_cast<int>((GetTickCount64() - m_animationStart) / 30);
		for (int i = 0; i < 16; ++i)
		{
			int x = (frame * (3 + i % 4) + i * (layout.width + 13)) %
				(client.Width() + layout.width) - layout.width;
			int y = (frame * (2 + i % 3) + i * (layout.height / 2 + 17)) %
				(client.Height() + layout.height) - layout.height;
			DrawCard(CRect(x, y, x + layout.width, y + layout.height),
				m_game.Foundation(i % 4)[(frame / 6 + i) % 13]);
		}
	}
	hr = m_renderer.EndDraw();
	if (hr == D2DERR_RECREATE_TARGET) Invalidate(FALSE);
	else if (FAILED(hr)) ReportRenderError(hr);
}

CGame::Location CMikasSolitaireView::HitTest(CPoint point, const Layout& layout) const
{
	using Pile = CGame::Pile;
	const CGame& m_game = GetDocument()->Game();
	if (layout.Slot(0, layout.top).PtInRect(point)) return { Pile::Stock, 0, 0 };
	if (layout.Slot(1, layout.top).PtInRect(point))
		return { Pile::Waste, 0, static_cast<int>(m_game.Waste().size()) - 1 };
	for (int i = 0; i < 4; ++i)
		if (layout.Slot(i + 3, layout.top).PtInRect(point))
			return { Pile::Foundation, i, static_cast<int>(m_game.Foundation(i).size()) - 1 };
	for (int i = 0; i < 7; ++i)
	{
		const auto& pile = m_game.Tableau(i);
		for (int j = static_cast<int>(pile.size()) - 1; j >= 0; --j)
			if (layout.Slot(i, layout.row + j * layout.step).PtInRect(point))
				return { Pile::Tableau, i, j };
		if (pile.empty() && layout.Slot(i, layout.row).PtInRect(point))
			return { Pile::Tableau, i, -1 };
	}
	return {};
}

void CMikasSolitaireView::OnLButtonDown(UINT flags, CPoint point)
{
	if (m_animating) return;
	using Pile = CGame::Pile;
	CGame& m_game = GetDocument()->Game();
	m_movedOnLastButtonDown = false;
	CGame::Location hit = HitTest(point, GetLayout());
	if (hit.pile == Pile::Stock)
	{
		m_selected = {};
		if (m_game.DrawStock()) AfterMove();
	}
	else
	{
		bool moved = m_selected.pile != Pile::None && m_game.Move(m_selected, hit);
		m_movedOnLastButtonDown = moved;
		m_selected = {};
		if (!moved && hit.card >= 0)
		{
			const CCard* card = nullptr;
			if (hit.pile == Pile::Waste) card = &m_game.Waste().back();
			else if (hit.pile == Pile::Foundation) card = &m_game.Foundation(hit.column).back();
			else if (hit.pile == Pile::Tableau) card = &m_game.Tableau(hit.column)[hit.card];
			if (card && card->IsFaceUp()) m_selected = hit;
		}
		if (moved) AfterMove();
		else if (m_selected.pile != Pile::None)
		{
			Layout layout = GetLayout();
			CRect rect;
			if (m_selected.pile == Pile::Waste) rect = layout.Slot(1, layout.top);
			else if (m_selected.pile == Pile::Foundation) rect = layout.Slot(m_selected.column + 3, layout.top);
			else rect = layout.Slot(m_selected.column, layout.row + m_selected.card * layout.step);
			m_dragStart = m_dragPoint = point;
			m_dragOffset = point - rect.TopLeft();
			m_trackingDrag = true;
			SetCapture();
			if (GetCapture() != this)
			{
				m_trackingDrag = false;
				AfxMessageBox(IDS_MOUSE_CAPTURE_FAILED, MB_ICONWARNING);
			}
		}
	}
	Invalidate(FALSE);
	CView::OnLButtonDown(flags, point);
}

void CMikasSolitaireView::OnMouseMove(UINT flags, CPoint point)
{
	if (m_trackingDrag)
	{
		if (!m_dragging && (abs(point.x - m_dragStart.x) >= GetSystemMetrics(SM_CXDRAG) ||
			abs(point.y - m_dragStart.y) >= GetSystemMetrics(SM_CYDRAG)))
			m_dragging = true;
		if (m_dragging)
		{
			m_dragPoint = point;
			::SetCursor(::LoadCursor(nullptr, IDC_HAND));
			Invalidate(FALSE);
		}
	}
	CView::OnMouseMove(flags, point);
}

void CMikasSolitaireView::OnLButtonUp(UINT flags, CPoint point)
{
	if (m_trackingDrag)
	{
		const bool dragged = m_dragging;
		const CGame::Location source = m_selected;
		CancelDrag();
		if (dragged)
		{
			m_selected = {};
			if (GetDocument()->Game().Move(source, HitTest(point, GetLayout())))
				AfterMove();
			Invalidate(FALSE);
		}
	}
	CView::OnLButtonUp(flags, point);
}

void CMikasSolitaireView::CancelDrag()
{
	m_trackingDrag = false;
	m_dragging = false;
	if (GetCapture() == this) ReleaseCapture();
}

void CMikasSolitaireView::OnCaptureChanged(CWnd* window)
{
	if (m_trackingDrag)
	{
		m_trackingDrag = false;
		m_dragging = false;
		m_selected = {};
		Invalidate(FALSE);
	}
	CView::OnCaptureChanged(window);
}

BOOL CMikasSolitaireView::OnSetCursor(CWnd* window, UINT hitTest, UINT message)
{
	if (m_dragging)
	{
		::SetCursor(::LoadCursor(nullptr, IDC_HAND));
		return TRUE;
	}
	if (hitTest == HTCLIENT && !m_animating)
	{
		CPoint point;
		GetCursorPos(&point);
		ScreenToClient(&point);
		const auto hit = HitTest(point, GetLayout());
		const CGame& game = GetDocument()->Game();
		const CCard* card = nullptr;
		if (hit.card >= 0)
		{
			if (hit.pile == CGame::Pile::Waste) card = &game.Waste().back();
			else if (hit.pile == CGame::Pile::Foundation) card = &game.Foundation(hit.column).back();
			else if (hit.pile == CGame::Pile::Tableau) card = &game.Tableau(hit.column)[hit.card];
		}
		if (card && card->IsFaceUp())
		{
			::SetCursor(::LoadCursor(nullptr, IDC_HAND));
			return TRUE;
		}
	}
	return CView::OnSetCursor(window, hitTest, message);
}

void CMikasSolitaireView::AfterMove()
{
	GetDocument()->SetModifiedFlag();
	Invalidate(FALSE);
	if (GetDocument()->Game().IsWon())
	{
		m_stuckNotified = false;
		m_animationStart = GetTickCount64();
		m_animating = true;
		if (!SetTimer(WinTimer, 30, nullptr))
		{
			m_animating = false;
			AfxMessageBox(IDS_ANIMATION_FAILED, MB_ICONWARNING);
			AfxMessageBox(IDS_GAME_WON);
		}
	}
	else CheckPosition();
}

void CMikasSolitaireView::CheckPosition()
{
	const CGame& game = GetDocument()->Game();
	if (game.IsWon()) return;
	if (game.HasForwardMoves())
	{
		m_stuckNotified = false;
		return;
	}
	if (m_stuckNotified) return;
	m_stuckNotified = true;
	int choice = AfxMessageBox(IDS_NO_FORWARD_MOVES, MB_YESNOCANCEL | MB_ICONQUESTION);
	if (choice == IDYES)
	{
		GetDocument()->Game().Restart();
		GetDocument()->SetModifiedFlag();
		m_selected = {};
		m_stuckNotified = false;
		Invalidate(FALSE);
	}
	else if (choice == IDNO)
		GetParentFrame()->SendMessage(WM_COMMAND, ID_FILE_SAVE);
}

LRESULT CMikasSolitaireView::OnCheckPosition(WPARAM, LPARAM)
{
	CheckPosition();
	return 0;
}

void CMikasSolitaireView::StopAnimation()
{
	if (m_animating && GetSafeHwnd()) KillTimer(WinTimer);
	m_animating = false;
}

void CMikasSolitaireView::OnTimer(UINT_PTR id)
{
	if (id == WinTimer && m_animating)
	{
		if (GetTickCount64() - m_animationStart >= 3000)
		{
			StopAnimation();
			Invalidate(FALSE);
			AfxMessageBox(IDS_GAME_WON);
		}
		else Invalidate(FALSE);
		return;
	}
	CView::OnTimer(id);
}

void CMikasSolitaireView::OnUndo()
{
	CancelDrag();
	StopAnimation();
	if (GetDocument()->Game().Undo())
	{
		m_selected = {};
		GetDocument()->SetModifiedFlag();
		Invalidate(FALSE);
		CheckPosition();
	}
}

void CMikasSolitaireView::OnUpdateUndo(CCmdUI* cmdUI)
{
	cmdUI->Enable(GetDocument()->Game().CanUndo());
}

void CMikasSolitaireView::OnRButtonUp(UINT, CPoint)
{
	CancelDrag();
	m_selected = {};
	Invalidate(FALSE);
}

void CMikasSolitaireView::OnLButtonDblClk(UINT flags, CPoint point)
{
	if (m_animating) return;
	CancelDrag();
	CGame& game = GetDocument()->Game();
	if (!m_movedOnLastButtonDown && game.MoveToFoundation(HitTest(point, GetLayout())))
	{
		m_selected = {};
		Invalidate(FALSE);
		AfterMove();
	}
	CView::OnLButtonDblClk(flags, point);
}

void CMikasSolitaireView::OnSize(UINT type, int cx, int cy)
{
	CView::OnSize(type, cx, cy);
	Invalidate(FALSE);
}

BOOL CMikasSolitaireView::OnEraseBkgnd(CDC*)
{
	return TRUE;
}

void CMikasSolitaireView::OnUpdate(CView*, LPARAM, CObject*)
{
	CancelDrag();
	StopAnimation();
	m_selected = {};
	m_stuckNotified = false;
	if (GetSafeHwnd())
	{
		Invalidate(FALSE);
		PostMessage(CheckPositionMessage);
	}
}

void CMikasSolitaireView::OnOpenGame()
{
	CString filter;
	filter.LoadString(IDS_GAME_FILTER);
	CFileDialog dialog(TRUE, L"mksol", nullptr, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY,
		filter, this);
	if (dialog.DoModal() != IDOK || !GetDocument()->ConfirmSaveChanges()) return;
	CString path = dialog.GetPathName();
	if (GetDocument()->OnOpenDocument(path))
		GetDocument()->SetPathName(path);
}

void CMikasSolitaireView::OnRestartGame()
{
	if (AfxMessageBox(IDS_RESTART_CONFIRM, MB_YESNO | MB_ICONQUESTION) != IDYES)
		return;
	GetDocument()->Game().Restart();
	GetDocument()->SetModifiedFlag();
	CancelDrag();
	StopAnimation();
	m_stuckNotified = false;
	m_selected = {};
	Invalidate(FALSE);
	CheckPosition();
}

void CMikasSolitaireView::OnCardBack(UINT id)
{
	m_cardBack = static_cast<int>(id - ID_CARD_BACK_1);
	if (!theApp.WriteProfileInt(L"Game", L"CardBack", m_cardBack))
		AfxMessageBox(IDS_CARD_BACK_SAVE_FAILED, MB_ICONWARNING);
	Invalidate(FALSE);
}

void CMikasSolitaireView::OnUpdateCardBack(CCmdUI* cmdUI)
{
	cmdUI->SetRadio(cmdUI->m_nID == ID_CARD_BACK_1 + m_cardBack);
}

void CMikasSolitaireView::OnUpdateUnavailable(CCmdUI* cmdUI)
{
	cmdUI->Enable(FALSE);
}

void CMikasSolitaireView::OnUpdateSave(CCmdUI* cmdUI)
{
	cmdUI->Enable(TRUE);
}

void CMikasSolitaireView::ReportRenderError(HRESULT error)
{
	if (m_renderErrorShown) return;
	m_renderErrorShown = true;
	CString message;
	message.Format(IDS_RENDER_FAILED, static_cast<unsigned>(error));
	AfxMessageBox(message, MB_ICONERROR);
}

#ifdef _DEBUG
void CMikasSolitaireView::AssertValid() const { CView::AssertValid(); }
void CMikasSolitaireView::Dump(CDumpContext& dc) const { CView::Dump(dc); }
CMikasSolitaireDoc* CMikasSolitaireView::GetDocument() const
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMikasSolitaireDoc)));
	return static_cast<CMikasSolitaireDoc*>(m_pDocument);
}
#endif
