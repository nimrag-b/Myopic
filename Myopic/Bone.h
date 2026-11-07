#pragma once
#include <DirectXMath.h>

#include "GraphicsDef.h"

#include <vector>
#include <unordered_map>
#include <string>

using namespace DirectX;




struct BonePositionKey {
	::XMFLOAT3 position;
	float time;
};

struct BoneRotationKey {
	::XMFLOAT4 rotation;
	float time;
};

struct BoneScaleKey {
	::XMFLOAT3 scale;
	float time;
};

constexpr size_t MAX_BONE_NAME = 32;

class Bone {
public:

	Bone() {
		mID = -1;
	}

	Bone(int boneID, char* name, ::XMFLOAT4X4& offset) :
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

	 ::XMFLOAT4X4* offset() {
		return &mOffset;
	}

	const char* name() const {
		return mName;
	}


private:
	::XMFLOAT4X4 mOffset; //offset matrix to transform to bone space
	int mID;
	char mName[MAX_BONE_NAME];

	std::vector<int> mChildren; //list of mID of children


};

class AnimBone
{
public:
	AnimBone(int boneID, BonePositionKey* positions, int posCount, BoneRotationKey* rotations, int rotCount, BoneScaleKey* scales, int scaleCount) :
	mID(boneID),
	mPositions(posCount),
	mRotations(rotCount),
	mScales(scaleCount)
	{

		for (size_t i = 0; i < posCount; i++)
		{
			mPositions[i] = positions[i];
		}

		for (size_t i = 0; i < rotCount; i++)
		{
			mRotations[i] = rotations[i];
		}

		for (size_t i = 0; i < scaleCount; i++)
		{
			mScales[i] = scales[i];
		}
	}


	void GetLocalTransform(::XMFLOAT4X4* transform, float animTime) const {
		::XMMATRIX position;
		::XMMATRIX rotation;
		::XMMATRIX scale;

		LerpPosition(&position, animTime);
		LerpRotation(&rotation, animTime);
		LerpScale(&scale, animTime);

		XMStoreFloat4x4(transform, position * rotation * scale);

	}


	int ID() const {
		return mID;
	}


private:
	int mID;
	std::vector<BonePositionKey> mPositions;
	std::vector<BoneRotationKey> mRotations;
	std::vector<BoneScaleKey> mScales;


	float LerpFactor(float last, float next, float animTime) const {

		float offset = animTime - last;
		float span = next - last;
		return offset / span;
	}

	void LerpPosition(::XMMATRIX* outMat, float animTime) const {


		if (mPositions.size() == 1) {

			*outMat = XMMatrixTranslationFromVector(XMLoadFloat3(&mPositions[0].position));
			return;
		}

		int i0 = GetPositionIndex(animTime);
		int i1 = i0 + 1;

		float fac = LerpFactor(mPositions[i0].time, mPositions[i1].time, animTime);

		::XMVECTOR pos = ::XMVectorLerp(XMLoadFloat3(&mPositions[i0].position), XMLoadFloat3(&mPositions[i1].position), fac);

		*outMat = XMMatrixTranslationFromVector(pos);
		return;
	}

	void LerpRotation(::XMMATRIX* outMat, float animTime) const {


		if (mRotations.size() == 1) {

			*outMat = XMMatrixRotationQuaternion(XMLoadFloat4(&mRotations[0].rotation));
			return;
		}

		int i0 = GetRotationIndex(animTime);
		int i1 = i0 + 1;

		float fac = LerpFactor(mRotations[i0].time, mRotations[i1].time, animTime);

		::XMVECTOR rot = ::XMVectorLerp(XMLoadFloat4(&mRotations[i0].rotation), XMLoadFloat4(&mRotations[i1].rotation), fac);

		*outMat = XMMatrixRotationQuaternion(rot);
		return;
	}

