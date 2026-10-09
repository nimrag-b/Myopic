#pragma once
#include "Resource.h"
#include "GraphicsDef.h"


class Shader : public IResource {
	friend class Graphics;
public:
	Shader() {

	}
	gfx_err Create(ComPtr<ID3D11PixelShader> pixelShader, ComPtr<ID3D11VertexShader> vertexShader, const VertexLayout* layout) {
		d3dPixelShader = pixelShader;
		d3dVertexShader = vertexShader;
		mLayout = layout;
		_initialised = true;
		return gfx_err::OK;
	}

	const VertexLayout& Layout() const {
		return *mLayout;
	}
private:
	ComPtr<ID3D11PixelShader> d3dPixelShader = nullptr;
	ComPtr<ID3D11VertexShader> d3dVertexShader = nullptr;
	const VertexLayout* mLayout = nullptr;
};

