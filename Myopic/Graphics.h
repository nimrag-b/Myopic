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

#include "Camera.h"

#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"

#include "Material.h"

#include "Model.h"

using namespace DirectX;


class Graphics
{
	friend class RenderComponent;
public:
	gfx_err Initialise();


	gfx_err OnResize();

	gfx_err Destroy();

	gfx_err BeginRender();
	gfx_err EndRender();
	gfx_err SetTexture(const Texture* texture, int slot = 0);
	gfx_err SetShader(const Shader* shader);
	gfx_err SetMaterial(const Material* material);
	gfx_err UpdateObjectProps(const Material* material, ObjectDataBuffer& buffer);
	gfx_err RenderMesh(const SubMesh* mesh);

	gfx_err RenderModel(const Model* model, ::XMFLOAT4X4& transform);

	gfx_err RenderSkybox(const Texture* cubemap);

	std::unique_ptr<Shader> LoadShader(const std::string& vertex, const std::string& fragment, VertexLayout* layout);

	void UpdateSun(::XMFLOAT3 direction, ::XMFLOAT3 colour);

	void SetDepthTest(bool value);
	void EnableAlphaBlend();
	void DisableAlphaBlend();
	void DXBindD2dTexture();

	std::unique_ptr<Texture> DXCreateTexture(void* pixels, int width, int height, int pitch, DXGI_FORMAT format);
	std::unique_ptr<Texture> DXCreateCubemap(void* pixels[6], int width, int height, int pitch, DXGI_FORMAT format);

	std::unique_ptr<Shader> DXCreateShader(ComPtr<ID3D11VertexShader> vertex, ComPtr<ID3D11PixelShader> pixel, const VertexLayout& layout);
	ComPtr<ID3D11VertexShader> DXCreateGVertexVertexShader(const std::wstring& fileName);
	ComPtr<ID3D11VertexShader> DXCreateVertexShader(const std::wstring& fileName, VertexLayout* layout);
	ComPtr<ID3D11PixelShader> DXCreatePixelShader(const std::wstring& fileName);


	void CreateSubMesh(SubMesh& mesh, std::vector<float>& vertices, size_t vertex_size_bytes, std::vector<uint32_t>& indices, size_t material_index = 0);

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
	bool Vsync() noexcept {
		return vsync > 0;
	}

	void SetVsync(bool value) noexcept {
		vsync = value ? 1 : 0;
	}

	const uint32_t WindowWidth() const noexcept{
		return windowWidth;
	}
	const uint32_t WindowHeight() const noexcept{
		return windowWidth;
	}
	void DXReportLiveObjects();

	Camera& GetCamera() {
		return camera;
	}

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

	ComPtr<ID3D11Buffer> d3dConstantDataBuffer = nullptr;
	ComPtr<ID3D11Buffer> d3dFrameDataBuffer = nullptr;
	ComPtr<ID3D11Buffer> d3dObjectDataBuffer = nullptr;

	ComPtr<ID3D11DepthStencilState> d3dDisableDepthStencilState = nullptr;

	//Skybox Data
	size_t d3dSkyboxCount = 0;
	SubMesh d3dSkyDome;
	ComPtr<ID3D11Buffer> d3dSkyboxMesh = nullptr;
	std::unique_ptr<Shader> skyboxShader = nullptr;
	VertexLayout d3dSkyboxLayout{ { { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 } } };

	//Render state
	VertexLayout d3dGvertexLayout{ GVertexLayout };
	const VertexLayout* currentVertexLayout = nullptr;
	D3D11_VIEWPORT d3dViewport{ 0,0,0,0 };
	ConstantDataBuffer constantDataBuffer;
	Camera camera;
	XMMATRIX projection;

	uint32_t windowWidth = 0;
	uint32_t windowHeight = 0;

	UINT vsync = 0;


	gfx_err CreateSkyboxResources();




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
