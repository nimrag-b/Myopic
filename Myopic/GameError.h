#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX 
#include <windows.h>
#include <exception>
#include <iostream>
#include <comdef.h>

namespace err {

#define ThrowOnFail(hr) ThrowOnFailEX(hr, #hr )

	inline void ThrowOnFailEX(HRESULT hr, const std::string& msg = "") {
		if (FAILED(hr)) { 
			_com_error err(hr);
			LPCTSTR errMsg = err.ErrorMessage();

			if (!msg.empty()) {
				std::cout << msg << ": ";
			}

			std::wcout << errMsg << std::endl;
			throw std::exception();
		}
	}



	void ErrorWithMessage(const wchar_t* msg, const wchar_t* caption = L"Error");

}