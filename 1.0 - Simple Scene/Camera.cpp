#include "Camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Position(position), WorldUp(up), Yaw(yaw), Pitch(pitch),
    Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED),
    MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
    updateCameraVectors();
}

Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
    : Camera(glm::vec3(posX, posY, posZ), glm::vec3(upX, upY, upZ), yaw, pitch) {}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(Position, Position + Front, Up);
}

void Camera::KB_input(Camera_Movement direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;

    if (direction == Camera_Movement::FORWARD) {
        Position += Front * velocity;
    }
    else if (direction == Camera_Movement::BACKWARD) {
        Position -= Front * velocity;
    }
    else if (direction == Camera_Movement::LEFT) {
        Position -= Right * velocity;
    }
    else if (direction == Camera_Movement::RIGHT) {
        Position += Right * velocity;
    }
    else if (direction == Camera_Movement::UP) {
        Position += Up * velocity;
    }
    else if (direction == Camera_Movement::DOWN) {
        Position -= Up * velocity;
    }
}


void Camera::Mouse_cam(float xoffset, float yoffset, GLboolean constrainPitch) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    if (constrainPitch) {
        if (Pitch > 89.0f) Pitch = 89.0f;
        if (Pitch < -89.0f) Pitch = -89.0f;
    }

    updateCameraVectors();
}

void Camera::Mouse_scroll(float yoffset) {
    Zoom -= yoffset;
    if (Zoom < 1.0f) Zoom = 1.0f;
    if (Zoom > 45.0f) Zoom = 45.0f;
}

void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);

    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}
