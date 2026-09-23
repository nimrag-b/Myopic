#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX 
#include <windows.h>
#include <exception>

namespace err {

	inline void ThrowOnFail(HRESULT hr) {
		if (FAILED(hr)) { 
			throw std::exception();
		}
	}

	void ErrorWithMessage(const wchar_t* msg, const wchar_t* caption = L"Error");

}