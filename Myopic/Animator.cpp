#include "Animator.h"


Animator::Animator(SkeletalRig* rig) : mRig(rig) {

	mBoneTransforms.resize(MAX_ANIM_BONES);

	for (size_t i = 0; i < MAX_ANIM_BONES; i++)
	{
		XMStoreFloat4x4(&mBoneTransforms[i], XMMatrixIdentity());
	}

}

void Animator::Update(float delta) {


	if (mAnim) {
		mCurtime += delta;
		mCurtime = fmod(mCurtime, mAnim->time());
		::XMFLOAT4X4 rootMat;
		XMStoreFloat4x4(&rootMat, XMMatrixIdentity());
		UpdateBoneTransform(mRig->rootBone(), rootMat);
	}


}

void Animator::UpdateBoneTransform(Bone* bone, ::XMFLOAT4X4 parentTransform) {

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