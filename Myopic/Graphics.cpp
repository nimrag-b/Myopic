#include "Graphics.h"
#include "GameError.h"
#include <iostream>
#include <comdef.h>
#include <d3dcompiler.h>

#include <SDL3/SDL.h>




#if _DEBUG
#include <dxgidebug.h>
#endif

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")

constexpr uint32_t defaultWindowWidth = 800;
constexpr uint32_t defaultWindowHeight = 600;

#pragma region Opaque Pointer Implementations
struct Shader::ShaderImpl {

	ShaderImpl(ComPtr<ID3D11PixelShader> pixelShader, ComPtr<ID3D11VertexShader> vertexShader, const VertexLayout& layout) : d3dPixelShader(pixelShader), d3dVertexShader(vertexShader), layout(layout) {

	}

	const VertexLayout& Layout() const {
		return layout;
	}

	void Bind(ID3D11DeviceContext4* deviceContext) const {
		deviceContext->VSSetShader(d3dVertexShader.Get(), nullptr, 0);
		deviceContext->PSSetShader(d3dPixelShader.Get(), nullptr, 0);
	}
	void Reset() {
		d3dPixelShader.Reset();
		d3dVertexShader.Reset();
	}

	ComPtr<ID3D11PixelShader> d3dPixelShader;
	ComPtr<ID3D11VertexShader> d3dVertexShader;
	const VertexLayout& layout;
};

struct Mesh::MeshImpl{

