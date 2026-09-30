#pragma once
#define WIN32_LEAN_AND_MEAN
#include <wrl/client.h>
#include <DirectXMath.h>

template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

using namespace DirectX;

using vector2_t = XMFLOAT2;
using vector3_t = XMFLOAT3;
using vector4_t = XMFLOAT4;


enum class gfx_err {
	OK = 0,
	CONFIG_ERROR,
	BACKEND_ERROR,
	NULL_ERROR,
	UNKNOWN
};

enum class CONSTANT_DATA_FLAGS {
    TEXTURE_DISABLE = 1,
};

//for data that rarely changes
struct alignas(16)  ConstantDataBuffer {
    CONSTANT_DATA_FLAGS flags;
    DirectX::XMFLOAT3 lightDirection;
};

//changed on every frame
struct alignas(16) FrameDataBuffer {
    DirectX::XMFLOAT4X4 projection;
    DirectX::XMFLOAT4X4 view;

    DirectX::XMFLOAT3 viewPos;

};

//changed for every object
struct alignas(16) ObjectDataBuffer {
    DirectX::XMFLOAT4X4 worldMatrix;
    DirectX::XMFLOAT4 meshColor;
    float specular;
    DirectX::XMFLOAT3 fresnelRange;
};



//#define MYOPIC_STRIP_D2D
