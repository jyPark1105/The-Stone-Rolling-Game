#ifndef ANIMATOR_H
#define ANIMATOR_H

#include "animation.h"

class Animator
{
private:
	std::vector<glm::mat4> finalBoneMatrices;
	Animation* currentAnimation;
	Animation* nextAnimation;
	Animation* queueAnimation;
	float currentTime;
	bool interpolating;
	float haltTime;
	float interTime;

	bool shouldRepeat = true;
	bool freezeAtEnd = false;
	bool isFrozen = false;

	int boneUpdateInterval = 5;			// 5 Frame에 한 번 계산
	int boneUpdateCounter = 0;         // 현재 프레임 카운터

public:
	Animator()
	{
		currentTime = 0.0;
		interpolating = false;
		haltTime = 0.0;
		interTime = 0.0;

		currentAnimation = nullptr;
		nextAnimation = nullptr;
		queueAnimation = nullptr;

		finalBoneMatrices.reserve(100);

		for (int i = 0; i < 100; i++)
			finalBoneMatrices.push_back(glm::mat4(1.0f));
	}

	Animation* getCurrentAnimation() const {
		return currentAnimation;
	}

	float getCurrentAnimationDurationInSeconds() const {
		if (!currentAnimation) return 1.0f;
		float tps = currentAnimation->getTicksPerSecond();
		if (tps == 0.0f) tps = 1.0f;
		return currentAnimation->getDuration() / tps;
	}

	void updateAnimation(float dt)
	{
		if (currentAnimation) {

			if (freezeAtEnd && isFrozen) {
				// 애니메이션 끝 프레임 유지
				calculateBoneTransform(currentAnimation->getRootNode(), glm::mat4(1.0f), currentAnimation, currentAnimation->getDuration());
				return;
			}

			currentTime += currentAnimation->getTicksPerSecond() * dt;

			if (!shouldRepeat && currentTime >= currentAnimation->getDuration()) {
				currentTime = currentAnimation->getDuration(); // 마지막 프레임 고정
				isFrozen = freezeAtEnd; // 다음 프레임부터 멈춤 처리
			}
			else {
				currentTime = fmod(currentTime, currentAnimation->getDuration());
			}

			float transitionTime = currentAnimation->getTicksPerSecond() * 0.2f;
			if (interpolating && interTime <= transitionTime) {
				interTime += currentAnimation->getTicksPerSecond() * dt;
				calculateBoneTransition(currentAnimation->getRootNode(), glm::mat4(1.0f), currentAnimation, nextAnimation, haltTime, interTime, transitionTime);
				return;
			}
			else if (interpolating) {
				if (queueAnimation) {
					currentAnimation = nextAnimation;
					haltTime = 0.0f;
					nextAnimation = queueAnimation;
					queueAnimation = nullptr;
					currentTime = 0.0f;
					interTime = 0.0;
					return;
				}

				interpolating = false;
				currentAnimation = nextAnimation;
				currentTime = 0.0;
				interTime = 0.0;
			}

			// 프레임 스킵 로직
			boneUpdateCounter++;
			if (boneUpdateCounter < boneUpdateInterval) {
				return; // 본 계산 스킵
			}
			boneUpdateCounter = 0;

			// 실제 본 계산
			calculateBoneTransform(currentAnimation->getRootNode(), glm::mat4(1.0f), currentAnimation, currentTime);
		}
	}


	void playAnimation(Animation* pAnimation, bool repeat = true, bool freeze = false)
	{
		if (!currentAnimation) {
			currentAnimation = pAnimation;
			shouldRepeat = repeat;
			freezeAtEnd = freeze;
			isFrozen = false;
			return;
		}

		if (interpolating) {
			if (pAnimation != nextAnimation)
				queueAnimation = pAnimation;
		}
		else {
			if (pAnimation != nextAnimation) {
				interpolating = true;
				haltTime = fmod(currentTime, currentAnimation->getDuration());
				nextAnimation = pAnimation;
				currentTime = 0.0f;
				interTime = 0.0;
				shouldRepeat = repeat;
				freezeAtEnd = freeze;
				isFrozen = false;
			}
		}
	}

	void calculateBoneTransition(const AssimpNodeData* curNode, glm::mat4 parentTransform, Animation* prevAnimation, Animation* nextAnimation, float haltTime, float currentTime, float transitionTime)
	{
		std::string nodeName = curNode->name;
		glm::mat4 transform = curNode->transformation;

		Bone* prevBone = prevAnimation->findBone(nodeName);
		Bone* nextBone = nextAnimation->findBone(nodeName);

		if (prevBone && nextBone)
		{
			KeyPosition prevPos = prevBone->getPositions(haltTime);
			KeyRotation prevRot = prevBone->getRotations(haltTime);
			KeyScale prevScl = prevBone->getScalings(haltTime);

			KeyPosition nextPos = nextBone->getPositions(0.0f);
			KeyRotation nextRot = nextBone->getRotations(0.0f);
			KeyScale nextScl = nextBone->getScalings(0.0f);

			prevPos.timeStamp = 0.0f;
			prevRot.timeStamp = 0.0f;
			prevScl.timeStamp = 0.0f;

			nextPos.timeStamp = transitionTime;
			nextRot.timeStamp = transitionTime;
			nextScl.timeStamp = transitionTime;

			glm::mat4 p = interpolatePosition(currentTime, prevPos, nextPos);
			glm::mat4 r = interpolateRotation(currentTime, prevRot, nextRot);
			glm::mat4 s = interpolateScaling(currentTime, prevScl, nextScl);

			transform = p * r * s;
		}

		glm::mat4 globalTransformation = parentTransform * transform;

		auto boneProps = nextAnimation->getBoneProps();
		for (unsigned int i = 0; i < boneProps.size(); i++) {
			if (boneProps[i].name == nodeName) {
				glm::mat4 offset = boneProps[i].offset;
				finalBoneMatrices[i] = globalTransformation * offset;
				break;
			}
		}

		for (int i = 0; i < curNode->childrenCount; i++)
			calculateBoneTransition(&curNode->children[i], globalTransformation, prevAnimation, nextAnimation, haltTime, currentTime, transitionTime);
	}

	void calculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform, Animation* animation, float currentTime)
	{
		std::string nodeName = node->name;
		glm::mat4 boneTransform = node->transformation;

		Bone* bone = animation->findBone(nodeName);

		if (bone)
		{
			bone->update(currentTime);
			boneTransform = bone->getTransform();
		}

		glm::mat4 globalTransformation = parentTransform * boneTransform;

		auto boneProps = animation->getBoneProps();

		for (unsigned int i = 0; i < boneProps.size(); i++) {
			if (boneProps[i].name == nodeName) {
				glm::mat4 offset = boneProps[i].offset;
				finalBoneMatrices[i] = globalTransformation * offset;
				break;
			}
		}

		for (int i = 0; i < node->childrenCount; i++)
			calculateBoneTransform(&node->children[i], globalTransformation, animation, currentTime);
	}

	std::vector<glm::mat4> getFinalBoneMatrices()
	{
		return finalBoneMatrices;
	}
};

#endif