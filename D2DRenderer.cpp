#include "pch.h"
#include "D2DRenderer.h"
#pragma comment(lib, "d2d1.lib")

bool CD2DRenderer::Initialize(CString& error)
{
	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, m_factory.GetAddressOf());
	if (FAILED(hr))
	{
		error.Format(L"Cannot initialize Direct2D (0x%08X).", static_cast<unsigned>(hr));
		return false;
	}
	return m_images.LoadAllCards(error);
}

HRESULT CD2DRenderer::Prepare(HWND window, UINT width, UINT height, UINT cardWidth, UINT cardHeight)
{
	HRESULT hr = S_OK;
	if (!m_target)
	{
		auto properties = D2D1::RenderTargetProperties();
		properties.dpiX = properties.dpiY = 96.0f;
		hr = m_factory->CreateHwndRenderTarget(properties,
			D2D1::HwndRenderTargetProperties(window, D2D1::SizeU(width, height)), m_target.GetAddressOf());
		if (SUCCEEDED(hr)) hr = m_target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), m_brush.GetAddressOf());
	}
	else
	{
		D2D1_SIZE_U size = m_target->GetPixelSize();
		if (size.width != width || size.height != height)
			hr = m_target->Resize(D2D1::SizeU(width, height));
	}
	if (SUCCEEDED(hr) && (m_cardWidth != cardWidth || m_cardHeight != cardHeight))
	{
		hr = m_images.CreateBitmaps(m_target.Get(), cardWidth, cardHeight);
		if (SUCCEEDED(hr)) { m_cardWidth = cardWidth; m_cardHeight = cardHeight; }
	}
	if (FAILED(hr)) DiscardTarget();
	return hr;
}

void CD2DRenderer::BeginDraw()
{
	m_target->BeginDraw();
	m_target->Clear(D2D1::ColorF(0.0f, 100.0f / 255.0f, 54.0f / 255.0f));
}

HRESULT CD2DRenderer::EndDraw()
{
	HRESULT hr = m_target->EndDraw();
	if (hr == D2DERR_RECREATE_TARGET) DiscardTarget();
	return hr;
}

void CD2DRenderer::DrawBitmap(ID2D1Bitmap* bitmap, const CRect& rect)
{
	m_target->DrawBitmap(bitmap, D2D1::RectF(static_cast<float>(rect.left), static_cast<float>(rect.top),
		static_cast<float>(rect.right), static_cast<float>(rect.bottom)), 1.0f,
		D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
}

void CD2DRenderer::DrawOutline(const CRect& rect, D2D1_COLOR_F color, float thickness)
{
	m_brush->SetColor(color);
	float inset = thickness / 2.0f;
	m_target->DrawRectangle(D2D1::RectF(rect.left + inset, rect.top + inset,
		rect.right - inset, rect.bottom - inset), m_brush.Get(), thickness);
}

void CD2DRenderer::DiscardTarget()
{
	m_images.ReleaseBitmaps();
	m_brush.Reset();
	m_target.Reset();
	m_cardWidth = m_cardHeight = 0;
}
