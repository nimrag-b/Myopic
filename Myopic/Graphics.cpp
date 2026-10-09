#include "Graphics.h"
#include "GameError.h"
#include <iostream>
#include <comdef.h>
#include <d3dcompiler.h>


#include <SDL3/SDL.h>

#include "Icosphere.h"


#if _DEBUG
#include <dxgidebug.h>
#endif

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")

constexpr uint32_t defaultWindowWidth = 800;
constexpr uint32_t defaultWindowHeight = 600;




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

	//uint32_t dpiWidth = dpi * defaultWindowWidth / USER_DEFAULT_SCREEN_DPI;
	//uint32_t dpiHeight = dpi * defaultWindowHeight / USER_DEFAULT_SCREEN_DPI;

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



	//Buffers

	CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ConstantDataBuffer), D3D11_BIND_CONSTANT_BUFFER);
	err::ThrowOnFail(
		d3dDevice.Get()->CreateBuffer(
			&constantBufferDesc,
			nullptr,
			&d3dConstantDataBuffer
		));
	CD3D11_BUFFER_DESC frameBufferDesc(sizeof(FrameDataBuffer), D3D11_BIND_CONSTANT_BUFFER);
	err::ThrowOnFail(
		d3dDevice.Get()->CreateBuffer(
			&frameBufferDesc,
			nullptr,
			&d3dFrameDataBuffer
		));
	CD3D11_BUFFER_DESC objectBufferDesc(sizeof(ObjectDataBuffer), D3D11_BIND_CONSTANT_BUFFER);
	err::ThrowOnFail(
		d3dDevice.Get()->CreateBuffer(
			&objectBufferDesc,
			nullptr,
			&d3dObjectDataBuffer
		));




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


	constantDataBuffer.flags = CONSTANT_DATA_FLAGS::NONE;
	constantDataBuffer.constantLightColor = { 1.0f, 0.9f, 0.9f };
	constantDataBuffer.constantLightDir = { 1, -1, 0 };
	constantDataBuffer.light_count = 0;

	// Prepare the constant buffer to send it to the graphics device.
	d3dDeviceContext->UpdateSubresource(d3dConstantDataBuffer.Get(), 0, nullptr, &constantDataBuffer, 0, 0);


	CD3D11_DEPTH_STENCIL_DESC desc{ CD3D11_DEFAULT() };
	desc.DepthEnable = false;
	desc.StencilEnable = false;

	err::ThrowOnFail(
		d3dDevice->CreateDepthStencilState(&desc, d3dDisableDepthStencilState.ReleaseAndGetAddressOf())
	);


	CD3D11_RASTERIZER_DESC rsDesc{ CD3D11_DEFAULT() };
	rsDesc.CullMode = D3D11_CULL_NONE;
	rsDesc.FillMode = D3D11_FILL_WIREFRAME;

	d3dDevice->CreateRasterizerState(&rsDesc, &d3dEnableWireframeRasterizer);


	//finally, init any internal gamestate managed by the renderer

	CreateSkyboxResources();


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


gfx_err Graphics::CreateSkyboxResources() {
	float skyboxVertices[] = {
		// positions          
		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
	};


	Icosphere sphere(2);

	auto& skydomeVerts = sphere.verts();
	auto& skydomeIndices = sphere.inds();

	CreateMesh(&d3dSkyDome, skydomeVerts, 3 * sizeof(float), skydomeIndices);


	//create vertex buffer

	D3D11_BUFFER_DESC bd = { 0 };
	D3D11_SUBRESOURCE_DATA initData = { 0 };

	//create vertex buffer
	bd.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE; //regular usage, no streaming or weirdness
	bd.ByteWidth = static_cast<UINT>(sizeof(skyboxVertices));
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER; //bind vertexes to vertex buffer
	bd.CPUAccessFlags = 0;
	initData.pSysMem = skyboxVertices; //attach data

	err::ThrowOnFail(
		d3dDevice->CreateBuffer(&bd, &initData, d3dSkyboxMesh.ReleaseAndGetAddressOf())
	);
	Shader* shader = new Shader;

	LoadShader(shader, "shaders/Cubemap.vs.cso", "shaders/Cubemap.ps.cso", &d3dSkyboxLayout);
	skyboxShader = std::unique_ptr<Shader>(shader);
	return gfx_err::OK;
}

