#include "pch.h"
#include "framework.h"
#include "MikasSolitaire.h"
#include "MikasSolitaireDoc.h"
#include "MikasSolitaireView.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMikasSolitaireView, CView)

BEGIN_MESSAGE_MAP(CMikasSolitaireView, CView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_FILE_OPEN, &CMikasSolitaireView::OnOpenGame)
	ON_COMMAND(ID_GAME_RESTART, &CMikasSolitaireView::OnRestartGame)
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
	if (!m_game.Waste().empty()) DrawCard(waste, m_game.Waste().back());
	for (int i = 0; i < 4; ++i)
	{
		CRect rect = layout.Slot(i + 3, layout.top);
		slot(rect);
		if (!m_game.Foundation(i).empty()) DrawCard(rect, m_game.Foundation(i).back());
	}
	for (int i = 0; i < 7; ++i)
	{
		const auto& pile = m_game.Tableau(i);
		CRect rect = layout.Slot(i, layout.row);
		slot(rect);
		for (size_t j = 0; j < pile.size(); ++j)
		{
			rect = layout.Slot(i, layout.row + static_cast<int>(j) * layout.step);
			DrawCard(rect, pile[j]);
		}
	}
	CRect selected;
	if (m_selected.pile == CGame::Pile::Waste) selected = waste;
	else if (m_selected.pile == CGame::Pile::Foundation)
		selected = layout.Slot(m_selected.column + 3, layout.top);
	else if (m_selected.pile == CGame::Pile::Tableau)
		selected = layout.Slot(m_selected.column, layout.row + m_selected.card * layout.step);
	if (m_selected.pile != CGame::Pile::None)
	{
		m_renderer.DrawOutline(selected, D2D1::ColorF(1.0f, 215.0f / 255, 0.0f), 2.0f);
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
	using Pile = CGame::Pile;
	CGame& m_game = GetDocument()->Game();
	m_movedOnLastButtonDown = false;
	CGame::Location hit = HitTest(point, GetLayout());
	if (hit.pile == Pile::Stock)
	{
		m_selected = {};
		if (m_game.DrawStock()) GetDocument()->SetModifiedFlag();
	}
	else
	{
		bool moved = m_selected.pile != Pile::None && m_game.Move(m_selected, hit);
		m_movedOnLastButtonDown = moved;
		if (moved) GetDocument()->SetModifiedFlag();
		m_selected = {};
		if (!moved && hit.card >= 0)
		{
			const CCard* card = nullptr;
			if (hit.pile == Pile::Waste) card = &m_game.Waste().back();
			else if (hit.pile == Pile::Foundation) card = &m_game.Foundation(hit.column).back();
			else if (hit.pile == Pile::Tableau) card = &m_game.Tableau(hit.column)[hit.card];
			if (card && card->IsFaceUp()) m_selected = hit;
		}
		if (moved && m_game.IsWon()) AfxMessageBox(L"You won! Congratulations!");
	}
	Invalidate(FALSE);
	CView::OnLButtonDown(flags, point);
}

void CMikasSolitaireView::OnRButtonUp(UINT, CPoint)
{
	m_selected = {};
	Invalidate(FALSE);
}

void CMikasSolitaireView::OnLButtonDblClk(UINT flags, CPoint point)
{
	CGame& game = GetDocument()->Game();
	if (!m_movedOnLastButtonDown && game.MoveToFoundation(HitTest(point, GetLayout())))
	{
		GetDocument()->SetModifiedFlag();
		m_selected = {};
		Invalidate(FALSE);
		if (game.IsWon()) AfxMessageBox(L"You won! Congratulations!");
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
	m_selected = {};
	if (GetSafeHwnd()) Invalidate(FALSE);
}

void CMikasSolitaireView::OnOpenGame()
{
	CFileDialog dialog(TRUE, L"mksol", nullptr, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY,
		L"Solitaire games (*.mksol)|*.mksol|All files (*.*)|*.*||", this);
	if (dialog.DoModal() != IDOK || !GetDocument()->ConfirmSaveChanges()) return;
	CString path = dialog.GetPathName();
	if (GetDocument()->OnOpenDocument(path))
		GetDocument()->SetPathName(path);
}

void CMikasSolitaireView::OnRestartGame()
{
	if (AfxMessageBox(L"Restart this deal from the beginning?", MB_YESNO | MB_ICONQUESTION) != IDYES)
		return;
	GetDocument()->Game().Restart();
	GetDocument()->SetModifiedFlag();
	m_selected = {};
	Invalidate(FALSE);
}

void CMikasSolitaireView::OnCardBack(UINT id)
{
	m_cardBack = static_cast<int>(id - ID_CARD_BACK_1);
	if (!theApp.WriteProfileInt(L"Game", L"CardBack", m_cardBack))
		AfxMessageBox(L"The card back could not be saved.", MB_ICONWARNING);
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
	message.Format(L"Cannot draw the game (Direct2D error 0x%08X).", static_cast<unsigned>(error));
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