	void Render(ID3D11DeviceContext4* deviceContext, int index)
	{
		deviceContext->IASetIndexBuffer(subMeshes[index].indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		deviceContext->DrawIndexed(static_cast<UINT>(subMeshes[index].indexCount), 0, 0);

	}
	void Reset() {
		d3dVertexBuffer.Reset();

		for (size_t i = 0; i < subMeshCount; i++)
		{
			subMeshes[i].indexBuffer.Reset();
		}
	}
	//vertex data
	ComPtr<ID3D11Buffer> d3dVertexBuffer = nullptr;

	size_t stride = 0; //stride
	size_t vertexCount = 0;


	struct index_group {
		ComPtr<ID3D11Buffer> indexBuffer = nullptr;
		size_t indexCount = 0;
	};
	std::unique_ptr<index_group[]> subMeshes = nullptr;
	size_t subMeshCount = 0;
};


struct Texture::TextureImpl {
	ComPtr<ID3D11ShaderResourceView> srv;
};


Shader::Shader(ShaderImpl* impl)
{
	this->impl = std::unique_ptr<ShaderImpl>(impl);
}

Shader::~Shader() = default;

Mesh::Mesh(MeshImpl* impl)
{
	this->impl = std::unique_ptr<MeshImpl>(impl);
}

Mesh::~Mesh() = default;


Texture::Texture(TextureImpl* impl)
{
	this->impl = std::unique_ptr<TextureImpl>(impl);
}

Texture::~Texture() = default;

#pragma endregion

gfx_err Graphics::Initialise() {
	//init window with SDL
	sdlWindow = SDL_CreateWindow("sdlwindow", defaultWindowWidth, defaultWindowHeight, SDL_WINDOW_RESIZABLE);

	windowWidth = defaultWindowWidth;
	windowHeight = defaultWindowHeight;

	if (sdlWindow == NULL) {
		err::ErrorWithMessage(L"Failed to create window");
		return gfx_err::CONFIG_ERROR;
	}

	//obtain HWND from sdl window
	SDL_SetWindowSurfaceVSync(sdlWindow, SDL_WINDOW_SURFACE_VSYNC_DISABLED);
	hwnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(sdlWindow), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
	if (hwnd == NULL) {
		err::ErrorWithMessage(L"SDL Error");
		return gfx_err::CONFIG_ERROR;
	}

	UINT dpi = GetDpiForWindow(hwnd);

	uint32_t dpiWidth = dpi * defaultWindowWidth / USER_DEFAULT_SCREEN_DPI;
	uint32_t dpiHeight = dpi * defaultWindowHeight / USER_DEFAULT_SCREEN_DPI;

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

	ComPtr<ID3D11Device> tmpD3dDevice;
	ComPtr<ID3D11DeviceContext> tmpD3dDeviceContext;

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
	err::ThrowOnFail(tmpD3dDevice->QueryInterface(d3dDevice.GetAddressOf()));
	err::ThrowOnFail(tmpD3dDeviceContext->QueryInterface(d3dDeviceContext.GetAddressOf()));

	if (FAILED(hr)) {
		err::ErrorWithMessage(L"Failed to create D3D11 Device", L"D3D Error");
		return gfx_err::CONFIG_ERROR;
	}

	// create swapchain

	DXGI_SWAP_CHAIN_DESC1 d3dSwapChainDesc = { 0 };
	d3dSwapChainDesc.Width = windowWidth;
	d3dSwapChainDesc.Height = windowHeight;
	d3dSwapChainDesc.Stereo = false;
	d3dSwapChainDesc.Format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
	d3dSwapChainDesc.SampleDesc.Count = 1; //dont multisample
	d3dSwapChainDesc.SampleDesc.Quality = 0;
	d3dSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	d3dSwapChainDesc.BufferCount = 2; //use double buffering
	d3dSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	d3dSwapChainDesc.Scaling = DXGI_SCALING::DXGI_SCALING_NONE;
	d3dSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC d3dSwapChainFullscreenDesc = { 0 };
	d3dSwapChainFullscreenDesc.Windowed = true;


	ComPtr<IDXGIDevice2> pDXGIDevice;
	hr = d3dDevice->QueryInterface(__uuidof(IDXGIDevice2), (void**)&pDXGIDevice);

	ComPtr < IDXGIAdapter> pDXGIAdapter;
	hr = pDXGIDevice->GetParent(__uuidof(IDXGIAdapter2), (void**)&pDXGIAdapter);


	ComPtr < IDXGIFactory4> pIDXGIFactory;
	pDXGIAdapter->GetParent(__uuidof(IDXGIFactory4), (void**)&pIDXGIFactory);


	ComPtr<IDXGISwapChain1> tmpD3dSwapChain;
	hr = pIDXGIFactory->CreateSwapChainForHwnd(
		d3dDevice.Get(),
		hwnd,
		&d3dSwapChainDesc,
		&d3dSwapChainFullscreenDesc,
		NULL,
		&tmpD3dSwapChain);

	err::ThrowOnFail(
		pDXGIDevice->SetMaximumFrameLatency(1)
	);

	if (FAILED(hr)) {
		err::ErrorWithMessage(L"Failed to create Swapchain", L"D3D Error");
		return gfx_err::CONFIG_ERROR;
	}

	err::ThrowOnFail(tmpD3dSwapChain->QueryInterface(d3dSwapChain.GetAddressOf()));

	CD3D11_SAMPLER_DESC samplerDesc{ CD3D11_DEFAULT() };
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;


	err::ThrowOnFail(
		d3dDevice->CreateSamplerState(
			&samplerDesc,
			&d3dSamplerState));



	D3D11_BLEND_DESC blendStateDesc;
	ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));
	blendStateDesc.AlphaToCoverageEnable = FALSE;
	blendStateDesc.IndependentBlendEnable = FALSE;
	blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	err::ThrowOnFail(
		d3dDevice->CreateBlendState(&blendStateDesc, d3dBlendStateAlpha.GetAddressOf())
	);


	//init d2d before window resources so we can pass the created idxgisurface
	ComPtr<IDXGIDevice> dxgiDevice;
	d3dDevice->QueryInterface(dxgiDevice.GetAddressOf());
	gfx2d.Initialise(dxgiDevice);


	if (!DXUpdateWindowResources()) {
		return gfx_err::BACKEND_ERROR;
	}




	return gfx_err::OK;
}

gfx_err Graphics::OnResize()
{
	int w, h;
	SDL_GetWindowSizeInPixels(sdlWindow, &w, &h);
	windowWidth = static_cast<uint32_t>(w);
	windowHeight = static_cast<uint32_t>(h);

	std::cout << "Resized to " << windowWidth << ", " << windowHeight << std::endl;

	DXUpdateWindowResources();

	return gfx_err::OK;
}

gfx_err Graphics::Destroy() {

	//ComPtrs get destructed automatically on exit but its nice to have explicitness
	d3dSwapChain.Reset();

	d3dDeviceContext.Reset();

	d3dDevice.Reset();

	d3dRenderTarget.Reset();
	d3dRenderTargetView.Reset();

	d3dDepthStencil.Reset();

	d3dDepthStencilView.Reset();


	SDL_DestroyWindow(sdlWindow);



	return gfx_err::OK;
}

#pragma region Rendering

