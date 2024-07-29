#ifndef SIMPLE_SCENE_Model_Scene_H
#define SIMPLE_SCENE_Model_Scene_H

#include "../Global/Scene.h"
#include <glm/glm.hpp>
#include <vector>
#include "VisualEntity.h"
#include "Model.h"
#include "Camera.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "partition.h"
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
    void RenderOctTree(partition::TreeNode* tree, const glm::mat4& projection, const glm::mat4& view, int col);
    void RenderBSPTree(partition::BSPNode* tree, const glm::mat4& projection, const glm::mat4& view);
    void FreeOctTree(partition::TreeNode* node);
    void FreeBSPTree(partition::BSPNode* node);

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
    partition::TreeNode* spatialPartitionTree;
    partition::BSPNode* BSPTree;
    std::map<const char*, std::vector<partition::poly_shape>> modelPolys;
    std::vector<partition::poly_shape> totalObjPolygons;
    int spatPartTree = 0;
    bool BSPTreeEnabled = false;
    bool renderBSPTree = true;
};

extern int minPolyCount;

#endif //SIMPLE_SCENE_Model_Scene_H
