#pragma once
#include "Resource.h"

class Mesh : public IResource {
public:
	struct MeshImpl;
	Mesh(MeshImpl* impl);
	~Mesh();

	MeshImpl* inner() const{
		return impl.get();
	}
private:

	std::unique_ptr<MeshImpl> impl;
};
