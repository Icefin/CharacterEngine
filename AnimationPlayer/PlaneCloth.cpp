#include <algorithm>

#include "PlaneCloth.h"

PlaneCloth::PlaneCloth(glm::vec3 position, glm::vec3 color, uint32 width, uint32 height, uint32 widthNum, uint32 heightNum)
{
	_position = position;

	float dw = static_cast<float>(width) / static_cast<float>(widthNum);
	float dh = static_cast<float>(height) / static_cast<float>(heightNum);
	
	_massPointList.resize(widthNum * heightNum);
	// MassPoint Initialize
	for (int32 h = 0; h < heightNum; ++h)
	{
		for (int32 w = 0; w < widthNum; ++w)
		{
			MassPoint newPoint;
			newPoint.invMass = 1.0f;
			newPoint.prevPosition = glm::vec3(position.x + w * dw, position.y, position.z + h * dh);
			newPoint.position = glm::vec3(position.x + w * dw, position.y, position.z + h * dh);
			newPoint.velocity = glm::vec3(0.0f, 0.0f, 0.0f);
			//newPoint.netForce = glm::vec3(0.0f, 0.0f, 0.0f);
			newPoint.color = color;

			_massPointList[w + h * widthNum] = newPoint;
		}
	}
	_massPointList[0].invMass = 0.0f;
	_massPointList[widthNum - 1].invMass = 0.0f;
	//_massPointList.back().mass = 0.0f;

	// StructuralSpring Initialize
	for (int32 h = 0; h < heightNum - 1; ++h)
	{
		for (int32 w = 0; w < widthNum - 1; ++w)
		{
			MassPoint* origin = &_massPointList[w + h * widthNum];
			MassPoint* right = &_massPointList[(w + 1) + h * widthNum];
			MassPoint* lower = &_massPointList[w + (h + 1) * widthNum];
			
			float restLengthRightward = glm::distance(origin->position, right->position);
			float restLengthDownward = glm::distance(origin->position, lower->position);

			_internalConstraints.push_back({ restLengthRightward, origin, right });
			_internalConstraints.push_back({ restLengthDownward, origin, lower });

			//_springList.push_back({ SpringType::Structural, restLengthRightward, origin, right });
			//_springList.push_back({ SpringType::Structural, restLengthDownward, origin, lower });
		}
	}

	for (int32 w = 0; w < widthNum - 1; ++w)
	{
		MassPoint* origin = &_massPointList[w + (heightNum - 1) * widthNum];
		MassPoint* right = &_massPointList[(w + 1) + (heightNum - 1) * widthNum];

		float restLengthRightward = glm::distance(origin->position, right->position);

		_internalConstraints.push_back({ restLengthRightward, origin, right });
		//_springList.push_back({ SpringType::Structural, restLengthRightward, origin, right });
	}

	for (int32 h = 0; h < heightNum - 1; ++h)
	{
		MassPoint* origin = &_massPointList[widthNum - 1 + h * widthNum];
		MassPoint* lower = &_massPointList[widthNum - 1 + (h + 1) * widthNum];

		float restLengthDownward = glm::distance(origin->position, lower->position);

		_internalConstraints.push_back({ restLengthDownward, origin, lower });
		//_springList.push_back({ SpringType::Structural, restLengthDownward, origin, lower });
	}

	// ShearSpring Initialize
	for (int32 h = 0; h < heightNum - 1; ++h)
	{
		for (int32 w = 0; w < widthNum - 1; ++w)
		{
			MassPoint* origin = &_massPointList[w + h * widthNum];
			MassPoint* lowerRight = &_massPointList[(w + 1) + (h + 1) * widthNum];

			float restLengthLowerRight = glm::distance(origin->position, lowerRight->position);

			_internalConstraints.push_back({ restLengthLowerRight, origin, lowerRight });
			//_springList.push_back({ SpringType::Shear, restLengthLowerRight, origin, lowerRight });
		}
	}

	for (int32 h = 0; h < heightNum - 1; ++h)
	{
		for (int32 w = 1; w < widthNum; ++w)
		{
			MassPoint* origin = &_massPointList[w + h * widthNum];
			MassPoint* lowerLeft = &_massPointList[(w - 1) + (h + 1) * widthNum];

			float restLengthLowerLeft = glm::distance(origin->position, lowerLeft->position);

			_internalConstraints.push_back({ restLengthLowerLeft, origin, lowerLeft });
			//_springList.push_back({ SpringType::Shear, restLengthLowerLeft, origin, lowerLeft });
		}
	}

	// BendSpring Initialize
	for (int32 h = 0; h < heightNum; ++h)
	{
		for (int32 w = 0; w < widthNum - 2; ++w)
		{
			MassPoint* origin = &_massPointList[w + h * widthNum];
			MassPoint* distRight = &_massPointList[(w + 2) + h * widthNum];

			float restLengthDistRight = glm::distance(origin->position, distRight->position);

			_internalConstraints.push_back({ restLengthDistRight, origin, distRight });
			//_springList.push_back({ SpringType::Flexion, restLengthDistRight, origin, distRight });
		}
	}

	for (int32 h = 0; h < heightNum - 2; ++h)
	{
		for (int32 w = 0; w < widthNum; ++w)
		{
			MassPoint* origin = &_massPointList[w + h * widthNum];
			MassPoint* distLower = &_massPointList[w + (h + 2) * widthNum];

			float restLengthDistRight = glm::distance(origin->position, distLower->position);

			_internalConstraints.push_back({ restLengthDistRight, origin, distLower });
			//_springList.push_back({ SpringType::Flexion, restLengthDistRight, origin, distLower });
		}
	}

	// Indices Initialize
	for (uint32 h = 0; h < heightNum - 1; ++h)
	{
		for (uint32 w = 0; w < widthNum - 1; ++w)
		{
			_indices.push_back(w + h * widthNum);
			_indices.push_back((w + 1) + (h + 1) * widthNum);
			_indices.push_back((w + 1) + h * widthNum);

			_indices.push_back(w + h * widthNum);
			_indices.push_back(w + (h + 1) * widthNum);
			_indices.push_back((w + 1) + (h + 1) * widthNum);
		}
	}

	// GLContext Initialize
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	glGenBuffers(1, &_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, _massPointList.size() * sizeof(MassPoint), _massPointList.data(), GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MassPoint), (void*)offsetof(MassPoint, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MassPoint), (void*)offsetof(MassPoint, color));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(MassPoint), (void*)offsetof(MassPoint, normal));

	glGenBuffers(1, &_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(uint32), _indices.data(), GL_STATIC_DRAW);
}

