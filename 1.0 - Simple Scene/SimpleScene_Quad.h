//
// Created by pushpak on 6/1/18.
//

#ifndef SIMPLE_SCENE_SIMPLESCENE_QUAD_H
#define SIMPLE_SCENE_SIMPLESCENE_QUAD_H

#include "../Common/Scene.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <vector>
#include "GameObject.h"
#include "Model.h"
#include "Camera.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "SpatialPartitioning.h"
//#include "Collision.h"

class SimpleScene_Quad : public Scene
{

public:
    SimpleScene_Quad() = default;
    SimpleScene_Quad( int windowWidth, int windowHeight );
    virtual ~SimpleScene_Quad();


public:
    int Init() override;
    void CleanUp() override;

    void CollisionCheck(GameObject& first, GameObject& second);
    int preRender() override;
    int Render() override;
    int postRender() override;
    inline GLuint GetProgramID() { return programID; }
    inline Camera* GetCamera() { return &camera; }
    void RenderOctTree(SpatialPartitioning::TreeNode* tree, const glm::mat4& projection, const glm::mat4& view, int col);
    void RenderBSPTree(SpatialPartitioning::BSPNode* tree, const glm::mat4& projection, const glm::mat4& view);
    void FreeOctTree(SpatialPartitioning::TreeNode* node);
    void FreeBSPTree(SpatialPartitioning::BSPNode* node);
private:

    // member functions
    void initMembers();

    void SetupNanoGUI(GLFWwindow *pWwindow) override;

    // data members
    GLuint  programID;
    GLfloat   angleOfRotation;
    Model ourModel{}; //To be removed
    Camera camera{};
    Camera miniMapCam{};
    std::map<const char*, Model> models; //contain all the models and their ids (indices)
    std::map<int, const char*> intModelID; //contain all the models and their ids (indices) in int format
    std::vector<GameObject> gameObjList;
    std::map<const char*, Collision::Collider*> boundingVolume;

    std::vector<GameObject*> BVHObjs{ 8 };
    //GameObject* BVHObjs[7];
    bool BVHenabled;
    bool newTree = true;
   
    bool renderBV = true;
    int renderDepth = 0; 
    bool bottomUpTree = false;

    bool OctTreeEnabled = false;
    bool renderOctTree = true;
    bool newOctTree = false;
    int octTreeRenderDepth = 0;
    bool octTreeColoured = false;
    SpatialPartitioning::TreeNode* spatialPartitionTree;
    SpatialPartitioning::BSPNode* BSPTree;
    std::map<const char*, std::vector<SpatialPartitioning::Polygon>> modelPolys;
    std::vector<SpatialPartitioning::Polygon> totalObjPolygons;
    int spatPartTree = 0;
    bool BSPTreeEnabled = false;
    bool renderBSPTree = true;
};


//Externs for BVH
extern bool renderBVHSphere;
extern int indexOfTreeInt;

extern float nearestNeighbourWeight;
extern float combinedVolWeight;
extern float relVolIncreaseWeight;

extern int minPolyCount;
#endif //SIMPLE_SCENE_SIMPLESCENE_QUAD_H
