// author: Geonho Shin (icefin@pearlabyss.com)
#include <string>
#include <queue>

#include "CharacterLoader.h"
#include "Transform.h"

constexpr float CHARACTER_SCALE = 0.1f;

/*
static void		TEST_PRINT_BONE_INFO(ASFData* asfData)
{
	std::unordered_map<std::string, ASFBone>& boneMap = asfData->boneMap;

	for (auto& elem : boneMap)
	{
		printf("Bone Name : %s\n", elem.first.c_str());
		printf("Bone direction : %f %f %f\n",elem.second.direction.x, elem.second.direction.y, elem.second.direction.z);
		printf("Bone length : %f\n", elem.second.length);
		printf("Bone Axis : %f %f %f\n", elem.second.orientation.x, elem.second.orientation.y, elem.second.orientation.z);
		printf("\n");
	}
}
static void		TEST_PRINT_POSTURE_INFO(AMCData* amcData)
{
	std::vector<std::vector<AMCPosture>>& boneMotions = amcData->boneMotions;

	for (int32 i = 18; i < 19; i++)
	{
		for (int32 j = 0; j < 500; j++)
		{
			printf("Rotation of (ID : %d, frame : %d) : %f %f %f\n", i, j, glm::degrees(boneMotions[i][j].frameRotation.x), glm::degrees(boneMotions[i][j].frameRotation.y), glm::degrees(boneMotions[i][j].frameRotation.z));
		}
	}
}
*/

void CharacterLoader::loadCharacter(Character& character, std::string& asf, std::string& amc)
{
	ASFData* asfData = _asfParser.readASF(asf);
	AMCData* amcData = _amcParser.readAMC(amc, asfData);

	Skeleton* skeleton = generateSkeleton(asfData);
	Motion* motion = generateMotion(amcData, asfData->totalBoneNumber);

 	character.initialize(skeleton, motion);
}

Skeleton* CharacterLoader::generateSkeleton(ASFData* asfData)
{
	setupToParentMatrix(asfData);

	std::vector<Bone*> boneList(asfData->totalBoneNumber);
	for (auto& elem : asfData->boneMap)
	{
		int32 index = elem.second.boneIndex;
		boneList[index] = generateBone(&elem.second);
	}

	setupSkeletonHierarchy(boneList, asfData);
	
	Skeleton* skeleton = new Skeleton(boneList[0]);
	return skeleton;
}

void	CharacterLoader::setupToParentMatrix(ASFData* asfData)
{
	ASFBone* root = &(asfData->boneMap["root"]);
	root->toParent = glm::mat4(1.0f);

	std::queue<ASFBone*> qBone;

	qBone.push(root);
	while (qBone.empty() == false)
	{
		ASFBone* currentBone = qBone.front(); qBone.pop();

		for (ASFBone* childBone : currentBone->childList)
		{
			computeToParentMatrix(currentBone, childBone);
			qBone.push(childBone);
		}
	}
}

void	CharacterLoader::computeToParentMatrix(ASFBone* parent, ASFBone* child)
{
	glm::mat4 rotation = glm::mat4(1.0f);

	rotation = glm::rotate(rotation, -parent->orientation.x, { 1.0f, 0.0f, 0.0f });
	rotation = glm::rotate(rotation, -parent->orientation.y, { 0.0f, 1.0f, 0.0f });
	rotation = glm::rotate(rotation, -parent->orientation.z, { 0.0f, 0.0f, 1.0f });

	rotation = glm::rotate(rotation, child->orientation.z, { 0.0f, 0.0f, 1.0f });
	rotation = glm::rotate(rotation, child->orientation.y, { 0.0f, 1.0f, 0.0f });
	rotation = glm::rotate(rotation, child->orientation.x, { 1.0f, 0.0f, 0.0f });

	glm::mat4 toChildRotation = glm::mat4(1.0f);

	toChildRotation = glm::rotate(toChildRotation, -child->orientation.x, { 1.0f, 0.0f, 0.0f });
	toChildRotation = glm::rotate(toChildRotation, -child->orientation.y, { 0.0f, 1.0f, 0.0f });
	toChildRotation = glm::rotate(toChildRotation, -child->orientation.z, { 0.0f, 0.0f, 1.0f });

	glm::vec3 offset = glm::vec3(toChildRotation * glm::vec4(parent->direction, 0.0f)) * parent->length * CHARACTER_SCALE;
	glm::mat4 translation = glm::translate(glm::mat4(1.0f), offset);

	child->offset = offset;
	child->toParent = rotation * translation;
}

Bone* CharacterLoader::generateBone(ASFBone* boneData)
{
	Bone* newBone = new Bone();
	
	newBone->index = boneData->boneIndex;
	newBone->toParent = boneData->toParent;
	newBone->direction = boneData->offset;

	return newBone;
}

