#pragma once
#include "Bone.h"
#include <unordered_map>
#include <string>

struct bone_dat {
	char name[MAX_BONE_NAME];
	::XMFLOAT4X4 offset;
	int id;
	int parent;
};

class SkeletalRig {
public:

	SkeletalRig(bone_dat* dat, size_t count);

	Bone* rootBone() {
		return &mBones[0];
	}

	Bone* getBone(int idx);

	int mapName(const std::string& name);
private:
	std::vector<Bone> mBones;

	std::unordered_map<std::string, int> mBoneNameMap;
};