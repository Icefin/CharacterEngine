// author: Geonho Shin (icefin@pearlabyss.com)
#include "Transform.h"

QuantizedQuaternion quantizeQuaternion(const glm::quat quaternion, const float scale)
{
	QuantizedQuaternion quantizedQuaternion;

	quantizedQuaternion.qy = static_cast<int16>(quaternion.y * scale);
	quantizedQuaternion.qz = static_cast<int16>(quaternion.z * scale);
	quantizedQuaternion.qw = static_cast<int16>(quaternion.w * scale);

	if (quaternion.x < 0.0f)
		quantizedQuaternion.isPositive = false;
	/*
	if (quaternion.x < 0.0f)
	{
		quantizedQuaternion.qy *= -1.0;
		quantizedQuaternion.qz *= -1.0;
		quantizedQuaternion.qw *= -1.0;
	}
	*/
	return quantizedQuaternion;
}

glm::quat dequantizeQuaternion(const QuantizedQuaternion& quantizedQuaternion, const float scale)
{
	glm::quat quaternion;

	quaternion.y = static_cast<float>(quantizedQuaternion.qy) / scale;
	quaternion.z = static_cast<float>(quantizedQuaternion.qz) / scale;
	quaternion.w = static_cast<float>(quantizedQuaternion.qw) / scale;

	quaternion.x = sqrtf(1.0f - ((quaternion.y * quaternion.y) + (quaternion.z * quaternion.z) + (quaternion.w * quaternion.w)));

	if (quantizedQuaternion.isPositive == false)
		quaternion.x = -quaternion.x;

	return glm::normalize(quaternion);
}

float interpolateCatmullRomSpline(float p0, float p1, float p2, float p3, float t)
{
	float c0 = 0.5f * 2 * p1;
	float c1 = 0.5f * t * (-p0 + p2);
	float c2 = 0.5f * t * t * (2 * p0 - 5 * p1 + 4 * p2 - p3);
	float c3 = 0.5f * t * t * t * (-p0 + 3 * p1 - 3 * p2 + p3);
	return (c0 + c1 + c2 + c3);
}