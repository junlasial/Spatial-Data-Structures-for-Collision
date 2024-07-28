#pragma once
#include <glm/glm.hpp>
#include <utility>
#include <vector>
#include "Collision.h"
#include "VisualEntity.h"
#include <array>

namespace BoundingVolume
{


	Collision::AABB createAABB(std::vector<glm::vec3>& vertices);


};