gfx_err Graphics::BeginRender()
{
	//prepare frame for render


	ID3D11RenderTargetView* const targets[1] = { d3dRenderTargetView.Get() };
	d3dDeviceContext->OMSetRenderTargets(1, targets, d3dDepthStencilView.Get());
	d3dDeviceContext->ClearDepthStencilView(d3dDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	d3dDeviceContext->PSSetSamplers(0, 1, d3dSamplerState.GetAddressOf());


	const float ClearColor[4] = { 0.5f, 0.5f, 0.8f, 1.0f };
	d3dDeviceContext->ClearRenderTargetView(d3dRenderTargetView.Get(), ClearColor);



	return gfx_err::OK;
}

gfx_err Graphics::EndRender() {

	gfx2d.Render();
	d3dSwapChain->Present(0, 0);

	return gfx_err::OK;
}



#pragma endregion

gfx_err Graphics::SetTexture(Texture* texture, int slot)
{
	d3dDeviceContext->PSSetShaderResources(slot, 1, texture->inner()->srv.GetAddressOf());
	return gfx_err();
}


void Graphics::EnableAlphaBlend()
{
	d3dDeviceContext->OMSetBlendState(d3dBlendStateAlpha.Get(), NULL, 0xFFFFFF);
	ID3D11RenderTargetView* const targets[1] = { d3dRenderTargetView.Get() };
	d3dDeviceContext->OMSetRenderTargets(1, targets, nullptr);
}
void Graphics::DisableAlphaBlend()
{
	d3dDeviceContext->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);
	ID3D11RenderTargetView* const targets[1] = { d3dRenderTargetView.Get() };
	d3dDeviceContext->OMSetRenderTargets(1, targets, d3dDepthStencilView.Get());
}

void Graphics::DXBindD2dTexture()
{
	//d3dDeviceContext->PSSetShaderResources(0, 1, d2dShaderResourceView.GetAddressOf());
}


gfx_err Graphics::SetShader(Shader* shader) {
	if (shader == nullptr) return gfx_err::NULL_ERROR;
	Shader::ShaderImpl* impl = shader->inner();
	d3dDeviceContext->VSSetShader(impl->d3dVertexShader.Get(), nullptr, 0);
	d3dDeviceContext->PSSetShader(impl->d3dPixelShader.Get(), nullptr, 0);

	if (currentVertexLayout == nullptr || (currentVertexLayout->layout() != impl->Layout().layout())) {
		currentVertexLayout = &impl->Layout();
		d3dDeviceContext->IASetInputLayout(currentVertexLayout->layout());
	}
	return gfx_err::OK;
}

