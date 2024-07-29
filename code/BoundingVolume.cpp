#include "BoundingVolume.h"

namespace BoundingVolume
{


	Collision::AABB makeAABB(std::vector<glm::vec3>& vertices)
	{
		glm::vec3 Min = glm::vec3(FLT_MAX);
		glm::vec3 Max = glm::vec3(-FLT_MAX);
        // Loop through each vertex in the vector
        for (const glm::vec3& pt : vertices)
        {
            // Update the minimum coordinates for the AABB
            if (pt.x < Min.x) Min.x = pt.x;
            if (pt.y < Min.y) Min.y = pt.y;
            if (pt.z < Min.z) Min.z = pt.z;

            // Update the maximum coordinates for the AABB
            if (pt.x > Max.x) Max.x = pt.x;
            if (pt.y > Max.y) Max.y = pt.y;
            if (pt.z > Max.z) Max.z = pt.z;
        }

        // Return the constructed AABB using the min and max points
        return Collision::AABB(Min, Max);

	}
}