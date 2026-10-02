#pragma once
#include <string>
#include <iostream>

class ResourceLocation {
public:

	ResourceLocation(nullptr_t) : _path(), _namespace() {

	}

	ResourceLocation(const char* path) : ResourceLocation(std::string(path)) {

	}

	ResourceLocation(const std::string& path) {



		size_t n = path.find(':');

		if (n == std::string::npos) {
			_namespace = "myopic";
			_path = path;
		}
		else {
			_namespace = path.substr(0, n);
			_path = path.substr(n + 1);
		}

		if (_namespace.find_first_not_of("abcdefghijklmnopqrstuvwxyz") != std::string::npos) {
			std::cout << "Invalid Namespace for path '" << path << "'" << std::endl;
			SetError();
			return;
		}

		
		if (std::find_if(_path.begin(), _path.end(), [](int ch) {return !(isalnum(ch) || ch == '_' || ch == '.'); }) != _path.end()) {
			std::cout << "Invalid Path for path '" << path << "'" << std::endl;
			SetError();
			return;
		}
	}

	bool empty() const {
		return _path.empty();
	}

	const std::string& GetNamespace() const {
		return _namespace;
	}

	const std::string& GetPath() const {
		return _path;
	}

	const std::string ToString() const {
		return _namespace + ':' + _path;
	}

	std::string Filepath(const std::string& subfolder, const std::string& ext = "") const {
		std::string res = _namespace + '/' + subfolder + '/' + _path;

		for (size_t i = 0; i < res.size(); i++) {
			if (res[i] == '.') {
				res[i] = '/';
			}
		}
		res += ext;
		return res;
	}

private:

	void SetError() {
		_namespace = "myopic";
		_path = "error.error";
	}

	std::string _namespace;
	std::string _path;
};
