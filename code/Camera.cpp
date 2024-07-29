#include "Camera.h"

// Constructor with vectors for position and up direction, initializes camera orientation
Camera::Camera(glm::vec3 startPosition, glm::vec3 upDirection, float startYaw, float startPitch)
    : Position(startPosition),
    WorldUp(upDirection),
    Yaw(startYaw),
    Pitch(startPitch),
    Front(glm::vec3(0.0f, 0.0f, -8.0f)),
    MovementSpeed(SPEED),
    MouseSensitivity(SENSITIVITY),
    Zoom(ZOOM)
{
    // Initialize the camera vectors based on the given yaw and pitch
    updateVec();
}

// Constructor with individual float values for position and up direction, uses the vector constructor
Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float startYaw, float startPitch)
    : Camera(glm::vec3(posX, posY, posZ), glm::vec3(upX, upY, upZ), startYaw, startPitch) {}

// Returns the view matrix calculated using the camera's position, front, and up vectors
glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(Position, Position + Front, Up);
}

// Processes keyboard input to move the camera based on the direction and delta time
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

// Updates the camera's front, right, and up vectors based on the current yaw and pitch angles
void Camera::updateVec() {
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    newFront.y = sin(glm::radians(Pitch));
    newFront.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(newFront);

    Right = glm::normalize(glm::cross(Front, WorldUp)); // Normalize the right vector
    Up = glm::normalize(glm::cross(Right, Front));      // Normalize the up vector
}


// Processes mouse movement to update the camera's yaw and pitch, with optional pitch constraint
void Camera::Mouse_cam(float xOffset, float yOffset, GLboolean constrainPitch) {
    xOffset *= MouseSensitivity;
    yOffset *= MouseSensitivity;

    Yaw += xOffset;
    Pitch += yOffset;

    if (constrainPitch) {
        if (Pitch > 89.0f) Pitch = 89.0f;
        if (Pitch < -89.0f) Pitch = -89.0f;
    }

    updateVec();
}

// Processes mouse scroll input to zoom in or out
void Camera::Mouse_scroll(float yOffset) {
    Zoom -= yOffset;
    if (Zoom < 1.0f) Zoom = 1.0f;
    if (Zoom > 45.0f) Zoom = 45.0f;
}