gfx_err Graphics::RenderMesh(Mesh* mesh, int subMesh) {
	if (mesh == nullptr) return gfx_err::NULL_ERROR;
	Mesh::MeshImpl* impl = mesh->inner();
	UINT stride = static_cast<UINT>(impl->stride);
	UINT offset = 0;
	//bind mesh
	d3dDeviceContext->IASetVertexBuffers(0, 1, impl->d3dVertexBuffer.GetAddressOf(), &stride, &offset);

	//bind vertices
	d3dDeviceContext->IASetIndexBuffer(impl->subMeshes[subMesh].indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	d3dDeviceContext->DrawIndexed(static_cast<UINT>(impl->subMeshes[subMesh].indexCount), 0, 0);

	return gfx_err::OK;
}


std::unique_ptr<Shader> Graphics::LoadShader(const std::string& vertex, const std::string& fragment, VertexLayout* layout) {


	if (layout == nullptr) {
		layout = &d3dGvertexLayout;
	}

	const std::wstring wvert = std::wstring(vertex.begin(), vertex.end());
	ComPtr<ID3D11VertexShader> vs = DXCreateVertexShader(wvert, layout);

	const std::wstring wpix = std::wstring(fragment.begin(), fragment.end());
	ComPtr<ID3D11PixelShader> ps = DXCreatePixelShader(wpix);

	return DXCreateShader(vs, ps, *layout);
}

std::unique_ptr<Shader> Graphics::DXCreateShader(ComPtr<ID3D11VertexShader> vertex, ComPtr<ID3D11PixelShader> pixel, const VertexLayout& layout)
{
	Shader::ShaderImpl* shader = new Shader::ShaderImpl(pixel, vertex, layout);
	return std::make_unique<Shader>(shader);
}

bool Graphics::DXCompileShader(const std::wstring& fileName, const std::string& entryPoint, const std::string& profile, ComPtr<ID3DBlob>& shaderBlob)
{
	HRESULT hr;
	//precompiled .cso file
	if (fileName.substr(fileName.length() - 4) == L".cso") {
		hr = D3DReadFileToBlob(fileName.c_str(), &shaderBlob);
		if (FAILED(hr)) {
			std::cout << "D3D11: Failed to load precompiled shader from file\n";
			return false;
		}

		return true;
	}

	constexpr UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;

	ComPtr<ID3DBlob> errorBlob = nullptr;
	if (FAILED(D3DCompileFromFile(
		fileName.data(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entryPoint.data(),
		profile.data(),
		compileFlags,
		0,
		&shaderBlob,
		&errorBlob)))
	{
		std::cout << "D3D11: Failed to read shader from file\n";
		if (errorBlob != nullptr)
		{
			std::cout << "D3D11: With message: " <<
				static_cast<const char*>(errorBlob->GetBufferPointer()) << "\n";
		}

		return false;
	}

	return true;
}

ComPtr<ID3D11VertexShader> Graphics::DXCreateGVertexVertexShader(const std::wstring& fileName)
{
	return DXCreateVertexShader(fileName, &d3dGvertexLayout);
}

ComPtr<ID3D11VertexShader> Graphics::DXCreateVertexShader(const std::wstring& fileName, VertexLayout* layout)
{
	ComPtr<ID3DBlob> vertexShaderBlob = nullptr;
	if (!DXCompileShader(fileName, "main", "vs_5_0", vertexShaderBlob))
	{
		return nullptr;
	}
	ComPtr<ID3D11VertexShader> vertexShader;
	if (FAILED(d3dDevice->CreateVertexShader(
		vertexShaderBlob->GetBufferPointer(),
		vertexShaderBlob->GetBufferSize(),
		nullptr,
		&vertexShader)))
	{
		std::cout << "D3D11: Failed to compile vertex shader\n";
		return nullptr;
	}

	if (layout != nullptr) {
		if (!layout->has_layout()) {
			if (FAILED(d3dDevice->CreateInputLayout(
				layout->desc(),
				(UINT)layout->size(),
				vertexShaderBlob->GetBufferPointer(),
				vertexShaderBlob->GetBufferSize(),
				layout->layout_address())))
			{
				std::cout << "D3D11: Failed to create vertex input layout\n";
				return nullptr;
			}
		}

	}





	return vertexShader;
}

ComPtr<ID3D11PixelShader> Graphics::DXCreatePixelShader(const std::wstring& fileName)
{
	ComPtr<ID3DBlob> pixelShaderBlob = nullptr;
	if (!DXCompileShader(fileName, "main", "ps_5_0", pixelShaderBlob))
	{
		return nullptr;
	}

	ComPtr<ID3D11PixelShader> pixelShader;
	if (FAILED(d3dDevice->CreatePixelShader(
		pixelShaderBlob->GetBufferPointer(),
		pixelShaderBlob->GetBufferSize(),
		nullptr,
		&pixelShader)))
	{
		std::cout << "D3D11: Failed to compile pixel shader\n";
		return nullptr;
	}

	return pixelShader;
}

std::unique_ptr<Mesh> Graphics::CreateMeshFlat(void* vertices, size_t count, size_t stride, int subMeshCount) {
	Mesh::MeshImpl* mesh = new Mesh::MeshImpl;
	mesh->vertexCount = count;
	mesh->subMeshCount = subMeshCount;
	mesh->subMeshes = std::make_unique<Mesh::MeshImpl::index_group[]>(subMeshCount);
	mesh->stride = stride;

	D3D11_BUFFER_DESC bd = { 0 };
	D3D11_SUBRESOURCE_DATA initData = { 0 };

	//create vertex buffer
	bd.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE; //regular usage, no streaming or weirdness
	bd.ByteWidth = static_cast<UINT>(stride * mesh->vertexCount);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER; //bind vertexes to vertex buffer
	bd.CPUAccessFlags = 0;
	initData.pSysMem = vertices; //attach data

	err::ThrowOnFail(
		d3dDevice->CreateBuffer(&bd, &initData, &mesh->d3dVertexBuffer)
	);

	return std::make_unique<Mesh>(mesh);
}

void Graphics::SetSubMesh(const std::unique_ptr<Mesh>& mesh, const int subMesh, std::vector<uint32_t>& indices) {

	if (subMesh < 0 || subMesh >= mesh->inner()->subMeshCount) {
		throw std::exception();
	}

	Mesh::MeshImpl::index_group& group = mesh->inner()->subMeshes[subMesh];

	group.indexCount = indices.size();

	D3D11_BUFFER_DESC bd = { 0 };
	D3D11_SUBRESOURCE_DATA initData = { 0 };
	//create index buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * indices.size());
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	initData.pSysMem = indices.data();

	err::ThrowOnFail(
		d3dDevice->CreateBuffer(&bd, &initData, group.indexBuffer.ReleaseAndGetAddressOf())
	);
}

#pragma region DX internal functions


void Graphics::DXReportLiveObjects() {

	//debug layer is disabled in release mode
#if (_DEBUG)

	IDXGIDebug1* dbglayer;
	DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dbglayer));

	HRESULT hr = dbglayer->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
	dbglayer->Release();
#endif
}


