#pragma once
#define WIN32_LEAN_AND_MEAN
#include <d3d11_4.h>

#include "GraphicsDef.h"
#include <string>

#include <SDL3/SDL_video.h>
#include <DirectXMath.h>
#include <vector>
#include <memory>
#include "Resource.h"

#include "Graphics2D.h"

using namespace DirectX;

using POSITION = ::XMFLOAT3;
using COLOUR = ::XMFLOAT3;
using NORMAL = ::XMFLOAT3;
using TEXCOORD = ::XMFLOAT2;
using TANGENT = ::XMFLOAT3;
using BINORMAL = ::XMFLOAT3;



class VertexLayout {
public:

	VertexLayout(D3D11_INPUT_ELEMENT_DESC* desc, size_t size) {
		_desc = std::make_unique< D3D11_INPUT_ELEMENT_DESC[]>(size);
		std::copy(desc, desc + size, _desc.get());
		_size = size;
	}

	VertexLayout(std::initializer_list<D3D11_INPUT_ELEMENT_DESC> desc) {
		_desc = std::make_unique< D3D11_INPUT_ELEMENT_DESC[]>(desc.size());
		std::copy(desc.begin(), desc.end(), _desc.get());
		_size = desc.size();
	}

	template <size_t S>
	VertexLayout(D3D11_INPUT_ELEMENT_DESC(&desc)[S]) {
		_desc = std::make_unique< D3D11_INPUT_ELEMENT_DESC[]>(S);
		std::copy(desc, desc + S, _desc.get());
		_size = S;
	}



	D3D11_INPUT_ELEMENT_DESC* desc() const {
		return _desc.get();
	}
	size_t size() const {
		return _size;
	}
	ID3D11InputLayout* layout() const {
		return _layout.Get();
	}
	ID3D11InputLayout** layout_address() {
		return _layout.GetAddressOf();
	}

	bool has_layout() const {
		return _layout != nullptr;
	}

	void invalidate_layout() {
		_layout.Reset();
	}

	bool operator==(VertexLayout& other) {
		return _layout.Get() == other._layout.Get();
	}
	bool operator!=(VertexLayout& other) {
		return _layout.Get() != other._layout.Get();
	}

	bool operator==(const VertexLayout& other) {
		return _layout.Get() == other._layout.Get();
	}
	bool operator!=(const VertexLayout& other) {
		return _layout.Get() != other._layout.Get();
	}

private:
	ComPtr<ID3D11InputLayout> _layout = nullptr;
	std::unique_ptr<D3D11_INPUT_ELEMENT_DESC[]> _desc;
	size_t _size;
};



//standard game vertex
struct GVertex
{
	POSITION position;
	NORMAL normal;
	TEXCOORD textureCoordinate;
	TANGENT tangent;
	BINORMAL binormal;
};



static D3D11_INPUT_ELEMENT_DESC GVertexLayout[] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(GVertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(GVertex, normal), D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, offsetof(GVertex, textureCoordinate), D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(GVertex, tangent), D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(GVertex, binormal), D3D11_INPUT_PER_VERTEX_DATA, 0},
};





class Texture : public IResource {
public:
	struct TextureImpl;
	Texture(TextureImpl* impl);
	~Texture();

	TextureImpl* inner() {
		return impl.get();
	}
private:

	std::unique_ptr<TextureImpl> impl;
};


class Mesh : public IResource {
public:
	struct MeshImpl;
	Mesh(MeshImpl* impl);
	~Mesh();

	MeshImpl* inner() {
		return impl.get();
	}
private:

	std::unique_ptr<MeshImpl> impl;
};


class Shader : public IResource {
public:
	struct ShaderImpl;
	Shader(ShaderImpl* impl);
	~Shader();
	ShaderImpl* inner() {
		return impl.get();
	}
private:

	std::unique_ptr<ShaderImpl> impl;
};


class Graphics
{
	friend class RenderComponent;
public:
	gfx_err Initialise();


	gfx_err OnResize();

	gfx_err Destroy();

