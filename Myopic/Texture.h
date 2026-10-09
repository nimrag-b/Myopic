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
	Texture(){}

	gfx_err Create(ID3D11Device* d3dDevice, D3D11_TEXTURE2D_DESC* desc, D3D11_SUBRESOURCE_DATA* initialData, TextureType type){


		mType = type;

		HRESULT hr = d3dDevice->CreateTexture2D(desc, initialData, d3dTex.ReleaseAndGetAddressOf());
		if (FAILED(hr)) {
			return gfx_err::BACKEND_ERROR;
		}

		D3D_SRV_DIMENSION dimension = D3D11_SRV_DIMENSION_TEXTURE2D;

		switch (mType)
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
		if (FAILED(hr)) {
			d3dTex.Reset();
			return gfx_err::BACKEND_ERROR;
		}

		_initialised = true;

		return gfx_err::OK;
	}


private:
	ComPtr<ID3D11Texture2D> d3dTex = nullptr;
	ComPtr<ID3D11ShaderResourceView> d3dSrv = nullptr;
	TextureType mType = TextureType::NORMAL;
};
