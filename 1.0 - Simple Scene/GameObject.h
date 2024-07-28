#pragma once
#include <glm/glm.hpp>
#include "Transform.h"
#include "Collision.h"

class GameObject
{
public:
    void SetTransform(Transform trans);
    void SetModelID(const char* id);
    Transform& GetTransform();
    const char* GetModelID();
    void DrawImGuiControls();

    unsigned int m_id{};
    Transform transform{};

    // Bounding Volume
    Collision::AABB aabbBV{ glm::vec3(0.f), glm::vec3(0.f) };

    const char* colliderName = "AABB"; // Default collider
    bool changedCollider = true; // Track if the collider has changed
    void* octTreeNode = nullptr; // Used for rendering

private:
    const char* model_id{};
};
