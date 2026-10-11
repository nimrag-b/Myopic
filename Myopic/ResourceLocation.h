#pragma once
#include <string>
#include <iostream>

class ResourceLocation {
public:


	static constexpr const char DefaultNameSpace[] = "myopic";
	static constexpr const char ErrorLocation[] = "error/error";


	ResourceLocation(nullptr_t) : mString() {

	}

	ResourceLocation(const char* path) : ResourceLocation(std::string(path)) {

	}

	ResourceLocation(const std::string& path) {



		size_t n = path.find(':');

		if (n == std::string::npos) {
			mString = std::string(DefaultNameSpace) + ':' + path;
			n = sizeof(DefaultNameSpace);
		}
		else {
			mString = path;
		}

		if (mString.substr(0,n-1).find_first_not_of("abcdefghijklmnopqrstuvwxyz") != std::string::npos) {
			std::cout << "Invalid Namespace for path '" << path << "'" << std::endl;
			SetError();
			return;
		}

		//canonise filepaths
		for (size_t i = n+1; i < mString.size(); i++) {
			if (mString[i] == '\\') {
				mString[i] = '/';
			}
		}
		
		if (std::find_if(mString.begin() + n + 1, mString.end(), [](int ch) {return !(isalnum(ch) || ch == '_' || ch == '.' || ch == '/'); }) != mString.end()) {
			std::cout << "Invalid Path for path '" << path << "'" << std::endl;
			SetError();
			return;
		}

		mPathPtr = n;
	}

	bool empty() const {
		return mString.empty();
	}

	const std::string GetNamespace() const {
		return mString.substr(0, mPathPtr - 1);
	}

	const std::string GetPath() const {
		return mString.substr(mPathPtr);
	}

	const std::string& Get() {
		return mString;
	}

	const char* CStr() const {
		return mString.c_str();
	}


	std::string Filepath(const std::string& subfolder, const std::string& ext = "") const {
		std::string res = mString.substr(0, mPathPtr - 1) + '/' + subfolder + '/' + mString.substr(mPathPtr);


		res += ext;
		return res;
	}

private:

	void SetError() {
		mString = std::string(DefaultNameSpace) + ':' + std::string(ErrorLocation);
		mPathPtr = sizeof(DefaultNameSpace);
	}

	std::string mString;
	size_t mPathPtr;

};
