// Standard headers
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <cstring>
#include <fstream>

// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

// Local/project headers
#include "../Common/Scene.h"
#include "shader.hpp"
#include "modelscene.h"


#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

// Callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// Function declarations
bool savePPMImageFile(const std::string& filepath, const std::vector<GLfloat>& pixels, int width, int height);

// Global variables
GLFWwindow* window = nullptr;
Scene* scene = nullptr;
SimpleScene_Quad* sceneQuad = nullptr;
int windowWidth = 1280;
int windowHeight = 960;
float lastX = windowWidth / 2.0f;
float lastY = windowHeight / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        std::cin.get();
        return -1;
    }

    // Setting up OpenGL properties
    glfwWindowHint(GLFW_SAMPLES, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(windowWidth, windowHeight, "Assignment 3 Jun", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to open GLFW window.\n";
        std::cin.get();
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        glfwTerminate();
        return -1;
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    // Initialize the scene
    scene = new SimpleScene_Quad(windowWidth, windowHeight);
    sceneQuad = dynamic_cast<SimpleScene_Quad*>(scene);
    scene->Init();

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        scene->Display();

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    // Cleanup IMGUI
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    scene->CleanUp();
    delete scene;

    glfwTerminate();
    return 0;
}

bool savePPMImageFile(const std::string& filepath, const std::vector<GLfloat>& pixels, int width, int height)
{
    std::ofstream texFile(filepath);
    if (!texFile.is_open()) {
        return false;
    }

    texFile << "P3\n" << width << " " << height << "\n255\n";
    auto it = pixels.begin();
    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            texFile << *it++ << " " << *it++ << " " << *it++ << " ";
        }
        texFile << '\n';
    }
    texFile.close();
    return true;
}
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    Camera* camera = sceneQuad->GetCamera();

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera->KB_input(Camera_Movement::FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera->KB_input(Camera_Movement::BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera->KB_input(Camera_Movement::LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera->KB_input(Camera_Movement::RIGHT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camera->KB_input(Camera_Movement::UP, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        camera->KB_input(Camera_Movement::DOWN, deltaTime);
    }
}

// Callback function for resizing the window
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // Update the viewport to match the new window dimensions
    glViewport(0, 0, width, height);

    // Update the camera's dimensions to match the new window size
    Camera* camera = sceneQuad->GetCamera();
    camera->width = static_cast<float>(width);
    camera->height = static_cast<float>(height);
}

// Callback function for scrolling
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    ImGuiIO& io = ImGui::GetIO();
    // Check if ImGui wants to capture the mouse input; if so, do nothing
    if (io.WantCaptureMouse)
        return;

    // Pass the scroll input to the camera for zoom control
    sceneQuad->GetCamera()->Mouse_scroll(static_cast<float>(yoffset));
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    ImGuiIO& io = ImGui::GetIO();
    // If ImGui wants to capture the mouse, do not process the input
    if (io.WantCaptureMouse)
        return;

    // Convert mouse coordinates to float
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    int leftMouseButtonState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

    // Check if the left mouse button is pressed
    if (leftMouseButtonState == GLFW_PRESS) {
        // Initialize the lastX and lastY on the first mouse input
        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        // Calculate the offset in mouse movement
        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // Inverted since y-coordinates range from bottom to top

        // Update lastX and lastY for the next frame
        lastX = xpos;
        lastY = ypos;

        // Pass the mouse offset to the camera
        sceneQuad->GetCamera()->Mouse_cam(xoffset, yoffset);
    }
    else {
        // Update lastX and lastY if the mouse is not pressed
        lastX = xpos;
        lastY = ypos;
    }
}

