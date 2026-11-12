#pragma once
#include "Card.h"
#include <array>
#include <d2d1.h>
#include <wincodec.h>
#include <wrl/client.h>

class CCardImageManager
{
public:
	bool LoadAllCards(CString& error);
	HRESULT CreateBitmaps(ID2D1RenderTarget* target, UINT width, UINT height);
	void ReleaseBitmaps();
	ID2D1Bitmap* Face(const CCard& card) const;
	ID2D1Bitmap* Back(int index) const { return m_backs[index].Get(); }

private:
	using Source = Microsoft::WRL::ComPtr<IWICBitmapSource>;
	using Bitmap = Microsoft::WRL::ComPtr<ID2D1Bitmap>;
	HRESULT LoadImage(const CString& path, Source& source);
	HRESULT ScaleImage(ID2D1RenderTarget* target, IWICBitmapSource* source,
		UINT width, UINT height, Bitmap& bitmap);
	Microsoft::WRL::ComPtr<IWICImagingFactory> m_factory;
	std::array<std::array<Source, 13>, 4> m_faceSources;
	std::array<Source, 6> m_backSources;
	std::array<std::array<Bitmap, 13>, 4> m_faces;
	std::array<Bitmap, 6> m_backs;
};