PlaneCloth::~PlaneCloth(void)
{
	glDeleteVertexArrays(1, &_vao);
	glDeleteBuffers(1, &_vbo);
	glDeleteBuffers(1, &_ebo);
}

void PlaneCloth::update(float deltaTime, std::vector<pa::OBB>& colliders)
{
	static int32 _iterationCount = 10;

	//Store Initial Values by using Sympletic Euler integration
	for (MassPoint& massPoint : _massPointList)
	{
		massPoint.velocity = massPoint.velocity + pa::gravity * massPoint.invMass * deltaTime;
		massPoint.prevPosition = massPoint.position;
		massPoint.position = massPoint.position + massPoint.velocity * deltaTime;
		massPoint.color = glm::vec3(0.9f, 0.9f, 0.9f);
	}

	//Collision Detection && Generate Collision Constraint
	std::vector<CollisionConstraint> collisionConstraints;
	for (MassPoint& massPoint : _massPointList)
		generateCollisionConstraint(massPoint, colliders, &collisionConstraints);

	//Solve Constraints
	for (int32 cnt = 0; cnt < _iterationCount; ++cnt)
	{
		for (DistanctConstraint& constraint : _internalConstraints)
			solveDistantConstraint(constraint, deltaTime);

		for (CollisionConstraint& constraint : collisionConstraints)
			solveCollisionConstraint(constraint, deltaTime);
	}

	/*for (MassPoint& massPoint : _massPointList)
	{
		massPoint.color = glm::vec3(0.9f, 0.9f, 0.9f);
		for (pa::OBB& obb : colliders)
		{
			pa::Line travelPath(massPoint.prevPosition, massPoint.position);
			if (pa::isIntersection(travelPath, obb) == true)
			{
				massPoint.color = glm::vec3(1.0f, 0.0f, 0.0f);
				pa::Ray ray(massPoint.position, massPoint.prevPosition - massPoint.position);
				pa::RaycastInfo raycastInfo;
				pa::raycast(ray, obb, &raycastInfo);

				massPoint.position = raycastInfo.hitPoint + raycastInfo.normal * 0.003f;
				break;
			}
			else if (pa::isPointInside(massPoint.position, obb) == true)
			{
				massPoint.color = glm::vec3(1.0f, 0.0f, 0.0f);
				pa::Ray ray(massPoint.prevPosition, massPoint.position - massPoint.prevPosition);
				pa::RaycastInfo raycastInfo;
				pa::raycast(ray, obb, &raycastInfo);

				massPoint.position = raycastInfo.hitPoint + raycastInfo.normal * 0.003f;
				break;
			}
		}
	}*/

	//Velocity Update
	for (MassPoint& massPoint : _massPointList)
		massPoint.velocity = (massPoint.position - massPoint.prevPosition) / deltaTime;

	updateMassPointNormal();
}

