#pragma once
#include "GraphicsDef.h"
#include <d2d1_3.h>
#include <dwrite_3.h>

class Graphics2D
{
public:

	gfx_err Initialise(ComPtr<IDXGIDevice> dxgiDevice);

	gfx_err UpdateTarget(ComPtr<IDXGISurface> newTarget);
	gfx_err DestroyTarget();

	gfx_err Render();
private:



	ComPtr<IDXGISurface> d3dTarget = nullptr; //shared with d3d, do NOT edit here!!!
	ComPtr<ID2D1Factory1> d2dFactory = nullptr;
	ComPtr<ID2D1Device> d2dDevice = nullptr;
	ComPtr<ID2D1DeviceContext> d2dDeviceContext = nullptr;
	ComPtr<ID2D1Bitmap1> d2dTargetBitmap = nullptr;

	ComPtr<ID2D1RenderTarget> d2dRenderTarget = nullptr;
	ComPtr<IDWriteFactory1> d2dDwriteFactory = nullptr;
	ComPtr<IDWriteTextFormat> d2dTextFormat = nullptr;

	unsigned int width;
	unsigned int height;
};

