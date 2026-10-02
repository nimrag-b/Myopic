#pragma once
#include "Resource.h"
#include "GraphicsDef.h"

class Texture : public IResource {
	friend class Graphics;
public:

	Texture(ID3D11Device* d3dDevice, D3D11_TEXTURE2D_DESC* desc, D3D11_SUBRESOURCE_DATA* initialData) {

		HRESULT hr = d3dDevice->CreateTexture2D(desc, initialData, d3dTex.ReleaseAndGetAddressOf());


		CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(
			D3D11_SRV_DIMENSION_TEXTURE2D
		);

		hr = d3dDevice->CreateShaderResourceView(d3dTex.Get(), &srvDesc, d3dSrv.ReleaseAndGetAddressOf());
	}


private:
	ComPtr<ID3D11Texture2D> d3dTex;
	ComPtr<ID3D11ShaderResourceView> d3dSrv;
};
