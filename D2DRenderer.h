#pragma once
#include "CardImageManager.h"

class CD2DRenderer
{
public:
	bool Initialize(CString& error);
	HRESULT Prepare(HWND window, UINT width, UINT height, UINT cardWidth, UINT cardHeight);
	void BeginDraw();
	HRESULT EndDraw();
	void DrawBitmap(ID2D1Bitmap* bitmap, const CRect& rect);
	void DrawOutline(const CRect& rect, D2D1_COLOR_F color, float thickness = 1.0f);
	ID2D1Bitmap* Face(const CCard& card) const { return m_images.Face(card); }
	ID2D1Bitmap* Back(int index) const { return m_images.Back(index); }
	void DiscardTarget();

private:
	CCardImageManager m_images;
	Microsoft::WRL::ComPtr<ID2D1Factory> m_factory;
	Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> m_target;
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_brush;
	UINT m_cardWidth = 0, m_cardHeight = 0;
};
