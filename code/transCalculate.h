#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "Model.h"

struct Transform
{
    Transform() = default;
    Transform(glm::vec3 pos, float scale = 0.5f, glm::vec3 scale2 = { 0.5f, 0.5f, 0.5f }, glm::vec3 rot = { 0.f, 0.f, 0.f }, glm::vec3 dir = { 1.f, 0.f, 0.f });

    glm::vec3 Position = { 0.f, 0.f, 0.f };
    glm::vec3 scale2 = { 1.f, 1.f, 1.f };
    float scale{ 1.f };
    glm::vec3 rotation{ 0.f, 0.f, 0.f };
    glm::mat4 matrix_rot{};

    glm::mat4 model_matrix_getter();
    glm::mat4 model_matrix3f_getter();

    std::vector<Vertex> triangleVertices{ 3 };

    glm::vec3 rayDirection{};
    glm::vec3 rayIntersection{};
private:
    glm::mat4 modelMtx;
};