	gfx_err BeginRender();
	gfx_err EndRender();
	gfx_err SetTexture(Texture* texture, int slot = 0);
	gfx_err SetShader(Shader* shader);
	gfx_err RenderMesh(Mesh* mesh, int subMesh = 0);

	std::unique_ptr<Shader> LoadShader(const std::string& vertex, const std::string& fragment, VertexLayout* layout);

	std::unique_ptr<Texture> LoadTexture(const std::string& file);


	void EnableAlphaBlend();
	void DisableAlphaBlend();
	void DXBindD2dTexture();

	std::unique_ptr<Shader> DXCreateShader(ComPtr<ID3D11VertexShader> vertex, ComPtr<ID3D11PixelShader> pixel, const VertexLayout& layout);
	ComPtr<ID3D11VertexShader> DXCreateGVertexVertexShader(const std::wstring& fileName);
	ComPtr<ID3D11VertexShader> DXCreateVertexShader(const std::wstring& fileName, VertexLayout* layout);
	ComPtr<ID3D11PixelShader> DXCreatePixelShader(const std::wstring& fileName);

	std::unique_ptr<Mesh> CreateMeshFlat(void* vertices, size_t count, size_t stride, int subMeshCount = 1);
	template<typename VertexType>
	std::unique_ptr<Mesh> CreateMesh(std::vector<VertexType>& vertices, int subMeshCount = 1)
	{
		return CreateMeshFlat(vertices.data(), vertices.size(), sizeof(VertexType), subMeshCount);
	}

	void SetSubMesh(const std::unique_ptr<Mesh>& mesh, const int subMesh, std::vector<uint32_t>& indices);

	bool GetFullscreen() {
		BOOL fullscreen;
		d3dSwapChain->GetFullscreenState(&fullscreen,nullptr);
		return fullscreen;
	}
	void SetFullscreen(bool value) {
		d3dSwapChain->SetFullscreenState(value, nullptr);
	}

	SDL_Window* SdlWindow() const {
		return sdlWindow;
	}

	const uint32_t WindowWidth() const noexcept{
		return windowWidth;
	}
	const uint32_t WindowHeight() const noexcept{
		return windowWidth;
	}
	void DXReportLiveObjects();


private:
	//SDL resources
	SDL_Window* sdlWindow = nullptr;

	//2D Renderer
	Graphics2D gfx2d;

	//DX11 resources
	HWND hwnd = nullptr;
	ComPtr<ID3D11Device5> d3dDevice = nullptr;
	ComPtr<ID3D11DeviceContext4> d3dDeviceContext = nullptr;
	ComPtr<IDXGISwapChain4> d3dSwapChain = nullptr;
	ComPtr<ID3D11Texture2D> d3dRenderTarget = nullptr;
	ComPtr<ID3D11RenderTargetView1> d3dRenderTargetView = nullptr;
	ComPtr<ID3D11Texture2D> d3dDepthStencil = nullptr;
	ComPtr<ID3D11DepthStencilView> d3dDepthStencilView = nullptr;
	ComPtr<ID3D11SamplerState> d3dSamplerState = nullptr;
	ComPtr<ID3D11BlendState> d3dBlendStateAlpha = nullptr;

	//Render state
	VertexLayout d3dGvertexLayout{ GVertexLayout };
	const VertexLayout* currentVertexLayout = nullptr;
	D3D11_VIEWPORT d3dViewport{ 0,0,0,0 };


	//Render state
	uint32_t windowWidth = 0;
	uint32_t windowHeight = 0;

	//internal DX11 functions

	///Recreates the render target based on the current screen size
	bool DXCreateRenderTarget(); 

	void DXDestroyRenderTarget();
	bool DXCompileShader(const std::wstring& fileName, const std::string& entryPoint, const std::string& profile, ComPtr<ID3DBlob>& shaderBlob);



	/// <summary>
	/// Updates any parts of the pipeline that depends on the window size
	/// </summary>
	/// <returns>true on success</returns>
	bool DXUpdateWindowResources();

};