void PlaneCloth::generateCollisionConstraint(MassPoint& massPoint, std::vector<pa::OBB> colliders, std::vector<CollisionConstraint>* collisionConstraints)
{
	for (pa::OBB& obb : colliders)
	{
		pa::Line travelPath(massPoint.prevPosition, massPoint.position);
		
		if (pa::isIntersection(travelPath, obb) == true)
		{
			massPoint.color = glm::vec3(1.0f, 0.0f, 0.0f);
			pa::Ray ray(massPoint.prevPosition, massPoint.position - massPoint.prevPosition);
			pa::RaycastInfo raycastInfo;
			pa::raycast(ray, obb, &raycastInfo);

			glm::vec3 targetPosition = raycastInfo.hitPoint + raycastInfo.normal * 0.003f;

			collisionConstraints->push_back({ targetPosition, &massPoint });
			return;
		}
		else if (pa::isPointInside(massPoint.position, obb) == true)
		{
			massPoint.color = glm::vec3(1.0f, 0.0f, 0.0f);
			pa::Ray ray(massPoint.position, massPoint.prevPosition - massPoint.position);
			pa::RaycastInfo raycastInfo;
			pa::raycast(ray, obb, &raycastInfo);

			glm::vec3 targetPosition = raycastInfo.hitPoint + raycastInfo.normal * 0.003f;
			
			collisionConstraints->push_back({ targetPosition, &massPoint });
			return;
		}
	}
}

void PlaneCloth::solveDistantConstraint(DistanctConstraint& constraint, float deltaTime)
{
	MassPoint* left = constraint.left;
	MassPoint* right = constraint.right;

	float invMassSum = left->invMass + right->invMass;

	if (invMassSum == 0.0f)
		return;

	glm::vec3 n = right->position - left->position;
	float distance = glm::length(n);

	glm::vec3 direction = glm::normalize(n);

	glm::vec3 corr = n * (distance - constraint.restLength) / invMassSum;

	left->position += corr * left->invMass;
	right->position -= corr * right->invMass;
}

void PlaneCloth::solveCollisionConstraint(CollisionConstraint& constraint, float deltaTime)
{
	constraint.point->position = constraint.targetPosition;
}

void PlaneCloth::render(Shader& shader)
{
	glm::mat4 worldMat = glm::mat4(1.0f);
	shader.setUniformVec3("material.ambient", _materialAmbient);
	shader.setUniformVec3("material.emissive", _materialEmissive);
	shader.setUniformVec3("material.specular", _materialSpecular);
	shader.setUniformFloat("material.shininess", _materialShininess);
	shader.setUniformMat4("worldMat", worldMat);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glBindVertexArray(_vao);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, _massPointList.size() * sizeof(MassPoint), _massPointList.data());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);

	glLineWidth(1.0f);
	glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, NULL);
}

//void PlaneCloth::applyInternalForces(void)
//{
//	static const float stiffnessList[3] = {
//		1000.0f, 
//		1000.0f,
//		1500.0f
//	};
//
//	static const float dampingCoefficient = 1.0f;
//
//	for (Spring& spring : _springList)
//	{
//		MassPoint* left = spring.left;
//		MassPoint* right = spring.right;
//
//		glm::vec3 posDifference = left->position - right->position;
//		glm::vec3 velDfference = left->velocity - right->velocity;
//
//		float stiffness = stiffnessList[static_cast<uint32>(spring.type)];
//		float dist = glm::length(posDifference);
//
//		glm::vec3 springForce = -stiffness * (dist - spring.restLength) * glm::normalize(posDifference);
//		glm::vec3 dampingForce = -dampingCoefficient * velDfference;
//
//		left->netForce += (springForce + dampingForce);
//		right->netForce -= (springForce + dampingForce);
//	}
//}