bool Graphics::DXCreateRenderTarget() {
	if (FAILED(d3dSwapChain->GetBuffer(
		0,
		IID_PPV_ARGS(d3dRenderTarget.ReleaseAndGetAddressOf())))
	)
	{
		std::cout << "D3D11: Failed to get Back Buffer from the SwapChain\n";
		return false;
	}

	//release old RTV and create new
	d3dRenderTargetView.Reset();
	ComPtr<ID3D11RenderTargetView> tmpRTV;

	if (FAILED(d3dDevice->CreateRenderTargetView(
		d3dRenderTarget.Get(),
		nullptr,
		tmpRTV.GetAddressOf()))
	)
	{
		std::cout << "D3D11: Failed to create RTV from Back Buffer\n";
		return false;
	}

	//convert to ID3D11RenderTargetView1
	if (FAILED(tmpRTV.As(&d3dRenderTargetView))) {
		std::cout << "D3D11: Failed to convert RTV\n";
		return false;
	}

	ComPtr<IDXGISurface> d2dSurface;
	d3dRenderTarget->QueryInterface(d2dSurface.ReleaseAndGetAddressOf());

	gfx2d.UpdateTarget(d2dSurface);

	return true;
}

void Graphics::DXDestroyRenderTarget()
{
	d3dRenderTarget.Reset();
	d3dRenderTargetView.Reset();

	gfx2d.DestroyTarget();
}

bool Graphics::DXUpdateWindowResources()
{
	d3dDeviceContext->OMSetRenderTargets(0, 0, 0);

	DXDestroyRenderTarget();

	//resize render buffers to new window size
	HRESULT hr = d3dSwapChain->ResizeBuffers(
		2,          // double buffered
		static_cast<UINT>(windowWidth),
		static_cast<UINT>(windowHeight),
		DXGI_FORMAT_B8G8R8A8_UNORM,    //format
		0
	);

	if (FAILED(hr)) {
		//_com_error err(hr);
		//LPCTSTR errMsg = err.ErrorMessage();
		return false;
	}

	//recreate render targets based on new swapchain
	if (!DXCreateRenderTarget()) {
		return false;
	}



	// Create a depth stencil view for use with 3D rendering if needed.
	CD3D11_TEXTURE2D_DESC depthStencilDesc(
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		static_cast<UINT>(windowWidth),
		static_cast<UINT>(windowHeight),
		1, // This depth stencil view has only one texture.
		1, // Use a single mipmap level.
		D3D11_BIND_DEPTH_STENCIL
	);


	hr = d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, d3dDepthStencil.ReleaseAndGetAddressOf());

	if (FAILED(hr)) return false;

	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);

	hr = d3dDevice->CreateDepthStencilView(
		d3dDepthStencil.Get(),
		&depthStencilViewDesc,
		d3dDepthStencilView.ReleaseAndGetAddressOf()
	);
	
	if (FAILED(hr)) return false;






	// Set the 3D rendering viewport to target the entire window.
	d3dViewport = CD3D11_VIEWPORT(
		0.0f,
		0.0f,
		static_cast<float>(windowWidth),
		static_cast<float>(windowHeight)
	);

	d3dDeviceContext->RSSetViewports(1, &d3dViewport);


	////Create surface to write to with d2d
	//d3dD2dTexture.Reset();
	//d2dSurface.Reset();
	//d2dShaderResourceView.Reset();

	//CD3D11_TEXTURE2D_DESC d2dTexDesc{
	//	DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM,
	//	windowWidth,
	//	windowHeight,
	//	1,
	//	1
	//};



	//d3dDevice->CreateTexture2D(&d2dTexDesc, nullptr,d3dD2dTexture.ReleaseAndGetAddressOf());

	//d3dD2dTexture->QueryInterface(d2dSurface.ReleaseAndGetAddressOf());


	//CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(d3dD2dTexture.Get(),D3D11_SRV_DIMENSION_TEXTURE2D);

	//d3dDevice->CreateShaderResourceView(d3dD2dTexture.Get(), &srvDesc, d2dShaderResourceView.ReleaseAndGetAddressOf());



	//finally, pass into Graphics2D
	//gfx2d.UpdateTarget(d2dSurface);

	return true;
}

#pragma endregion

