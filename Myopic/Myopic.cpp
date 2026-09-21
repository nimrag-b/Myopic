#include <iostream>

#define WIN32_LEAN_AND_MEAN
//#include <d3d11.h>
#include <d3d11_4.h>
#include <wrl/client.h>

#include <SDL3/SDL.h>

#pragma comment(lib, "d3d11.lib")

template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;


inline void ThrowOnFail(HRESULT hr) {
	if (FAILED(hr)) {
		throw std::exception();
	}
}

void ErrorWithMessage(const wchar_t* msg, const wchar_t* caption = L"Error") {
	MessageBox(NULL, msg, caption, MB_OK | MB_ICONERROR);
	throw std::exception();
}

#define WINDOWWIDTH 800
#define WINDOWHEIGHT 600

int main() {


	//init window with SDL
	SDL_Window* window = SDL_CreateWindow("sdlwindow", WINDOWWIDTH, WINDOWHEIGHT, SDL_WINDOW_RESIZABLE);

	if (window == NULL) {
		ErrorWithMessage(L"Failed to create window");
	}

	//obtain HWND from sdl window
	SDL_SetWindowSurfaceVSync(window, SDL_WINDOW_SURFACE_VSYNC_DISABLED);
	HWND hwnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
	if (hwnd == NULL) {
		ErrorWithMessage(L"SDL Error");
	}


	ComPtr<IDXGISwapChain4> d3dSwapChain;
	ComPtr<ID3D11Device5> d3dDevice;
	ComPtr<ID3D11DeviceContext4> d3dDeviceContext;
	
	

	HRESULT hr;

	UINT d3dFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	d3dFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	const D3D_FEATURE_LEVEL d3dFeatureLevels[]{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};

	D3D_FEATURE_LEVEL* supportedFeatureLevel = NULL;

	ID3D11Device* tmpD3dDevice;
	ID3D11DeviceContext* tmpD3dDeviceContext;

	//create D3D device

	hr = D3D11CreateDevice(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		d3dFlags,
		d3dFeatureLevels,
		ARRAYSIZE(d3dFeatureLevels),
		D3D11_SDK_VERSION,
		&tmpD3dDevice,
		supportedFeatureLevel,
		&tmpD3dDeviceContext
	);

	//convert to later interfaces
	ThrowOnFail(tmpD3dDevice->QueryInterface(d3dDevice.GetAddressOf()));
	ThrowOnFail(tmpD3dDeviceContext->QueryInterface(d3dDeviceContext.GetAddressOf()));

	if (FAILED(hr)) {
		ErrorWithMessage(L"Failed to create D3D11 Device", L"D3D Error");
	}

	// create swapchain

	DXGI_SWAP_CHAIN_DESC1 d3dSwapChainDesc = { 0 };
	d3dSwapChainDesc.Width = WINDOWWIDTH;
	d3dSwapChainDesc.Height = WINDOWHEIGHT;
	d3dSwapChainDesc.Stereo = false;
	d3dSwapChainDesc.Format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
	d3dSwapChainDesc.SampleDesc.Count = 1; //dont multisample
	d3dSwapChainDesc.SampleDesc.Quality = 0;
	d3dSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	d3dSwapChainDesc.BufferCount = 2; //use double buffering
	d3dSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;
	d3dSwapChainDesc.Scaling = DXGI_SCALING::DXGI_SCALING_STRETCH;
	d3dSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC d3dSwapChainFullscreenDesc = {0};
	d3dSwapChainFullscreenDesc.Windowed = true;


	IDXGIDevice2* pDXGIDevice;
	hr = d3dDevice->QueryInterface(__uuidof(IDXGIDevice2), (void**)&pDXGIDevice);

	IDXGIAdapter* pDXGIAdapter;
	hr = pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&pDXGIAdapter);

	IDXGIFactory4* pIDXGIFactory;
	pDXGIAdapter->GetParent(__uuidof(IDXGIFactory4), (void**)&pIDXGIFactory);


	IDXGISwapChain1* tmpD3dSwapChain;

	hr = pIDXGIFactory->CreateSwapChainForHwnd(
		d3dDevice.Get(),
		hwnd,
		&d3dSwapChainDesc,
		&d3dSwapChainFullscreenDesc,
		NULL,
		&tmpD3dSwapChain);

	if (FAILED(hr)) {
		ErrorWithMessage(L"Failed to create Swapchain", L"D3D Error");
	}

	ThrowOnFail(tmpD3dSwapChain->QueryInterface(d3dSwapChain.GetAddressOf()));



	return 0;
}