#pragma once
#define WIN32_LEAN_AND_MEAN
#include <wrl/client.h>

template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;


enum class gfx_err {
	OK = 0,
	CONFIG_ERROR,
	BACKEND_ERROR,
	NULL_ERROR,
	UNKNOWN
};


//#define MYOPIC_STRIP_D2D
