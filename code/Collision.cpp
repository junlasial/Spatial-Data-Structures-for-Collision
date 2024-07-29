#include "Collision.h"
#include <algorithm>
#include <glm/glm.hpp>


bool Collision::AABB_self(const AABB& a, const AABB& b)
{
	// Check for overlap on all three axes: X, Y, Z
	for (unsigned int i = 0; i < 3; ++i)
	{
		// If there's no overlap on this axis, the boxes don't overlap
		if (a.m_Max[i] < b.m_Min[i] || b.m_Max[i] < a.m_Min[i])
		{
			return false;
		}
	}
	// If there is overlap on all axes, the boxes overlap
	return true;
}




bool Collision::AABB_Plane(const AABB& b, const Plane& p)
{
	// Calculate the center and half-extents of the AABB
	glm::vec3 center = (b.m_Max + b.m_Min) * 0.5f;
	glm::vec3 halfExtents = b.m_Max - center;

	// Calculate the projection interval radius of the AABB onto the plane's normal
	float radius =
		halfExtents.x * std::abs(p.m_Normal.x) +
		halfExtents.y * std::abs(p.m_Normal.y) +
		halfExtents.z * std::abs(p.m_Normal.z);

	// Calculate the distance from the AABB center to the plane
	float distance = glm::dot(p.m_Normal, center) - p.m_d;

	// Check if the distance is within the projection interval radius
	return std::abs(distance) <= radius;
}






bool Collision::AABB_tri(Triangle& t, const AABB& b)
{
    // Calculate the center and half-extents of the AABB
    glm::vec3 center = (b.m_Min + b.m_Max) * 0.5f;
    glm::vec3 halfExtents = (b.m_Max - b.m_Min) * 0.5f;

    // Translate triangle vertices to AABB's center
    t.v1 -= center;
    t.v2 -= center;
    t.v3 -= center;

    using Vector = glm::vec3;

    // Compute the edge vectors of the triangle
    Vector f0 = t.v2 - t.v1;
    Vector f1 = t.v3 - t.v2;
    Vector f2 = t.v1 - t.v3;

    // Check for separating axes based on the triangle's edge vectors and AABB's axes
    // Test axis for the cross product of AABB's and triangle's edges
    float p0, p1, p2, r;

    // Test axis a00
    p0 = t.v1.z * t.v2.y - t.v1.y * t.v2.z;
    p2 = t.v3.z * (t.v2.y - t.v1.y) - t.v3.z * (t.v2.z - t.v1.z);
    r = halfExtents.y * std::abs(f0.z) + halfExtents.z * std::abs(f0.y);
    if (std::max(-std::max(p0, p2), std::min(p0, p2)) > r) return false; // Separating axis found

    // Check for overlap on all three axes: X, Y, Z
    if (std::max({ t.v1.x, t.v2.x, t.v3.x }) < -halfExtents.x || std::min({ t.v1.x, t.v2.x, t.v3.x }) > halfExtents.x)
        return false;
    if (std::max({ t.v1.y, t.v2.y, t.v3.y }) < -halfExtents.y || std::min({ t.v1.y, t.v2.y, t.v3.y }) > halfExtents.y)
        return false;
    if (std::max({ t.v1.z, t.v2.z, t.v3.z }) < -halfExtents.z || std::min({ t.v1.z, t.v2.z, t.v3.z }) > halfExtents.z)
        return false;

    // Test if the triangle's plane intersects the AABB
    glm::vec3 normal = glm::cross(f0, f1);
    Plane p(normal, glm::dot(normal, t.v1));
    return AABB_Plane(b, p);
}