	void LerpScale(::XMMATRIX* outMat, float animTime) const {


		if (mScales.size() == 1) {

			*outMat = XMMatrixScalingFromVector(XMLoadFloat3(&mScales[0].scale));
			return;
		}

		int i0 = GetScaleIndex(animTime);
		int i1 = i0 + 1;

		float fac = LerpFactor(mScales[i0].time, mScales[i1].time, animTime);

		::XMVECTOR rot = ::XMVectorLerp(XMLoadFloat3(&mScales[i0].scale), XMLoadFloat3(&mScales[i1].scale), fac);

		*outMat = XMMatrixScalingFromVector(rot);
		return;
	}

	int GetPositionIndex(float animTime) const {
		for (size_t i = 0; i < mPositions.size() - 1; i++)
		{
			if (animTime < mPositions[i + 1].time) {
				return i;
			}
		}
		assert(0);
	}
	int GetRotationIndex(float animTime) const {
		for (size_t i = 0; i < mRotations.size() - 1; i++)
		{
			if (animTime < mRotations[i + 1].time) {
				return i;
			}
		}
		assert(0);
	}
	int GetScaleIndex(float animTime) const {
		for (size_t i = 0; i < mScales.size() - 1; i++)
		{
			if (animTime < mScales[i + 1].time) {
				return i;
			}
		}
		assert(0);
	}


};


struct bone_dat {
	::XMFLOAT4X4 offset;
	char name[MAX_BONE_NAME];
	int id;
	int parent;
};

class SkeletalRig {
public:

	SkeletalRig(bone_dat* dat, size_t count) {

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

	Bone* rootBone() {
		return &mBones[0];
	}

	Bone* getBone(int idx) {
		if (idx < 0 || idx >= mBones.size()) {
			return nullptr;
		}
		return &mBones[idx];
	}

	int mapName(const std::string& name) {
		if (mBoneNameMap.contains(name)) {
			return mBoneNameMap.at(name);
		}
		return -1;
	}
private:
	std::vector<Bone> mBones;

	std::unordered_map<std::string, int> mBoneNameMap;
};

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

	Animator(SkeletalRig* rig) : mRig(rig) {

		mBoneTransforms.resize(MAX_ANIM_BONES);

		for (size_t i = 0; i < MAX_ANIM_BONES; i++)
		{
			XMStoreFloat4x4(&mBoneTransforms[i], XMMatrixIdentity());
		}

	}

	void Update(float delta) {
		

		if (mAnim) {
			mCurtime += delta;
			mCurtime = fmod(mCurtime, mAnim->time());
			::XMFLOAT4X4 rootMat;
			XMStoreFloat4x4(&rootMat, XMMatrixIdentity());
			UpdateBoneTransform(mRig->rootBone(), rootMat);
		}

		
	}

	void Play(Animation* anim) {
		mAnim = anim;

		mCurtime = 0;
	}

	void UpdateBoneTransform(Bone* bone, ::XMFLOAT4X4 parentTransform) {

		AnimBone* abone = mAnim->getBone(bone->ID());

		::XMFLOAT4X4 localTransform;
		if (abone) {
			abone->GetLocalTransform(&localTransform, mCurtime);
		}
		else {
			XMStoreFloat4x4(&localTransform, XMMatrixIdentity());
		}
		

		
		::XMMATRIX transformMatrix = XMLoadFloat4x4(&parentTransform) * XMLoadFloat4x4(&localTransform);
		

		
		XMStoreFloat4x4(&mBoneTransforms[bone->ID()], transformMatrix * XMLoadFloat4x4(bone->offset()));

		::XMFLOAT4X4 worldTransform;

		XMStoreFloat4x4(&worldTransform, transformMatrix);
		for (auto& i : bone->children())
		{
			Bone* b = mRig->getBone(i);
			if (b) {
				UpdateBoneTransform(b, worldTransform);
			}
			
		}
	}
private:
	
	SkeletalRig* mRig;
	Animation* mAnim = nullptr;
	std::vector<::XMFLOAT4X4> mBoneTransforms;
	float mCurtime = 0;
};





