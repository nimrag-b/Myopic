#pragma once
#include <DirectXMath.h>

#include "GraphicsDef.h"

#include <vector>



struct BonePositionKey {
	DirectX::XMFLOAT3 position;
	float time;
};

struct BoneRotationKey {
	DirectX::XMFLOAT4 rotation;
	float time;
};

struct BoneScaleKey {
	DirectX::XMFLOAT3 scale;
	float time;
};

constexpr size_t MAX_BONE_NAME = 32;

class Bone {
public:

	Bone() : mName("NUL"), mOffset(), mID(-1) {}

	Bone(int boneID, char* name, DirectX::XMFLOAT4X4& offset) :
	mID(boneID),
	mOffset(offset)
	{
		
		strncpy_s(mName, name, MAX_BONE_NAME); //TODO log warning if we get a name too long
	}


	void AddChild(int boneID) {
		mChildren.push_back(boneID);
	}

	const std::vector<int>& children() const {
		return mChildren;
	}

	int ID() const {
		return mID;
	}

	DirectX::XMFLOAT4X4* offset() {
		return &mOffset;
	}

	const char* name() const {
		return mName;
	}


private:
	DirectX::XMFLOAT4X4 mOffset; //offset matrix to transform to bone space
	int mID;
	char mName[MAX_BONE_NAME];

	std::vector<int> mChildren; //list of mID of children


};
