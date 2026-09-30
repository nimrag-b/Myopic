#pragma once
#include "Resource.h"


class Shader : public IResource {
public:
	struct ShaderImpl;
	Shader(ShaderImpl* impl);
	~Shader();
	ShaderImpl* inner() const {
		return impl.get();
	}
private:

	std::unique_ptr<ShaderImpl> impl;
};
