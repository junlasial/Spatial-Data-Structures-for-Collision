#pragma once

// Include GLEW
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

#include <../code/Mesh.h>



using namespace std;
namespace partition
{
    struct poly_shape;
}
class Model
{
public:
    // model data 
    vector<Mesh> meshes;
    vector<glm::vec3> combined_v;
    std::string dir;
    bool gammaCorrection{};

    Model() = default;
    // constructor, expects a filepath to a 3D model.
    Model(std::string const& path, bool gamma = false);

    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(std::string const& path);
   
    void loadLine();
    void loadTriangle();

    void loadBSPPolygons(const std::vector<partition::poly_shape>& polygons);
    // draws the model, and thus all its meshes
    void Draw();
  
    void BV_draw();
    void tri_draw();
    void updateTriangle(std::vector<Vertex>& newVertices);
    void CleanUp();

private:

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode* node, const aiScene* scene);

    Mesh processMesh(aiMesh* mesh, const aiScene* scene);

};
