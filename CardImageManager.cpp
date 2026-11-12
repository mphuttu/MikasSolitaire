#include "pch.h"
#include "CardImageManager.h"
#pragma comment(lib, "windowscodecs.lib")

bool CCardImageManager::LoadAllCards(CString& error)
{
	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(m_factory.GetAddressOf()));
	if (FAILED(hr))
	{
		error.Format(L"Cannot initialize the image decoder (0x%08X).", static_cast<unsigned>(hr));
		return false;
	}
	wchar_t path[32768];
	DWORD length = GetModuleFileNameW(nullptr, path, _countof(path));
	if (length == 0 || length >= _countof(path))
	{
		error = L"Cannot determine the executable directory.";
		return false;
	}
	CString directory(path);
	int separator = directory.ReverseFind(L'\\');
	if (separator < 0)
	{
		error = L"Cannot determine the executable directory.";
		return false;
	}
	directory = directory.Left(separator);
	const wchar_t suits[] = { L'h', L'd', L'c', L's' };
	for (int suit = 0; suit < 4; ++suit)
	{
		for (int rank = 1; rank <= 13; ++rank)
		{
			CString file;
			file.Format(L"%s\\res\\front\\%c%02d.png", directory.GetString(), suits[suit], rank);
			hr = LoadImage(file, m_faceSources[suit][rank - 1]);
			if (FAILED(hr))
			{
				error.Format(L"Cannot load card image:\n%s\nError: 0x%08X",
					file.GetString(), static_cast<unsigned>(hr));
				return false;
			}
		}
	}
	for (int i = 0; i < 6; ++i)
	{
		CString file;
		file.Format(L"%s\\res\\back\\Card-Back-%02d.png", directory.GetString(), i + 1);
		hr = LoadImage(file, m_backSources[i]);
		if (FAILED(hr))
		{
			error.Format(L"Cannot load card image:\n%s\nError: 0x%08X",
				file.GetString(), static_cast<unsigned>(hr));
			return false;
		}
	}
	return true;
}

HRESULT CCardImageManager::LoadImage(const CString& path, Source& source)
{
	using Microsoft::WRL::ComPtr;
	ComPtr<IWICBitmapDecoder> decoder;
	ComPtr<IWICBitmapFrameDecode> frame;
	ComPtr<IWICFormatConverter> converter;
	ComPtr<IWICBitmap> cached;
	HRESULT hr = m_factory->CreateDecoderFromFilename(path, nullptr, GENERIC_READ,
		WICDecodeMetadataCacheOnLoad, decoder.GetAddressOf());
	if (SUCCEEDED(hr)) hr = decoder->GetFrame(0, frame.GetAddressOf());
	if (SUCCEEDED(hr)) hr = m_factory->CreateFormatConverter(converter.GetAddressOf());
	if (SUCCEEDED(hr)) hr = converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone, nullptr, 0, WICBitmapPaletteTypeCustom);
	if (SUCCEEDED(hr)) hr = m_factory->CreateBitmapFromSource(converter.Get(),
		WICBitmapCacheOnLoad, cached.GetAddressOf());
	if (SUCCEEDED(hr)) source = cached;
	return hr;
}

HRESULT CCardImageManager::ScaleImage(ID2D1RenderTarget* target, IWICBitmapSource* source,
	UINT width, UINT height, Bitmap& bitmap)
{
	Microsoft::WRL::ComPtr<IWICBitmapScaler> scaler;
	HRESULT hr = m_factory->CreateBitmapScaler(scaler.GetAddressOf());
	if (SUCCEEDED(hr)) hr = scaler->Initialize(source, width, height, WICBitmapInterpolationModeFant);
	if (SUCCEEDED(hr)) hr = target->CreateBitmapFromWicBitmap(scaler.Get(), nullptr, bitmap.GetAddressOf());
	return hr;
}

HRESULT CCardImageManager::CreateBitmaps(ID2D1RenderTarget* target, UINT width, UINT height)
{
	ReleaseBitmaps();
	for (int suit = 0; suit < 4; ++suit)
		for (int rank = 0; rank < 13; ++rank)
		{
			HRESULT hr = ScaleImage(target, m_faceSources[suit][rank].Get(), width, height, m_faces[suit][rank]);
			if (FAILED(hr)) { ReleaseBitmaps(); return hr; }
		}
	for (int i = 0; i < 6; ++i)
	{
		HRESULT hr = ScaleImage(target, m_backSources[i].Get(), width, height, m_backs[i]);
		if (FAILED(hr)) { ReleaseBitmaps(); return hr; }
	}
	return S_OK;
}

void CCardImageManager::ReleaseBitmaps()
{
	for (auto& suit : m_faces)
		for (auto& bitmap : suit) bitmap.Reset();
	for (auto& bitmap : m_backs) bitmap.Reset();
}

ID2D1Bitmap* CCardImageManager::Face(const CCard& card) const
{
	return m_faces[static_cast<int>(card.GetSuit())][card.GetRank() - 1].Get();
}
