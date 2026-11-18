#include "AnimBone.h"

AnimBone::AnimBone(int boneID, BonePositionKey* positions, int posCount, BoneRotationKey* rotations, int rotCount, BoneScaleKey* scales, int scaleCount) :
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

void AnimBone::GetLocalTransform(::XMFLOAT4X4* transform, float animTime) const {
	::XMMATRIX position;
	::XMMATRIX rotation;
	::XMMATRIX scale;

	LerpPosition(&position, animTime);
	LerpRotation(&rotation, animTime);
	LerpScale(&scale, animTime);

	XMStoreFloat4x4(transform, position * rotation * scale);

}

float AnimBone::LerpFactor(float last, float next, float animTime) const {

	float offset = animTime - last;
	float span = next - last;
	return offset / span;
}

void AnimBone::LerpPosition(::XMMATRIX* outMat, float animTime) const {


	if (mPositions.size() == 1) {

		*outMat = XMMatrixTranslationFromVector(XMLoadFloat3(&mPositions[0].position));
		return;
	}

	size_t i0 = GetPositionIndex(animTime);
	size_t i1 = i0 + 1;

	float fac = LerpFactor(mPositions[i0].time, mPositions[i1].time, animTime);

	::XMVECTOR pos = ::XMVectorLerp(XMLoadFloat3(&mPositions[i0].position), XMLoadFloat3(&mPositions[i1].position), fac);

	*outMat = XMMatrixTranslationFromVector(pos);
	return;
}

void AnimBone::LerpRotation(::XMMATRIX* outMat, float animTime) const {


	if (mRotations.size() == 1) {

		*outMat = XMMatrixRotationQuaternion(XMLoadFloat4(&mRotations[0].rotation));
		return;
	}

	size_t i0 = GetRotationIndex(animTime);
	size_t i1 = i0 + 1;

	float fac = LerpFactor(mRotations[i0].time, mRotations[i1].time, animTime);

	::XMVECTOR rot = ::XMVectorLerp(XMLoadFloat4(&mRotations[i0].rotation), XMLoadFloat4(&mRotations[i1].rotation), fac);

	*outMat = XMMatrixRotationQuaternion(rot);
	return;
}

void AnimBone::LerpScale(::XMMATRIX* outMat, float animTime) const {


	if (mScales.size() == 1) {

		*outMat = XMMatrixScalingFromVector(XMLoadFloat3(&mScales[0].scale));
		return;
	}

	size_t i0 = GetScaleIndex(animTime);
	size_t i1 = i0 + 1;

	float fac = LerpFactor(mScales[i0].time, mScales[i1].time, animTime);

	::XMVECTOR rot = ::XMVectorLerp(XMLoadFloat3(&mScales[i0].scale), XMLoadFloat3(&mScales[i1].scale), fac);

	*outMat = XMMatrixScalingFromVector(rot);
	return;
}

size_t AnimBone::GetPositionIndex(float animTime) const {
	for (size_t i = 0; i < mPositions.size() - 1; i++)
	{
		if (animTime < mPositions[i + 1].time) {
			return i;
		}
	}
	assert(0);
	return 0;
}

size_t AnimBone::GetRotationIndex(float animTime) const {
	for (size_t i = 0; i < mRotations.size() - 1; i++)
	{
		if (animTime < mRotations[i + 1].time) {
			return i;
		}
	}
	assert(0);
	return 0;
}

size_t AnimBone::GetScaleIndex(float animTime) const {
	for (size_t i = 0; i < mScales.size() - 1; i++)
	{
		if (animTime < mScales[i + 1].time) {
			return i;
		}
	}
	assert(0);
	return 0;
}
