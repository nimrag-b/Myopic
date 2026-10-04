#pragma once
#include "Resource.h"
#include "GraphicsDef.h"

enum class TextureType {
	NORMAL,
	CUBEMAP,
};

//NOT MANAGED! all data must be deleted manually

class Texture : public IResource {
	friend class Graphics;
public:

	Texture(ID3D11Device* d3dDevice, D3D11_TEXTURE2D_DESC* desc, D3D11_SUBRESOURCE_DATA* initialData, TextureType type) : type(type) {

		HRESULT hr = d3dDevice->CreateTexture2D(desc, initialData, d3dTex.ReleaseAndGetAddressOf());
		

		D3D_SRV_DIMENSION dimension;

		switch (type)
		{
		case TextureType::NORMAL:
			dimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			break;
		case TextureType::CUBEMAP:
			dimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
			break;
		default:
			break;
		}
		CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(
			dimension
		);

		hr = d3dDevice->CreateShaderResourceView(d3dTex.Get(), &srvDesc, d3dSrv.ReleaseAndGetAddressOf());
	}


private:
	ComPtr<ID3D11Texture2D> d3dTex;
	ComPtr<ID3D11ShaderResourceView> d3dSrv;
	TextureType type;
};


struct CubeMap {
	ComPtr<ID3D11Texture2D> d3dTex;
	ComPtr<ID3D11ShaderResourceView> d3dSrv;
};