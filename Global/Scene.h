
#ifndef SCENE_H
#define SCENE_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

//#include <nanogui/nanogui.h>

//#include "SceneManager.h"
//#include "VertexDataManager.h"


#define _GET_GL_ERROR   { GLenum err = glGetError(); std::cout << "[OpenGL Error] " << glewGetErrorString(err) << std::endl; }

class Scene
{

public:
    Scene();
    Scene( int windowWidth, int windowHeight );
    virtual ~Scene();

    // Public methods

    // Init: called once when the scene is initialized
    virtual int Init();



    // Display : encapsulates per-frame behavior of the scene
    virtual int Display();


    // Render : per frame rendering of the scene
    virtual int Render();

    // cleanup before destruction
    virtual void CleanUp();

   
    

protected:
    int window_HEIGHT, window_WIDTH;


};


#endif 
