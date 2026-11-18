#pragma once
#include "Bone.h"

class AnimBone
{
public:
	AnimBone(int boneID, BonePositionKey* positions, int posCount, BoneRotationKey* rotations, int rotCount, BoneScaleKey* scales, int scaleCount);


	void GetLocalTransform(::XMFLOAT4X4* transform, float animTime) const;


	int ID() const {
		return mID;
	}


private:
	int mID;
	std::vector<BonePositionKey> mPositions;
	std::vector<BoneRotationKey> mRotations;
	std::vector<BoneScaleKey> mScales;


	float LerpFactor(float last, float next, float animTime) const;

	void LerpPosition(::XMMATRIX* outMat, float animTime) const;

	void LerpRotation(::XMMATRIX* outMat, float animTime) const;

	void LerpScale(::XMMATRIX* outMat, float animTime) const;

	size_t GetPositionIndex(float animTime) const;
	size_t GetRotationIndex(float animTime) const;
	size_t GetScaleIndex(float animTime) const;


};







