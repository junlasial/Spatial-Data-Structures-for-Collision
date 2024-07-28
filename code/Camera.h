#pragma once

// Include GLEW for OpenGL functionalities
#include <GL/glew.h>
// Include GLM for vector and matrix operations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Enum class defining possible camera movement directions
enum class Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN,
};

// Default camera values
const float YAW = -90.0f;          // Default yaw angle
const float PITCH = 0.0f;          // Default pitch angle
const float SPEED = 3.0f;          // Default camera movement speed
const float SENSITIVITY = 0.05f;   // Default mouse sensitivity
const float ZOOM = 15.0f;          // Default camera zoom

// Screen settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Abstract camera class to process input and calculate corresponding Euler Angles, Vectors, and Matrices for OpenGL use
class Camera {
public:
    // Camera attributes
    glm::vec3 Position;    // Camera position in world space
    glm::vec3 Front;       // Direction the camera is facing
    glm::vec3 Up;          // Up vector in world space
    glm::vec3 Right;       // Right vector in world space
    glm::vec3 Down;        // Down vector in world space
    glm::vec3 WorldUp;     // Up vector used for calculating the camera's up vector

    // Euler Angles
    float Yaw;             // Yaw angle for rotation around the Y axis
    float Pitch;           // Pitch angle for rotation around the X axis

    // Camera options
    float MovementSpeed;   // Speed of camera movement
    float MouseSensitivity;// Sensitivity of mouse movement
    float Zoom;            // Camera zoom level
    float width = 1280.0f; // Default screen width
    float height = 960.0f; // Default screen height

    // Constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 8.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

    // Constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

    // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix() const;

    // Processes keyboard input for camera movement, accepts direction and delta time
    void KB_input(Camera_Movement direction, float deltaTime);

    // Processes mouse movement input for camera orientation, expects offset values and optional pitch constraint
    void Mouse_cam(float xoffset, float yoffset, GLboolean constrainPitch = true);

    // Processes mouse scroll input for zoom control, expects the scroll offset
    void Mouse_scroll(float yoffset);

    //void RenderMiniMapCam(); // Optional method for rendering a minimap camera
private:
    // Calculates the front vector from the camera's updated Euler Angles
    void updateCameraVectors();
};
