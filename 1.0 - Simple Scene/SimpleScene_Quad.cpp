//
// Created by pushpak on 6/1/18.
//
#include "SimpleScene_Quad.h"
#include <shader.hpp>
#include <glm/vec3.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Transform.h"
#include "Collision.h"
#include "BoundingVolume.h"
#include <random>




int minPolyCount = 30;

float model_scale = 1.0f;
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
void SimpleScene_Quad::SetupNanoGUI(GLFWwindow* pWindow)
{
	pWindow = nullptr;
}

bool compareX(GameObject* a, GameObject* b)
{
	return a->transform.Position.x < b->transform.Position.x;
}

bool compareY(GameObject* a, GameObject* b)
{
	return a->transform.Position.y < b->transform.Position.y;
}

bool compareZ(GameObject* a, GameObject* b)
{
	return a->transform.Position.z < b->transform.Position.z;
}


//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
SimpleScene_Quad::~SimpleScene_Quad()
{
	initMembers();
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
SimpleScene_Quad::SimpleScene_Quad(int windowWidth, int windowHeight) : Scene(windowWidth, windowHeight),
programID(0), angleOfRotation(0.0f)
{
	initMembers();
}

//#pragma clang diagnostic push
//#pragma ide diagnostic ignored "bugprone-unused-return-value"
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
void SimpleScene_Quad::initMembers()
{
	programID = 0;
	angleOfRotation = 0.0f;
}
//#pragma clang diagnostic pop

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
void SimpleScene_Quad::CleanUp()
{
	// Cleanup VBO
	for (auto model : models)
		model.second.CleanUp();
	glDeleteProgram(programID);
	
	if (spatialPartitionTree != nullptr)
	{
		FreeOctTree(spatialPartitionTree);
	}
	if (BSPTree != nullptr)
	{
		FreeBSPTree(BSPTree);
	}
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
int SimpleScene_Quad::Init()
{
	// Load models
	Model cube;
	cube.loadModel("Common/models/cube.obj");
	models.emplace("Cube", cube);
	intModelID.emplace(1, "Cube");

	Model fourSphere;
	fourSphere.loadModel("Common/models/1/part_a/g0.obj");
	models.emplace("4Sphere", fourSphere);
	intModelID.emplace(2, "4Sphere");

	Model fourSphere1;
	fourSphere1.loadModel("Common/models/1/part_a/g1.obj");
	models.emplace("4Sphere1", fourSphere1);
	intModelID.emplace(3, "4Sphere1");

	Model fourSphere2;
	fourSphere2.loadModel("Common/models/1/part_b/g0.obj");
	models.emplace("4Sphere2", fourSphere2);
	intModelID.emplace(4, "4Sphere2");

	Model fourSphere3;
	fourSphere3.loadModel("Common/models/1/part_b/g1.obj");
	models.emplace("4Sphere3", fourSphere3);
	intModelID.emplace(5, "4Sphere3");



	Model fourSphere4;
	fourSphere4.loadModel("Common/models/1/part_a/g2.obj");
	models.emplace("4Sphere4", fourSphere4);
	intModelID.emplace(5, "4Sphere4");



	GameObject second;
	second.SetTransform(Transform(glm::vec3{ 0.f, 0.f, 0.f }, model_scale)); // Default position and scale for fourSphere
	second.SetModelID("4Sphere");
	gameObjList.push_back(second);
	second.m_id = 1;

	GameObject third;
	third.SetTransform(Transform(glm::vec3{ 0.f, 0.f, 0.f }, model_scale)); // Default position and scale for fourSphere1
	third.SetModelID("4Sphere1");
	gameObjList.push_back(third);
	third.m_id = 2;

	GameObject fourth;
	fourth.SetTransform(Transform(glm::vec3{ 0.f, 0.f, 0.f }, model_scale)); // Default position and scale for fourSphere2
	fourth.SetModelID("4Sphere2");
	gameObjList.push_back(fourth);
	fourth.m_id = 3;

	GameObject fifth;
	fifth.SetTransform(Transform(glm::vec3{ 0.f, 0.f, 0.f }, model_scale)); // Default position and scale for fourSphere3
	fifth.SetModelID("4Sphere3");
	gameObjList.push_back(fifth);
	fifth.m_id = 4;


	GameObject sixth;
	sixth.SetTransform(Transform(glm::vec3{ 0.f, 0.f, 0.f }, model_scale)); // Default position and scale for fourSphere3
	sixth.SetModelID("4Sphere4");
	gameObjList.push_back(sixth);
	sixth.m_id = 5;

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("Common/shaders/DiffuseShader.vert", "Common/shaders/DiffuseShader.frag");

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	miniMapCam.Position = { 0.f, 0.f, 4.f };
	miniMapCam.Zoom = 60.f;

	// Get polygons from models
	for (auto& model : models)
	{
		modelPolys.insert({ model.first, SpatialPartitioning::getPolygonsFromModel(model.second) });
	}

	// Get polygons for every game object, put in a single vector
	for (auto& obj : gameObjList)
	{
		std::vector<SpatialPartitioning::Polygon> objPolys = SpatialPartitioning::getPolygonsOfObj(modelPolys[obj.GetModelID()], obj);
		totalObjPolygons.insert(totalObjPolygons.end(), objPolys.begin(), objPolys.end());
	}

	return Scene::Init();
}

void SimpleScene_Quad::CollisionCheck(GameObject& first, GameObject& second)
{


}

int SimpleScene_Quad::preRender()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glCullFace(GL_BACK);
	return 0;
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
int SimpleScene_Quad::Render()
{
	glUseProgram(programID);

	glEnableVertexAttribArray(0);

	//Update gameObject attribues (Transform, BV)
	for (int i = 0; i < gameObjList.size(); ++i)
	{
		Transform firstTrans(gameObjList[i].GetTransform().Position, gameObjList[i].GetTransform().scale, gameObjList[i].GetTransform().scale2, gameObjList[i].GetTransform().rotation, gameObjList[i].GetTransform().rayDirection);
		firstTrans.triangleVertices = gameObjList[i].GetTransform().triangleVertices;
		gameObjList[i].SetTransform(firstTrans);
		if (gameObjList[i].changedCollider)
		{
			gameObjList[i].aabbBV = BoundingVolume::createAABB(models[gameObjList[i].GetModelID()].combinedVertices);
			gameObjList[i].aabbBV.m_Min = gameObjList[i].transform.Position + gameObjList[i].aabbBV.m_Min;
			gameObjList[i].aabbBV.m_Max = gameObjList[i].transform.Position + gameObjList[i].aabbBV.m_Max;

			gameObjList[i].sphereBV.m_Position = gameObjList[i].transform.Position + gameObjList[i].sphereBV.m_Position; //offset sphere position with gameobj position

			BVHObjs[i] = &gameObjList[i];
			gameObjList[i].changedCollider = false;
		}
		BVHObjs[i] = &gameObjList[i];

	}



	// ---------- Rendering ----------

	// Camera transform
	glViewport(0, 0, (GLsizei)(camera.width), (GLsizei)(camera.height));
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), camera.width / camera.height, 0.1f, 100.0f);
	glm::mat4 view = camera.GetViewMatrix();

	for (auto& gameObjs : gameObjList)
	{
		Transform gameObjTrans = gameObjs.GetTransform();

		glm::mat4 objTrans;
		if (gameObjs.GetModelID() == "Cube")
			objTrans = projection * view * gameObjTrans.GetModelMtx3f();
		else
			objTrans = projection * view * gameObjTrans.GetModelMtx();
		// Uniform transformation (vertex shader)
		GLint vTransformLoc = glGetUniformLocation(programID, "vertexTransform");
		glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &objTrans[0][0]);
		//glm::vec3 colour{ 0.f, 1.f, 0.f };

		GLint fCamPosLoc = glGetUniformLocation(programID, "cameraPos");
		glUniform3f(fCamPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);
		glUniform1i(glGetUniformLocation(programID, "renderBoundingVolume"), false);
		//Render the model of the gameObjs
		if (!BSPTreeEnabled)
			models[gameObjs.GetModelID()].Draw();

		if (renderBV && !BSPTreeEnabled && !OctTreeEnabled)
		{
			//Render the model of the Bounding Volume
			if (gameObjs.colliderName == "AABB")
			{
				//AABB Update (for now only translation and scale)
				//Set transform for the cube based on the AABB size
				//half extents are the scale
				float scaleX = (gameObjs.aabbBV.m_Max.x - gameObjs.aabbBV.m_Min.x) * 0.5f;
				float scaleY = (gameObjs.aabbBV.m_Max.y - gameObjs.aabbBV.m_Min.y) * 0.5f;
				float scaleZ = (gameObjs.aabbBV.m_Max.z - gameObjs.aabbBV.m_Min.z) * 0.5f;

				glm::vec3 aabbScale{ scaleX, scaleY, scaleZ };
				glm::vec3 aabbCentre = (gameObjs.aabbBV.m_Max + gameObjs.aabbBV.m_Min) / 2.f;
				Transform aabbTrans(aabbCentre, gameObjs.GetTransform().scale, aabbScale, { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f });

				objTrans = projection * view * aabbTrans.GetModelMtx3f();
				GLint vTransformLoc = glGetUniformLocation(programID, "vertexTransform");
				glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &objTrans[0][0]);
				glUniform1i(glGetUniformLocation(programID, "renderBoundingVolume"), true);
				glm::vec3 colour = glm::vec3(0.f, 1.f, 0.f);
				if (OctTreeEnabled && spatialPartitionTree)
				{
					colour = glm::vec3(0.f, 1.f, 0.f);
					//SpatialPartitioning::TreeNode* node = static_cast<SpatialPartitioning::TreeNode*>(gameObjs.octTreeNode);
					//if (node->depth > 0)
					//	colour = node->colour;
				}
				glUniform3f(glGetUniformLocation(programID, "renderColour"), colour.x, colour.y, colour.z);
				//Draw
				models["Cube"].DrawBoundingVolume();
			}
			else if (gameObjs.colliderName == "Ritter's Sphere" || gameObjs.colliderName == "PCA Sphere"
				|| gameObjs.colliderName == "Larsson's EPOS8" || gameObjs.colliderName == "Larsson's EPOS12" || gameObjs.colliderName == "Larsson's EPOS24") //spheres
			{
				float sphereScale{ gameObjs.sphereBV.m_Radius };
				Transform sphereTrans(gameObjs.sphereBV.m_Position, sphereScale, { sphereScale, sphereScale, sphereScale }, { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f });

				objTrans = projection * view * sphereTrans.GetModelMtx();
				GLint vTransformLoc = glGetUniformLocation(programID, "vertexTransform");
				glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &objTrans[0][0]);
				glUniform1i(glGetUniformLocation(programID, "renderBoundingVolume"), true);
				glm::vec3 colour = glm::vec3(0.f, 1.f, 0.f);
				if (OctTreeEnabled && spatialPartitionTree)
				{
					colour = glm::vec3(1.f, 0.f, 0.f); //Red
				}

				glUniform3f(glGetUniformLocation(programID, "renderColour"), colour.x, colour.y, colour.z);
				//Draw
				models["Sphere"].DrawBoundingVolume();
			}
		}
		glDisableVertexAttribArray(0);
	}

	
	 if (OctTreeEnabled)
	{
		if (spatialPartitionTree == nullptr)
		{
			glm::vec3 Min = gameObjList[0].aabbBV.m_Min;
			glm::vec3 Max = gameObjList[0].aabbBV.m_Max;
			for (auto& obj : gameObjList)
			{
				if (Min.x > obj.aabbBV.m_Min.x)
					Min.x = obj.aabbBV.m_Min.x;
				if (Max.x < obj.aabbBV.m_Max.x)
					Max.x = obj.aabbBV.m_Max.x;
				if (Min.y > obj.aabbBV.m_Min.y)
					Min.y = obj.aabbBV.m_Min.y;
				if (Max.y < obj.aabbBV.m_Max.y)
					Max.y = obj.aabbBV.m_Max.y;
				if (Min.z > obj.aabbBV.m_Min.z)
					Min.z = obj.aabbBV.m_Min.z;
				if (Max.z < obj.aabbBV.m_Max.z)
					Max.z = obj.aabbBV.m_Max.z;
			}
			glm::vec3 center = (Min + Max) / 2.f;
			float halfWidth = std::max(Max.x - Min.x, Max.y - Min.y);
			halfWidth = std::max(Max.z - Min.z, halfWidth);
			halfWidth *= 0.5f;
			spatialPartitionTree = SpatialPartitioning::BuildOctTree(center, halfWidth, 3, 0, totalObjPolygons);

			//Insert all the game Objs into the list
			for (auto& obj : gameObjList)
			{
				std::vector<SpatialPartitioning::Polygon> objPolys = SpatialPartitioning::getPolygonsOfObj(modelPolys[obj.GetModelID()], obj);
				SpatialPartitioning::InsertIntoOctTree(spatialPartitionTree, objPolys);
			}

		}
		else
		{
			if (renderOctTree)
				RenderOctTree(spatialPartitionTree, projection, view, 0);
		}
	}
	else if (BSPTreeEnabled)
	{
		if (BSPTree == nullptr)
		{
			BSPTree = SpatialPartitioning::BuildBSPTree(totalObjPolygons, 0);
		}
		else
		{
			if (renderBSPTree)
				RenderBSPTree(BSPTree, projection, view);
		}
	}
	


	//Render IMGUI controls after rendering the scene
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	{
		ImGui::Begin("Controls");
		if (ImGui::BeginTabBar("Inspector"))
		{
			if (ImGui::BeginTabItem("Bounding Volumes"))
			{
				BVHenabled = false;
				OctTreeEnabled = false;
				BSPTreeEnabled = false;
				//runOnce = false;
				if (ImGui::Button("Update BV (After moving)"))
				{
					for (auto& obj : gameObjList) //Render gameObj controls
					{
						obj.changedCollider = true;
					}
				}

				ImGui::Text("Render Bounding Volumes");
				ImGui::Checkbox("##RenderBV", &renderBV);

				for (size_t i = 0; i < gameObjList.size(); i++) //Render gameObj controls
				{
					ImGui::Text("Object %d", i + 1);
					ImGui::PushID(i);
					gameObjList[i].DrawImGuiControls();
					ImGui::PopID();
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Spatial Partitioning"))
			{
				ImGui::Text("Click to rebuild tree");
				if (ImGui::Button("Update Tree"))
				{
					if (spatialPartitionTree != nullptr)
					{
						FreeOctTree(spatialPartitionTree);
						spatialPartitionTree = nullptr;
					}
					if (BSPTree != nullptr)
					{
						FreeBSPTree(BSPTree);
						BSPTree = nullptr;
					}
				}

				static const char* items[]{ "OctTree", "BSPTree" };
				ImGui::NewLine();
				ImGui::Text("Trees");
				ImGui::ListBox("", &spatPartTree, items, IM_ARRAYSIZE
				(items), 2);
				if (spatPartTree == 0)
				{
					OctTreeEnabled = true;
					BSPTreeEnabled = false;
				}
				else
				{
					BSPTreeEnabled = true;
					OctTreeEnabled = false;
				}
				BVHenabled = false;
				ImGui::Text("Render OctTree");
				ImGui::Checkbox("##RenderOctTree", &renderOctTree);

				ImGui::Text("OctTree Level (Max 3)");
				ImGui::InputInt("##OctTreeDepth", &octTreeRenderDepth);
				if (octTreeRenderDepth > 3) octTreeRenderDepth = 3;
				if (octTreeRenderDepth < 0) octTreeRenderDepth = 0;

				ImGui::Text("Render BSPTree");
				ImGui::Checkbox("##RenderBSPTree", &renderBSPTree);

				ImGui::Text("Minimum Polygon Count");
				ImGui::SameLine();
				ImGui::DragInt("##MinPolyCount", &minPolyCount, 1, 30, 300);

				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}


		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}


	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	return 0;
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
int SimpleScene_Quad::postRender()
{
	//angleOfRotation += 0.001f;

	return 0;
}



void SimpleScene_Quad::RenderOctTree(SpatialPartitioning::TreeNode* tree, const glm::mat4& projection, const glm::mat4& view, int col)
{
	SpatialPartitioning::TreeNode* node = tree;
	if (node == nullptr)
		return;

	if (node->depth < octTreeRenderDepth)
		return; //dont render the deeper nodes
	Transform aabbTrans, sphereTrans;


	glm::vec3 octTreeNodeCentre = node->center;
	Transform temp(octTreeNodeCentre, 1.f, { node->halfwidth, node->halfwidth, node->halfwidth }, { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f });
	aabbTrans = temp;

	//gameObjs.SetTransform(aabbTrans);
	glm::mat4 objTrans;
	objTrans = projection * view * aabbTrans.GetModelMtx3f();
	GLint vTransformLoc = glGetUniformLocation(programID, "vertexTransform");
	glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &objTrans[0][0]);
	glUniform1i(glGetUniformLocation(programID, "renderBoundingVolume"), true);

	////TreeDepth 0 Root Node
	glm::vec3 colour = glm::vec3(1.f, 0.f, 0.f); //Red

	//Get nodes' colours
	if (col > 0)
	{
		colour = node->colour;
	}

	glUniform3f(glGetUniformLocation(programID, "renderColour"), colour.x, colour.y, colour.z);
	//Draw
	models["Cube"].DrawBoundingVolume(); //Render the OctTree nodes
	
	for (size_t i = 0; i < 8; i++)
	{
		RenderOctTree(node->pChildren[i], projection, view, ++col); //Recursive call for all the children
	}
}

void SimpleScene_Quad::RenderBSPTree(SpatialPartitioning::BSPNode* tree, const glm::mat4& projection, const glm::mat4& view)
{
	SpatialPartitioning::BSPNode* node = tree;
	if (node == nullptr)
		return;

	if (node->currType == SpatialPartitioning::BSPNode::Type::INTERNAL)
	{
		RenderBSPTree(node->frontTree, projection, view);
		RenderBSPTree(node->backTree, projection, view);
		return;
	}
	else
	{
		glm::mat4 objTrans = projection * view * glm::mat4(1.0f);
		// Uniform transformation (vertex shader)
		GLint vTransformLoc = glGetUniformLocation(programID, "vertexTransform");
		glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &objTrans[0][0]);


		GLint fCamPosLoc = glGetUniformLocation(programID, "cameraPos");
		glUniform3f(fCamPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);
		glUniform1i(glGetUniformLocation(programID, "renderBoundingVolume"), true);

		glm::vec3 colour{ 0.f, 1.f, 0.f };
		//std::random_device rd;
		//std::default_random_engine eng(rd());
		//std::uniform_real_distribution<float> distr(0.f, 1.f);
		//colour = glm::vec3(distr(eng), distr(eng), distr(eng));
		colour = node->colour;
		glUniform3f(glGetUniformLocation(programID, "renderColour"), colour.x, colour.y, colour.z);
		node->geometry.GenericDrawTriangle();
	}

	

	//RenderBSPTree(node->frontTree, projection, view);
	//RenderBSPTree(node->backTree, projection, view);
}

void SimpleScene_Quad::FreeOctTree(SpatialPartitioning::TreeNode* node)
{
	if (node == nullptr)
		return;
	for (size_t i = 0; i < 8; i++)
	{
		FreeOctTree(node->pChildren[i]); //Free all children
	}
	delete node;
}

void SimpleScene_Quad::FreeBSPTree(SpatialPartitioning::BSPNode* node)
{
	if (node == nullptr)
		return;
	if (node->frontTree)
		FreeBSPTree(node->frontTree);
	if (node->backTree)
		FreeBSPTree(node->backTree);

	delete node;
}