#pragma region Rendering

gfx_err Graphics::BeginRender()
{
	//prepare frame for render


	//set render target
	ID3D11RenderTargetView* const targets[1] = { d3dRenderTargetView.Get() };
	d3dDeviceContext->OMSetRenderTargets(1, targets, d3dDepthStencilView.Get());

	//clear depth buffer
	d3dDeviceContext->ClearDepthStencilView(d3dDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);



	//clear render target
	const float ClearColor[4] = { 0.5f, 0.5f, 0.8f, 1.0f };
	d3dDeviceContext->ClearRenderTargetView(d3dRenderTargetView.Get(), ClearColor);


	d3dDeviceContext->VSSetConstantBuffers(0, 1, d3dConstantDataBuffer.GetAddressOf());
	d3dDeviceContext->VSSetConstantBuffers(1, 1, d3dFrameDataBuffer.GetAddressOf());
	d3dDeviceContext->VSSetConstantBuffers(2, 1, d3dObjectDataBuffer.GetAddressOf());
	//d3dContext->VSSetConstantBuffers(1, 1, m_constantBufferChangeOnResize.GetAddressOf());

	d3dDeviceContext->PSSetConstantBuffers(0, 1, d3dConstantDataBuffer.GetAddressOf());
	d3dDeviceContext->PSSetConstantBuffers(1, 1, d3dFrameDataBuffer.GetAddressOf());
	d3dDeviceContext->PSSetConstantBuffers(2, 1, d3dObjectDataBuffer.GetAddressOf());


	d3dDeviceContext->PSSetSamplers(0, 1, d3dSamplerState.GetAddressOf());





	// Prepare the frame buffer to send it to the graphics device.

	//update view matrix
	FrameDataBuffer frameData; //buffer of things that change every frame to be pushed, mostly camera stuff

	DirectX::XMStoreFloat4x4(&frameData.projection, projection);

	//DirectX::XMStoreFloat4x4(&frameData.view, DirectX::XMMatrixLookAtRH({ 0,0,0 }, { 0,0,-1 }, { 0,1,0 }));
	DirectX::XMStoreFloat4x4(&frameData.view, camera.GetView());

	//XMFLOAT3 lightPos;
	//GetLightMatrix(lightDirection, &frameData.lightView, &frameData.lightProjection, &lightPos);

	frameData.viewPos = camera.Position();

	d3dDeviceContext->UpdateSubresource(d3dFrameDataBuffer.Get(), 0, nullptr, &frameData, 0, 0);



	return gfx_err::OK;
}

gfx_err Graphics::EndRender() {

	gfx2d.Render();
	d3dSwapChain->Present(vsync, 0);

	return gfx_err::OK;
}



#pragma endregion

#pragma region State Management


gfx_err Graphics::SetTexture(const Texture* texture, int slot)
{
	d3dDeviceContext->PSSetShaderResources(slot, 1, texture->d3dSrv.GetAddressOf());
	return gfx_err::OK;
}


void Graphics::EnableDepthTest()
{
	d3dDeviceContext->OMSetDepthStencilState(nullptr, 0);
}
void Graphics::DisableDepthTest()
{
	d3dDeviceContext->OMSetDepthStencilState(d3dDisableDepthStencilState.Get(), 0);
}

void Graphics::EnableWireframe() {
	d3dDeviceContext->RSSetState(d3dEnableWireframeRasterizer.Get());
}
void Graphics::DisableWireframe() {
	d3dDeviceContext->RSSetState(nullptr);
}


