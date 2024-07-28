#include "Scene.h"

// Constructor with default window dimensions
Scene::Scene() : window_WIDTH(100), window_HEIGHT(100) {}

// Constructor with specified window dimensions
Scene::Scene(int windowWidth, int windowHeight)
    : window_WIDTH(windowWidth), window_HEIGHT(windowHeight) {}

// Destructor to clean up resources
Scene::~Scene()
{
    CleanUp();
}

// Initialize the scene (to be overridden in derived classes)
int Scene::Init()
{
    return -1;  // Placeholder return value, should be overridden
}


// Render the scene per frame (to be overridden in derived classes)
int Scene::Render()
{
    return -1;  // Placeholder return value, should be overridden
}

// Clean up resources before the scene is destroyed
void Scene::CleanUp()
{
    // To be implemented in derived classes
}

// Display the scene per frame (calls Render method)
int Scene::Display()
{
    Render();  // Call Render for per-frame rendering
    return -1; // Placeholder return value, should be overridden
}
