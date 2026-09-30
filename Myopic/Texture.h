#pragma once
#include "Resource.h"


class Texture : public IResource {
public:
	struct TextureImpl;
	Texture(TextureImpl* impl);
	~Texture();

	TextureImpl* inner() const {
		return impl.get();
	}
private:

	std::unique_ptr<TextureImpl> impl;
};