void Graphics::EnableAlphaBlend()
{
	d3dDeviceContext->OMSetBlendState(d3dBlendStateAlpha.Get(), NULL, 0xFFFFFF);
}
void Graphics::DisableAlphaBlend()
{
	d3dDeviceContext->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);
}

#pragma endregion



gfx_err Graphics::DXCreateTexture(Texture* pTex, void* pixels, int width, int height, int pitch, DXGI_FORMAT format)
{
	CD3D11_TEXTURE2D_DESC texDesc(
		format,//DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM,
		static_cast<UINT>(width),
		static_cast<UINT>(height),
		1,
		1,
		D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE
	);

	D3D11_SUBRESOURCE_DATA initialData;
	initialData.pSysMem = pixels;
	initialData.SysMemPitch = pitch;
	initialData.SysMemSlicePitch = 0;

	

	return pTex->Create(d3dDevice.Get(), &texDesc, &initialData, TextureType::NORMAL);
}

gfx_err Graphics::DXCreateCubemap(Texture* pTex, void* pixels[6], int width, int height, int pitch, DXGI_FORMAT format)
{
	//stolen from https://www.hlsl.co.uk/blog/2014/11/19/creating-a-cubemap-in-dx11
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 6;
	texDesc.Format = format;
	texDesc.CPUAccessFlags = 0;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	//Array to fill which we will use to point D3D at our loaded CPU images.
	D3D11_SUBRESOURCE_DATA initialData[6];
	for (int i = 0; i < 6; i++)
	{
		//Pointer to the pixel data
		initialData[i].pSysMem = pixels[i];
		//Line width in bytes
		initialData[i].SysMemPitch = pitch;
		// This is only used for 3d textures.
		initialData[i].SysMemSlicePitch = 0;
	}

	

	return pTex->Create(d3dDevice.Get(), &texDesc, initialData, TextureType::CUBEMAP);
}



gfx_err Graphics::SetShader(const Shader* shader) {
	if (shader == nullptr) return gfx_err::NULL_ERROR;
	d3dDeviceContext->VSSetShader(shader->d3dVertexShader.Get(), nullptr, 0);
	d3dDeviceContext->PSSetShader(shader->d3dPixelShader.Get(), nullptr, 0);

	if (currentVertexLayout == nullptr || (currentVertexLayout->layout() != shader->Layout().layout())) {
		currentVertexLayout = &shader->Layout();
		d3dDeviceContext->IASetInputLayout(currentVertexLayout->layout());
	}
	return gfx_err::OK;
}

gfx_err Graphics::SetMaterial(const Material* material)
{
	SetShader(material->mShader.get());
	SetTexture(material->mTexture.get(), 0);
	SetTexture(material->mNormal.get(), 1);

	return gfx_err::OK;
}

gfx_err Graphics::UpdateObjectProps(const Material* material, ObjectDataBuffer& buffer) {
	buffer.fresnelRange = material->mFresnelRange;
	buffer.meshColor = material->mMeshColour;
	buffer.specular = material->mSpecular;
	return gfx_err::OK;
}