//void PlaneCloth::applyExternalForces(void)
//{
//	for (MassPoint& massPoint : _massPointList)
//		massPoint.netForce += glm::vec3(0.0f, -9.81f, 0.0f) * massPoint.mass;
//}

//void PlaneCloth::updateMassPointState(float deltaTime)
//{
//	for (MassPoint& massPoint : _massPointList)
//	{
//		if (massPoint.mass == 0.0f)
//			continue;
//		//Explicit Euler-Method
//		/*glm::vec3 newVelocity = massPoint.velocity * 0.95f + deltaTime * massPoint.netForce / massPoint.mass;
//		glm::vec3 newPosition = massPoint.position + deltaTime * massPoint.velocity;
//
//		massPoint.velocity = newVelocity;
//		massPoint.position = newPosition;*/
//
//		//Verlet Method
//		glm::vec3 acceleration = massPoint.netForce / massPoint.mass;
//		glm::vec3 velocity = massPoint.position - massPoint.prevPosition;
//
//		massPoint.prevPosition = massPoint.position;
//		massPoint.position = massPoint.position + velocity * 0.95f + acceleration * deltaTime * deltaTime;
//		massPoint.netForce = glm::vec3(0.0f, 0.0f, 0.0f);
//	}
//}

//void PlaneCloth::solveConstraint(std::vector<pa::OBB>& constraints)
//{
//	for (MassPoint& massPoint : _massPointList)
//	{
//		massPoint.color = glm::vec3(0.9f, 0.9f, 0.9f);
//		for (pa::OBB& constraint : constraints)
//		{
//			pa::Line travelPath(massPoint.prevPosition, massPoint.position);
//
//			if (pa::isIntersection(travelPath, constraint) == true)
//			{
//				massPoint.color = glm::vec3(1.0f, 0.0f, 0.0f);
//				glm::vec3 velocity = massPoint.position - massPoint.prevPosition;
//				pa::Ray ray(massPoint.prevPosition, velocity);
//
//				pa::RaycastInfo raycastInfo;
//				pa::raycast(ray, constraint, &raycastInfo);
//				if (raycastInfo.isHit == true)
//				{
//					massPoint.position = raycastInfo.hitPoint + raycastInfo.normal * 0.003f;
//					massPoint.prevPosition = massPoint.position;
//					massPoint.velocity = glm::vec3(0.0f, 0.0f, 0.0f);
//					//glm::vec3 vn = raycastInfo.normal * glm::dot(raycastInfo.normal, velocity);
//					//glm::vec3 vt = velocity - vn;
//					//massPoint.velocity = vt + vn * 0.1f;
//					//massPoint.prevPosition = massPoint.position - (vt - vn);
//				}
//			}
//		}
//		//branch test
//	}
//}

void PlaneCloth::updateMassPointNormal(void)
{
	std::vector<std::vector<glm::vec3>> normals(_massPointList.size());
	int32 numIndices = _indices.size();
	for (int32 i = 0; i < numIndices; i += 6) {

		glm::vec3 v1 = _massPointList[_indices[i + 1]].position - _massPointList[_indices[i]].position;
		glm::vec3 v2 = _massPointList[_indices[i + 2]].position - _massPointList[_indices[i]].position;

		glm::vec3 v3 = _massPointList[_indices[i + 4]].position - _massPointList[_indices[i + 3]].position;
		glm::vec3 v4 = _massPointList[_indices[i + 5]].position - _massPointList[_indices[i + 3]].position;

		glm::vec3 normal1 = glm::cross(v1, v2);
		normal1 = glm::normalize(normal1);

		glm::vec3 normal2 = glm::cross(v3, v4);
		normal2 = glm::normalize(normal2);

		normals[_indices[i]].push_back(normal1);
		normals[_indices[i + 1]].push_back(normal1);
		normals[_indices[i + 2]].push_back(normal1);
		normals[_indices[i + 3]].push_back(normal2);
		normals[_indices[i + 4]].push_back(normal2);
		normals[_indices[i + 5]].push_back(normal2);
	}

	for (int32 i = 0; i < normals.size(); i++) {
		glm::vec3 avg{ 0.0f, 0.0f, 0.0f };

		for (int32 j = 0; j < normals[i].size(); j++)
			avg += normals[i][j];

		avg = glm::normalize(avg);
		_massPointList[i].normal = avg;
	}
}
