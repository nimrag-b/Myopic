#pragma once
#include "SkeletalRig.h"
#include "AnimBone.h"

class Animation {
public:

	AnimBone* getBone(int idx) {
		if (idx < 0 || idx >= mBones.size()) {
			return nullptr;
		}

		return &mBones[idx];
	}

	size_t boneCount() const {
		return mBones.size();
	}

	float time() const {
		return mTime;
	}
	float FPS() const {
		return mFramesPerSecond;
	}
private:
	float mTime;
	float mFramesPerSecond;
	std::vector<AnimBone> mBones;

};

class Animator {
public:

	Animator(SkeletalRig* rig);

	void Play(Animation* anim) {
		mAnim = anim;

		mCurtime = 0;
	}

	void Update(float delta);


	void UpdateBoneTransform(Bone* bone, ::XMFLOAT4X4 parentTransform);
private:

	SkeletalRig* mRig;
	Animation* mAnim = nullptr;
	std::vector<::XMFLOAT4X4> mBoneTransforms;
	float mCurtime = 0;
};





