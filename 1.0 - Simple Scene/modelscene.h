#ifndef SIMPLE_SCENE_Model_Scene_H
#define SIMPLE_SCENE_Model_Scene_H

#include "../Common/Scene.h"
#include <glm/glm.hpp>
#include <vector>
#include "VisualEntity.h"
#include "Model.h"
#include "Camera.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "SpatialPartitioning.h"
//#include "Collision.h"

class Model_Scene : public Scene
{
public:
    Model_Scene() = default;
    Model_Scene(int windowWidth, int windowHeight);
    ~Model_Scene();

    int Init() override;
    void CleanUp() override;
    int Render() override;

    inline GLuint GetProgramID() { return programID; }
    inline Camera* GetCamera() { return &camera; }
    void RenderOctTree(SpatialPartitioning::TreeNode* tree, const glm::mat4& projection, const glm::mat4& view, int col);
    void RenderBSPTree(SpatialPartitioning::BSPNode* tree, const glm::mat4& projection, const glm::mat4& view);
    void FreeOctTree(SpatialPartitioning::TreeNode* node);
    void FreeBSPTree(SpatialPartitioning::BSPNode* node);

private:
    void initMembers();
    void SetupNanoGUI(GLFWwindow* pWindow) override;

    GLuint programID;
    GLfloat angleOfRotation;
    Model ourModel; // To be removed
    Camera camera;
    Camera miniMapCam;
    std::map<const char*, Model> models; // Contains all the models and their ids (indices)
    std::map<int, const char*> intModelID; // Contains all the models and their ids (indices) in int format
    std::vector<VisualEntity> gameObjList;
    std::map<const char*, Collision::Shape*> boundingVolume;

    std::vector<VisualEntity*> BVHObjs{ 5 };
    bool BVHenabled;
    bool newTree = true;

    bool renderBV = true;
    int renderDepth = 0;

    bool enable_Oct = false;
    bool oct_render = true;
    bool newOctTree = false;
    int depth_OctT = 0;
    bool colored_oct = false;
    SpatialPartitioning::TreeNode* spatialPartitionTree;
    SpatialPartitioning::BSPNode* BSPTree;
    std::map<const char*, std::vector<SpatialPartitioning::poly_shape>> modelPolys;
    std::vector<SpatialPartitioning::poly_shape> totalObjPolygons;
    int spatPartTree = 0;
    bool BSPTreeEnabled = false;
    bool renderBSPTree = true;
};

extern int minimumP_count;

#endif //SIMPLE_SCENE_Model_Scene_H
