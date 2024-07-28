#include "Collision.h"
#include <algorithm>
#include <glm/glm.hpp>


bool Collision::AABBAABB(const AABB& a, const AABB& b)
{
	// for each axis { X, Y, Z }
	for (unsigned int i = 0; i < 3; ++i)
	{
		// if no overlap for the axis, no overlap overall
		if (a.m_Max[i] < b.m_Min[i] || b.m_Max[i] < a.m_Min[i])
			return false;
	}
	return true;
}




// Test if AABB b intersects plane p
bool Collision::AABBPlane(const AABB& b, const Plane& p)
{
	// These two lines not necessary with a (center, extents) AABB representation
	glm::vec3 c = (b.m_Max + b.m_Min) * 0.5f; // Compute AABB center
	glm::vec3 e = b.m_Max - c; // Compute positive extents
	// Compute the projection interval radius of b onto L(t) = b.c + t * p.n
	float r = e[0] * std::abs(p.m_Normal[0]) + e[1] * std::abs(p.m_Normal[1]) + e[2] * std::abs(p.m_Normal[2]);
	// Compute distance of box center from plane
	float s = glm::dot(p.m_Normal, c) - p.m_d;
	// Intersection occurs when distance s falls within [-r,+r] interval
	return std::abs(s) <= r;
}



inline float TriArea2D(float x1, float y1, float x2, float y2, float x3, float y3)
{
	return (x1 - x2) * (y2 - y3) - (x2 - x3) * (y1 - y2);
}





bool Collision::TriangleAABB(Triangle& t, const AABB& b)
{
	float p0, p1, p2, r;
	// Compute box center and extents (if not already given in that format)
	glm::vec3 c = (b.m_Min + b.m_Max) * 0.5f;
	float e0 = (b.m_Max.x - b.m_Min.x) * 0.5f;
	float e1 = (b.m_Max.y - b.m_Min.y) * 0.5f;
	float e2 = (b.m_Max.z - b.m_Min.z) * 0.5f;
	// Translate triangle as conceptually moving AABB to origin
	t.v1 = t.v1 - c;
	t.v2 = t.v2 - c;
	t.v3 = t.v3 - c;
	using Vector = glm::vec3;
	// Compute edge vectors for triangle
	Vector f0 = t.v2 - t.v1, f1 = t.v3 - t.v2, f2 = t.v1 - t.v3;

	// Test axes a00..a22 (category 3)
	// Test axis a00
	p0 = t.v1.z * t.v2.y - t.v1.y * t.v2.z;
	p2 = t.v3.z * (t.v2.y - t.v1.y) - t.v3.z * (t.v2.z - t.v1.z);
	r = e1 * std::abs(f0.z) + e2 * std::abs(f0.y);
	if (std::max(-std::max(p0, p2), std::min(p0, p2)) > r) return 0; // Axis is a separating axis
	// Repeat similar tests for remaining axes a01..a22
		// Test the three axes corresponding to the face normals of AABB b (category 1).
		// Exit if...
		// ... [-e0, e0] and [min(v0.x,v1.x,v2.x), max(v0.x,v1.x,v2.x)] do not overlap
	if (std::max({ t.v1.x, t.v2.x, t.v3.x }) < -e0 || std::min({ t.v1.x, t.v2.x, t.v3.x }) > e0) return 0;
	// ... [-e1, e1] and [min(v0.y,v1.y,v2.y), max(v0.y,v1.y,v2.y)] do not overlap
	if (std::max({ t.v1.y, t.v2.y, t.v3.y }) < -e1 || std::min({ t.v1.y, t.v2.y, t.v3.y }) > e1) return 0;
	// ... [-e2, e2] and [min(v0.z,v1.z,v2.z), max(v0.z,v1.z,v2.z)] do not overlap
	if (std::max({ t.v1.z, t.v2.z, t.v3.z }) < -e2 || std::min({ t.v1.z, t.v2.z, t.v3.z }) > e2) return 0;

	//Test separating axis corresponding to triangle face normal (category 2)
	glm::vec3 normal = glm::cross(f0, f1);
	Plane p(normal, glm::dot(normal, t.v1));
	return AABBPlane(b, p);
}