void	CharacterLoader::setupSkeletonHierarchy(std::vector<Bone*>& boneList, ASFData* asfData)
{
	for (auto& elem : asfData->boneMap)
	{
		ASFBone& asfBone = elem.second;
		int32 index = asfBone.boneIndex;
		for (int32 i = 0; i < asfBone.childList.size(); ++i)
			boneList[index]->childList.push_back(boneList[asfBone.childList[i]->boneIndex]);
	}
}

Motion* CharacterLoader::generateMotion(AMCData* amcData, int32 totalBoneNumber)
{
	int32	totalFrameNumber = amcData->totalFrameNumber;
	std::vector<std::vector<AMCPosture>>& motionDatas = amcData->boneMotions;
	
	Motion* motion = new Motion(totalBoneNumber, totalFrameNumber);
	for (int32 boneIndex = 0; boneIndex < totalBoneNumber; ++boneIndex)
	{
		std::vector<AMCPosture>& posture = motionDatas[boneIndex];
		if (posture.size() == 0)
			continue;
		
		std::vector<AnimationData> animData(totalFrameNumber);
		for (int32 frame = 0; frame < totalFrameNumber; ++frame)
		{
			glm::mat4 rotation = glm::mat4(1.0f);
			rotation = glm::rotate(rotation, posture[frame].frameRotation.z, { 0.0f, 0.0f, 1.0f });
			rotation = glm::rotate(rotation, posture[frame].frameRotation.y, { 0.0f, 1.0f, 0.0f });
			rotation = glm::rotate(rotation, posture[frame].frameRotation.x, { 1.0f, 0.0f, 0.0f });

			animData[frame].rotation = glm::quat(rotation);
		}
		std::vector<CompressedAnimationData> compressedAnimation = compressAnimation(animData);
		motion->setBoneAnimation(boneIndex, compressedAnimation);
	}
	return motion;
}

std::vector<CompressedAnimationData>	CharacterLoader::compressAnimation(std::vector<AnimationData>& data)
{
	static constexpr float kThreshold = 0.1f;

	int32 dataSize = data.size();
	std::vector<std::pair<int32, glm::quat>> keyFrameRotation(dataSize);

	keyFrameRotation[0] = { 0, data[0].rotation };
	keyFrameRotation[1] = { 0, data[0].rotation };
	keyFrameRotation[2] = { dataSize / 2, data[dataSize / 2].rotation };
	keyFrameRotation[3] = { dataSize - 1, data[dataSize - 1].rotation };
	keyFrameRotation[4] = { dataSize - 1, data[dataSize - 1].rotation };
	int32 keyFrameSize = 5;

	bool isCompressed = false;
	while (isCompressed == false)
	{
		float maxOffset = 0.0f;
		int32 maxRangeIndex = 0;

		//threshold check
		for (int32 i = 0; i < keyFrameSize - 3; ++i)
		{
			glm::quat p0 = keyFrameRotation[i].second;
			glm::quat p1 = keyFrameRotation[i + 1].second;
			glm::quat p2 = keyFrameRotation[i + 2].second;
			glm::quat p3 = keyFrameRotation[i + 3].second;

			int32 startFrame = keyFrameRotation[i + 1].first;
			int32 endFrame = keyFrameRotation[i + 2].first;
			int32 frameRange = endFrame - startFrame;

			for (int32 t = startFrame + 1; t < endFrame; ++t)
			{
				glm::quat frameQuat = data[t].rotation;
				float timeStep = (t - startFrame) / frameRange;
				float CRx = interpolateCatmullRomSpline(p0.x, p1.x, p2.x, p3.x, timeStep);
				float CRy = interpolateCatmullRomSpline(p0.y, p1.y, p2.y, p3.y, timeStep);
				float CRz = interpolateCatmullRomSpline(p0.z, p1.z, p2.z, p3.z, timeStep);
				float CRw = interpolateCatmullRomSpline(p0.w, p1.w, p2.w, p3.w, timeStep);

				float currentOffset = abs(CRx - frameQuat.x) + abs(CRy - frameQuat.y) + abs(CRz - frameQuat.z) + abs(CRw - frameQuat.w);
				if (currentOffset > maxOffset)
				{
					maxOffset = currentOffset;
					maxRangeIndex = i;
				}
			}
		}

		if (maxOffset < kThreshold)
		{
			isCompressed = true;
			continue;
		}

		//if is not compressed, insert new key frame
		int32 targetFrame = (keyFrameRotation[maxRangeIndex + 1].first + keyFrameRotation[maxRangeIndex + 2].first) / 2;
		std::pair<int32, glm::quat> newFrame = { targetFrame, data[targetFrame].rotation };
		keyFrameRotation.insert(keyFrameRotation.begin() + maxRangeIndex + 2, newFrame);
		keyFrameSize++;
	}

	std::vector<CompressedAnimationData> compressedAnimation(keyFrameSize);
	for (int32 i = 0; i < keyFrameSize; ++i)
	{
		compressedAnimation[i].keyTime = keyFrameRotation[i].first;
		compressedAnimation[i].rotation = quantizeQuaternion(keyFrameRotation[i].second);
	}
	return compressedAnimation;
}