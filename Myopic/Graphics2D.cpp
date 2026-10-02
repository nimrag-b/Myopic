#include "Graphics2D.h"
#include "GameError.h"

#include <string>


#pragma comment (lib, "D2d1.lib")
#pragma comment (lib, "Dwrite.lib")

gfx_err Graphics2D::Initialise(ComPtr<IDXGIDevice> dxgiDevice) {

#ifdef MYOPIC_STRIP_D2D
	return gfx_err::OK;
#endif // MYOPIC_STRIP_D2D


	HRESULT hr;
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, d2dFactory.ReleaseAndGetAddressOf());

	err::ThrowOnFail(
		d2dFactory->CreateDevice(dxgiDevice.Get(), d2dDevice.ReleaseAndGetAddressOf())
	);
	err::ThrowOnFail(
		d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, d2dDeviceContext.ReleaseAndGetAddressOf())
	);


	static const WCHAR msc_fontName[] = L"Verdana";
	static const FLOAT msc_fontSize = 50;

	err::ThrowOnFail(
		DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(d2dDwriteFactory),
			reinterpret_cast<IUnknown**>(d2dDwriteFactory.ReleaseAndGetAddressOf())
	));

	err::ThrowOnFail(
		d2dDwriteFactory->CreateTextFormat(
			msc_fontName,
			NULL,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			msc_fontSize,
			L"", //locale
			d2dTextFormat.ReleaseAndGetAddressOf()
		)
	);

	d2dTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);

	d2dTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

	return gfx_err::OK;
}

gfx_err Graphics2D::UpdateTarget(ComPtr<IDXGISurface> newTarget)
{
#ifdef MYOPIC_STRIP_D2D
	return gfx_err::OK;
#endif // MYOPIC_STRIP_D2D
	HRESULT hr;
	d3dTarget = newTarget;

	auto v = newTarget.Reset();

	D2D1_BITMAP_PROPERTIES1 bitmapProperties =
		D2D1::BitmapProperties1(
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
			96,
			96
		);


	hr = d2dDeviceContext->CreateBitmapFromDxgiSurface(d3dTarget.Get(), &bitmapProperties, d2dTargetBitmap.ReleaseAndGetAddressOf());

	d2dDeviceContext->SetTarget(d2dTargetBitmap.Get());



	auto s = d2dTargetBitmap->GetPixelSize();
	width = s.width;
	height = s.height;

	return gfx_err::OK;
}

gfx_err Graphics2D::DestroyTarget() {
#ifdef MYOPIC_STRIP_D2D
	return gfx_err::OK;
#endif // MYOPIC_STRIP_D2D
	d2dDeviceContext->SetTarget(nullptr);

	auto v = d2dTargetBitmap.Reset();
	auto v1 = d3dTarget.Reset();



	return gfx_err::OK;
}

int avg_fps();

gfx_err Graphics2D::Render() {
#ifdef MYOPIC_STRIP_D2D
	return gfx_err::OK;
#endif // MYOPIC_STRIP_D2D
	HRESULT hr;
	ComPtr<ID2D1SolidColorBrush> solidBrush;
	hr = d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &solidBrush);

	d2dDeviceContext->BeginDraw();

	//d2dDeviceContext->FillRectangle(D2D1::RectF(100, 100, 200, 200), solidBrush.Get());
	int fps = avg_fps();

	const std::wstring sc_helloWorld = std::to_wstring(fps) + L"ms";

	d2dDeviceContext->DrawTextW(
		sc_helloWorld.c_str(),
		sc_helloWorld.length(),
		d2dTextFormat.Get(),
		D2D1::RectF(0, 0, width, height),
		solidBrush.Get()
	);

	//FIXME this is incredibly slow for some reason, so fix in future!
	hr = d2dDeviceContext->EndDraw();

	return gfx_err::OK;
}