gfx_err Graphics::RenderMesh(const Mesh* mesh) {
	if (mesh == nullptr) return gfx_err::NULL_ERROR;

	UINT stride = static_cast<UINT>(mesh->mStride);
	UINT offset = 0;
	//bind mesh
	d3dDeviceContext->IASetVertexBuffers(0, 1, mesh->d3dVertexBuffer.GetAddressOf(), &stride, &offset);

	//bind vertices
	d3dDeviceContext->IASetIndexBuffer(mesh->d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	d3dDeviceContext->DrawIndexed(static_cast<UINT>(mesh->mIndexCount), 0, 0);

	return gfx_err::OK;
}

gfx_err Graphics::RenderModel(const Model* model, ::XMFLOAT4X4& transform)
{
	if (model == nullptr) {
		return gfx_err::NULL_ERROR;
	}
	ObjectDataBuffer odb;
	::XMStoreFloat4x4(&odb.worldMatrix, ::XMMatrixTranspose(::XMLoadFloat4x4(&transform))); //store transposed because hlsl prefers it that way

	for (auto& it : model->mMeshes)
	{
		if (it->mMaterialIndex >= model->mMaterialCount) {
			return gfx_err::CONFIG_ERROR;
		}

		Material* mat = model->mMaterials[it->mMaterialIndex];

		SetMaterial(mat);
		UpdateObjectProps(mat, odb);
		d3dDeviceContext.Get()->UpdateSubresource(d3dObjectDataBuffer.Get(), 0, nullptr, &odb, 0, 0);

		SetTexture(mat->mTexture.get(), 0);
		SetTexture(mat->mNormal.get(), 1);

		RenderMesh(it);
	}

	return gfx_err::OK;
}

gfx_err Graphics::RenderSkybox(const Texture* cubemap)
{
	//disable depth stencil 
	//dont blindly set/reset incase it wasnt set originally
	ID3D11DepthStencilState* depthStencil;
	d3dDeviceContext->OMGetDepthStencilState(&depthStencil, 0);
	d3dDeviceContext->OMSetDepthStencilState(d3dDisableDepthStencilState.Get(), 0);

	SetTexture(cubemap);

	UINT stride = 3 * sizeof(float);
	UINT offset = 0;

	SetShader(skyboxShader.get());

	//d3dDeviceContext->IASetVertexBuffers(0, 1, d3dSkyboxMesh.GetAddressOf(), &stride, &offset);
	//d3dDeviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//d3dDeviceContext->Draw(d3dSkyboxCount, 0);

	RenderMesh(&d3dSkyDome);



	//reenable depth stencil
	d3dDeviceContext->OMSetDepthStencilState(depthStencil, 0);

	return gfx_err::OK;
}


gfx_err Graphics::LoadShader(Shader* pShader, const std::string& vertex, const std::string& fragment, VertexLayout* layout) {

	if (layout == nullptr) {
		layout = &sGvertexLayout;
	}

	const std::wstring wvert = std::wstring(vertex.begin(), vertex.end());
	ComPtr<ID3D11VertexShader> vs = DXCreateVertexShader(wvert, layout);

	const std::wstring wpix = std::wstring(fragment.begin(), fragment.end());
	ComPtr<ID3D11PixelShader> ps = DXCreatePixelShader(wpix);

	return DXCreateShader(pShader, vs, ps, *layout);
}


void Graphics::UpdateSun(::XMFLOAT3 direction, ::XMFLOAT3 colour)
{
	constantDataBuffer.constantLightColor = colour;
	constantDataBuffer.constantLightDir = direction;

	// Prepare the constant buffer to send it to the graphics device.
	d3dDeviceContext->UpdateSubresource(d3dConstantDataBuffer.Get(), 0, nullptr, &constantDataBuffer, 0, 0);
}

gfx_err Graphics::DXCreateShader(Shader* pShader, ComPtr<ID3D11VertexShader> vertex, ComPtr<ID3D11PixelShader> pixel, const VertexLayout& layout)
{
	return pShader->Create(pixel, vertex, &layout);
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
	return DXCreateVertexShader(fileName, &sGvertexLayout);
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


void Graphics::CreateMesh(Mesh* mesh, std::vector<float>& vertices, size_t vertex_size_bytes, std::vector<uint32_t>& indices, size_t material_index)
{
	mesh->Create(d3dDevice.Get(), vertices, vertex_size_bytes, indices, material_index);

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


	float aspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
	float fovAngleY = 60.0f * DirectX::XM_PI / 180.0f;

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	DirectX::XMMATRIX perspectiveMatrix = DirectX::XMMatrixPerspectiveFovLH(
		fovAngleY,
		aspectRatio,
		0.01f,
		1000.0f
	);

	projection = DirectX::XMMatrixTranspose(perspectiveMatrix);

	return true;
}

#pragma endregion

