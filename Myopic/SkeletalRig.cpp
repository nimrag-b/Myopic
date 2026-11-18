#include "SkeletalRig.h"

SkeletalRig::SkeletalRig(bone_dat* dat, size_t count) {

	assert(count <= MAX_ANIM_BONES);

	mBones.resize(count);

	//set bones
	for (size_t i = 0; i < count; i++)
	{
		mBones[dat[i].id] = Bone{ dat[i].id, dat[i].name,dat[i].offset };

		mBoneNameMap.insert({ dat[i].name, dat[i].id });
	}
	//could do this in 1 loop if dat was in order but its easier to just assume its not and have two loops
	//set parents
	for (size_t i = 0; i < count; i++)
	{
		int parent = dat[i].parent;
		mBones[parent].AddChild(dat[i].id);
	}
}

Bone* SkeletalRig::getBone(int idx) {
	if (idx < 0 || idx >= mBones.size()) {
		return nullptr;
	}
	return &mBones[idx];
}

int SkeletalRig::mapName(const std::string& name) {
	if (mBoneNameMap.contains(name)) {
		return mBoneNameMap.at(name);
	}
	return -1;
}
