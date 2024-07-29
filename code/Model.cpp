#include "Model.h"
#include "partition.h"

Model::Model(std::string const& path, bool gamma) : gammaCorrection(gamma)
{
    loadModel(path);
}

void Model::Draw()
{
    for (unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].Draw();
}


void Model::BV_draw()
{
    for (unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].BV_draw();
}

void Model::tri_draw()
{
    for (unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].tri_draw();
}

void Model::CleanUp()
{
    for (unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].CleanUp();
}

void Model::loadModel(std::string const& path)
{
    // read file via ASSIMP
    Assimp::Importer importer;
    importer.SetPropertyBool(AI_CONFIG_PP_PTV_NORMALIZE, true);
    const aiScene* scene = importer.ReadFile(path, aiProcess_ValidateDataStructure | aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_SortByPType | aiProcess_JoinIdenticalVertices
        | aiProcess_RemoveRedundantMaterials | aiProcess_FindDegenerates | aiProcess_FindInvalidData | aiProcess_GenNormals | aiProcess_FixInfacingNormals);
    // check for errors
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
    {
        cout << "ERROR in assimp " << importer.GetErrorString() << endl;
        return;
    }
    // retrieve the dir path of the filepath
    dir = path.substr(0, path.find_last_of('/'));

    // process ASSIMP's root node recursively
    processNode(scene->mRootNode, scene);

    if (path.find("quad.obj"))
        loadLine();
}


void Model::loadLine()
{
    // data to fill
    vector<Vertex> vertices;
    //vector<unsigned int> indices;

    vertices = { {glm::vec3(0.f, 0.f, 0.f)}, {glm::vec3(0.f, 0.f, 1.f)} };

    //indices = { 0, 1, 2, 3, 4, 5 };
    Mesh lineMesh{};
    lineMesh.vertices = vertices;
    lineMesh.setup_M();
    meshes.push_back(lineMesh);
}



void Model::loadTriangle()
{
    // data to fill
    vector<Vertex> vertices;
    //vector<unsigned int> indices;

    vertices = { {glm::vec3(-0.5f, -0.5f, 0.f)}, {glm::vec3(0.5f, -0.5f, 0.f)},
                 { glm::vec3(0.f, 0.5f, 0.f)} };

    //indices = { 0, 1, 2, 3, 4, 5 };
    Mesh triMesh{};
    triMesh.vertices = vertices;
    triMesh.setup_M();
    meshes.push_back(triMesh);
}


void Model::loadBSPPolygons(const std::vector<partition::poly_shape>& polygons)
{
    vector<Vertex> vertices;
    for (auto& poly : polygons)
    {
        for (auto& v : poly.vertices)
            vertices.push_back({ v });
    }

    Mesh polyMesh{};
    polyMesh.vertices = vertices;
    polyMesh.setup_M();
    meshes.push_back(polyMesh);
}

void Model::updateTriangle(std::vector<Vertex>& newVertices)
{
    
    meshes[0].vertices = newVertices;
    meshes[0].update_M();
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
    // Process each mesh located at the current node
    unsigned int meshIndex = 0;
    while (meshIndex < node->mNumMeshes)
    {
        // The node object only contains indices to the actual objects in the scene.
        // The scene contains all the data; node is just to keep stuff organized (like relations between nodes).
        aiMesh* mesh = scene->mMeshes[node->mMeshes[meshIndex]];
        meshes.push_back(processMesh(mesh, scene));
        meshIndex++;
    }

    // After we've processed all of the meshes (if any), we then recursively process each of the children nodes
    unsigned int childIndex = 0;
    while (childIndex < node->mNumChildren)
    {
        processNode(node->mChildren[childIndex], scene);
        childIndex++;
    }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    // Containers to hold vertex data and indices
    vector<Vertex> vertices;
    vector<unsigned int> indices;

    // Iterate over each vertex in the mesh
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector; // Temporary vector to hold the data

        // Extract vertex positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;

        // Extract vertex normals
        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.Normal = vector;

        // Add vertex to the list
        vertices.push_back(vertex);
        combined_v.push_back(vertex.Position);
    }

    // Iterate over each face (each face is a triangle) to get the indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        // Retrieve all indices of the face and store them in the indices vector
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    // Create and return a Mesh object using the extracted data
    return Mesh(vertices, indices);
}
