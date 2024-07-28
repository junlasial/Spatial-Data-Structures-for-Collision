#pragma once
#include <glm/glm.hpp>
#include <vector>

namespace Collision
{
	struct Shape{};
	struct Plane : Shape
	{
		glm::vec3 m_Normal{}; //normalized
		float m_d{}; // d = dot(n,p) for a given point p on the plane
		Plane() {};
		Plane(glm::vec3 normal, float d) : m_Normal{ normal }, m_d {d} {}
	};

	struct Triangle : Shape
	{
		glm::vec3 v1;
		glm::vec3 v2;
		glm::vec3 v3;
		Triangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3)
			: v1 { v1 }, v2 { v2 },
			  v3 { v3 } {}
	};


	struct AABB : Shape
	{
		glm::vec3 m_Min;
		glm::vec3 m_Max;
		AABB(glm::vec3 min, glm::vec3 max) : m_Min{ min }, m_Max{ max } {}
		
	};


	
	bool AABBAABB(const AABB& a, const AABB& b);
	bool AABBPlane(const AABB& b, const Plane& p);
	bool TriangleAABB(Triangle& t, const AABB& b);
	
	


	
};