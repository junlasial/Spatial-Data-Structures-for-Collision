#pragma once

// GLEW for handling OpenGL extensions
#include <GL/glew.h>
// GLM for mathematics operations related to vectors and matrices
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Enum class for defining camera movement directions
enum class Camera_Movement {
    FORWARD,    // Move camera forward
    BACKWARD,   // Move camera backward
    LEFT,       // Move camera left
    RIGHT,      // Move camera right
    UP,         // Move camera up
    DOWN        // Move camera down
};

// Default camera configuration values
const float YAW = -90.0f;          // Initial yaw angle
const float PITCH = 0.0f;          // Initial pitch angle
const float SPEED = 3.0f;          // Camera movement speed
const float SENSITIVITY = 0.05f;   // Mouse movement sensitivity
const float ZOOM = 15.0f;          // Camera zoom level

// Screen dimensions
const unsigned int width_screen = 1600;  // Default screen width
const unsigned int height_screen = 900; // Default screen height

// Camera class for handling movement and orientation
class Camera {
public:
    // Position and orientation attributes
    glm::vec3 Position;    // Camera's position in world coordinates
    glm::vec3 Front;       // Direction the camera is facing
    glm::vec3 Up;          // Up vector in world coordinates
    glm::vec3 Right;       // Right vector in world coordinates
    glm::vec3 Down;        // Down vector in world coordinates
    glm::vec3 WorldUp;     // Global up vector

    // Euler Angles for camera rotation
    float Yaw;             // Rotation around the Y axis
    float Pitch;           // Rotation around the X axis

    // Configuration for camera options
    float MovementSpeed;   // Speed at which the camera moves
    float MouseSensitivity;// Sensitivity to mouse movement
    float Zoom;            // Zoom level for the camera
    float width = 1600.0f; // Width of the screen
    float height = 900.0f; // Height of the screen

    // Constructor initializing camera with vectors for position and up direction, along with yaw and pitch
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 8.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

    // Constructor initializing camera with individual float values for position and up direction, along with yaw and pitch
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

    // Returns the view matrix for rendering the scene from the camera's perspective
    glm::mat4 GetViewMatrix() const;

    // Handles keyboard input to move the camera based on the given direction and time delta
    void KB_input(Camera_Movement direction, float deltaTime);

    // Handles mouse movement to adjust the camera's orientation, with optional pitch constraint
    void Mouse_cam(float xoffset, float yoffset, GLboolean constrainPitch = true);

    // Handles mouse scroll input to zoom the camera in or out
    void Mouse_scroll(float yoffset);

    //void RenderMiniMapCam(); // Optional function to render a minimap view from the camera
private:
    // Updates the camera vectors based on the current yaw and pitch values
    void updateVec();
};
