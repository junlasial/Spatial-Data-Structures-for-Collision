#pragma once
#include <glm/glm.hpp>
#include "transCalculate.h"
#include "Collision.h"

class VisualEntity
{
public:
    void UpdateTransform(Transform newTransform);
    void AssignModelIdentifier(const char* identifier);
    Transform& AccessTransform();
    const char* FetchModelIdentifier();
    void RenderImGuiControls();

    unsigned int entityID{};
    Transform entityTransform{};

    Collision::AABB boundingVolume{ glm::vec3(0.f), glm::vec3(0.f) };

    const char* currentShape = "AABB";
    bool ShapeModified = true;
    void* treeRenderNode = nullptr;

private:
    const char* modelIdentifier{};
};
